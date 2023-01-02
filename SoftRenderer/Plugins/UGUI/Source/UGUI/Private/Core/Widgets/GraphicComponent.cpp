#include "Core/Widgets/GraphicComponent.h"
#include "Animation/ColorTween.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/VertexHelper.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Render/CanvasRaycastFilterInterface.h"
#include "Core/VertexModifiers/MeshModifierInterface.h"
#include "Core/Widgets/MaskableGraphicElementInterface.h"
#include "UGUI.h"
#include "UGUIWorldSubsystem.h"

/////////////////////////////////////////////////////
// UGraphicComponent

/** Watches for culture changes and updates all live UGraphicComponent components */
class FGraphicComponentCultureChangedFixUp
{
public:
    FGraphicComponentCultureChangedFixUp()
        : ImplPtr(MakeShareable(new FImpl()))
    {
        ImplPtr->Register();
    }

private:
    struct FImpl : public TSharedFromThis<FImpl>
    {
        void Register() const
        {
            FTextLocalizationManager::Get().OnTextRevisionChangedEvent.AddSP(this, &FImpl::HandleLocalizedTextChanged);
        }

        void HandleLocalizedTextChanged() const
        {
            for (UGraphicComponent* GraphicComponent : TObjectRange<UGraphicComponent>())
            {
                GraphicComponent->OnTextLocalizationChanged();
            }
        }
    };

    TSharedPtr<FImpl> ImplPtr;
};

/////////////////////////////////////////////////////

UGraphicComponent::UGraphicComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Color = FLinearColor::White;
    Material = nullptr;
    OverrideMaterial = nullptr;
    
    Canvas = nullptr;
    CanvasRenderer = nullptr;

    bIsVertsDirty = false;
    bIsMaterialDirty = false;
    bIsGraphicsEffectDirty = false;
    bIsRenderOpacityDirty = false;

    bRaycastTarget = true;
    bAntiAliasing = false;

#if WITH_EDITOR
    bRegisterGraphicForEditor = false;
#endif

    const auto CanvasRenderSubComp = CreateDefaultSubobject<UCanvasRendererSubComponent>(TEXT("CanvasRendererSubComp0"));
    if (CanvasRenderSubComp)
    {
        SubComponents.Emplace(CanvasRenderSubComp);
    }

    if(!IsRunningDedicatedServer())
    {
        {
            // Static used to watch for culture changes and update all live UGraphicComponent components
            // In this constructor so that it has a known initialization order, and is only created when we need it
            static FGraphicComponentCultureChangedFixUp GraphicComponentCultureChangedFixUp;
        }
    }
}

#if WITH_EDITORONLY_DATA

void UGraphicComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
    if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UGraphicComponent, bRaycastTarget))
    {
        const bool bNewRaycastTarget = bRaycastTarget;
        bRaycastTarget = !bNewRaycastTarget;
        SetRaycastTarget(bNewRaycastTarget);
    }

    SetAllDirty();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UGraphicComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    SetAllDirty();
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif

void UGraphicComponent::OnEnable()
{
    Super::OnEnable();

    Canvas = GetCanvas();

    if (IsRaycastTargetForGraphic())
    {
        const auto CacheCanvas = GetCanvas();
        const auto World = GetWorld();
        if (IsValid(CacheCanvas) && IsValid(World))
        {
            const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
            if (IsValid(UIWorldSubsystem))
            {
                UIWorldSubsystem->AddRaycastTarget(CacheCanvas, this);
            }
        }
    }

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetAntiAliasing(bAntiAliasing);
    }

    SetAllDirty();
}

void UGraphicComponent::OnDisable()
{
    if (IsRaycastTargetForGraphic())
	{
		const auto CacheCanvas = GetCanvas();
        const auto World = GetWorld();
        if (IsValid(CacheCanvas) && IsValid(World))
		{
		    const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
		    if (IsValid(UIWorldSubsystem))
		    {
		        UIWorldSubsystem->RemoveRaycastTarget(CacheCanvas, this);
		    }
		}
	}

    FCanvasUpdateRegistry::UnRegisterCanvasElementForRebuild(this);

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->Clear();
    }

    FLayoutRebuilder::MarkLayoutForRebuild(this);

    Super::OnDisable();
}

void UGraphicComponent::OnDestroy()
{
    if (ColorTweenRunner.IsValid())
    {
        ColorTweenRunner->StopTween();
        ColorTweenRunner.Reset();
    }

    Super::OnDestroy();
}

void UGraphicComponent::OnRectTransformDimensionsChange()
{
    Super::OnRectTransformDimensionsChange();

    if (IsActiveAndEnabled())
    {
        // prevent double dirtying...
        if (FCanvasUpdateRegistry::IsRebuildingLayout())
        {
            SetVerticesDirty();
        }
        else
        {
            SetVerticesDirty();
            SetLayoutDirty();
        }
    }

	if (HasBeenAwaken())
	{
		const auto World = GetWorld();
		if (IsRaycastTargetForGraphic() && IsValid(World) && GetCanvas())
		{
			const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
			if (IsValid(UIWorldSubsystem))
			{
				UIWorldSubsystem->AddRaycastTarget(Canvas, this);
			}
		}
	}
}

void UGraphicComponent::OnTransformParentChanged()
{
    Super::OnTransformParentChanged();

    FLayoutRebuilder::MarkLayoutForRebuild(this);
    
	// Use Canvas so we don't auto call CacheCanvas
	const auto CurrentCanvas = Canvas;
	
    Canvas = nullptr;

    if (!IsActiveAndEnabled())
        return;

    CacheCanvas();

    const auto World = GetWorld();
    if (IsRaycastTargetForGraphic() && CurrentCanvas != Canvas && IsValid(World))
	{
	    const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
	    if (IsValid(UIWorldSubsystem))
	    {
	        UIWorldSubsystem->RemoveRaycastTarget(CurrentCanvas, this);
	        UIWorldSubsystem->AddRaycastTarget(Canvas, this);
	    }
	}

    SetAllDirty();
}

void UGraphicComponent::OnTransformChanged()
{
    Super::OnTransformChanged();

	const auto World = GetWorld();
	if (IsRaycastTargetForGraphic() && IsValid(World) && GetCanvas())
	{
		const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
		if (IsValid(UIWorldSubsystem))
		{
			UIWorldSubsystem->AddRaycastTarget(Canvas, this);
		}
	}
}

void UGraphicComponent::OnBlockRaycastsStateChanged()
{
    Super::OnBlockRaycastsStateChanged();
	
    if (const auto World = GetWorld())
    {
        if (const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>())
        {
            if (IsRaycastTargetForGraphic())
            {
                UIWorldSubsystem->AddRaycastTarget(GetCanvas(), this);
            }
            else
            {
                UIWorldSubsystem->RemoveRaycastTarget(GetCanvas(), this);
            }
        }
    }
}

void UGraphicComponent::Rebuild(ECanvasUpdate Executing)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (!IsValid(CanvasRendererComp) || CanvasRendererComp->IsShouldCull() || CanvasRendererComp->IsHidePrimitive())
    {
        return;
    }

    switch (Executing)
    {
    case ECanvasUpdate::CanvasUpdate_PreRender:
        if (bIsVertsDirty)
        {
            UpdateGeometry();
            bIsVertsDirty = false;
        }
        else
        {
            if (bIsGraphicsEffectDirty)
            {
                UpdateGraphicEffects();
            }
        }

        bIsGraphicsEffectDirty = false;

        if (bIsMaterialDirty)
        {
            UpdateMaterial();
            bIsMaterialDirty = false;
        }

        if (bIsRenderOpacityDirty)
        {
            UpdateRenderOpacity();
            bIsRenderOpacityDirty = false;
        }
        break;
    default:
        ;
    }
}

int32 UGraphicComponent::GetNumOverrideMaterials() const
{
    if (Material)
    {
        return 1;
    }
    return 0;
}

UMaterialInterface* UGraphicComponent::GetOverrideMaterial(int32 MaterialIndex) const
{
    if (OverrideMaterial)
    {
        return OverrideMaterial;
    }
    return Material;
}

void UGraphicComponent::SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial)
{
    OverrideMaterial = InMaterial;
    SetMaterialDirty();
}

void UGraphicComponent::OnGrayingStateChanged()
{
    Super::OnGrayingStateChanged();
    SetGraphicEffectsDirty();
}

void UGraphicComponent::OnInvertColorStateChanged()
{
    Super::OnInvertColorStateChanged();
    SetGraphicEffectsDirty();
}

void UGraphicComponent::OnRenderOpacityChanged()
{
    Super::OnInvertColorStateChanged();
    SetRenderOpacityDirty();
}

void UGraphicComponent::OnCanvasHierarchyChanged()
{
    Super::OnCanvasHierarchyChanged();

    // Use Canvas so we don't auto call CacheCanvas
    const auto CurrentCanvas = Canvas;

    // Clear the cached canvas. Will be fetched below if active.
    Canvas = nullptr;

    if (!IsActiveAndEnabled())
        return;

    CacheCanvas();

    const auto World = GetWorld();
    if (IsRaycastTargetForGraphic() && CurrentCanvas != Canvas && IsValid(World))
    {
        const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
        if (IsValid(UIWorldSubsystem))
        {
            UIWorldSubsystem->RemoveRaycastTarget(CurrentCanvas, this);
            UIWorldSubsystem->AddRaycastTarget(Canvas, this);
        }
    }
}

void UGraphicComponent::OnCullingChanged()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if ((bIsVertsDirty || bIsMaterialDirty || bIsRenderOpacityDirty || bIsGraphicsEffectDirty) && IsValid(CanvasRendererComp) && !CanvasRendererComp->IsShouldCull() && !CanvasRendererComp->IsHidePrimitive())
    {
        // When we were culled, we potentially skipped calls to Rebuild
        FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    }
}

UCanvasSubComponent* UGraphicComponent::GetCanvas()
{
    if (!IsValid(Canvas))
    {
        CacheCanvas();
    }
    return Canvas;
}

void UGraphicComponent::CacheCanvas()
{
    if (IsValid(OwnerCanvas))
    {
        Canvas = OwnerCanvas->GetOrderOverrideCanvas();
    }
    else
    {
        Canvas = nullptr;
    }
}

void UGraphicComponent::SetAntiAliasing(bool bInAntiAliasing)
{
    if (bAntiAliasing != bInAntiAliasing)
    {
        bAntiAliasing = bInAntiAliasing;

        const auto CanvasRendererComp = GetCanvasRenderer();
        if (IsValid(CanvasRendererComp))
        {
            CanvasRendererComp->SetAntiAliasing(bAntiAliasing);
        }

        SetVerticesDirty();
    }
}

void UGraphicComponent::SetRaycastTarget(bool bInRaycastTarget)
{
    if (bRaycastTarget != bInRaycastTarget)
    {
        bRaycastTarget = bInRaycastTarget;

        if (IsRaycastTargetForGraphic())
        {
            const auto CacheCanvas = GetCanvas();
            const auto World = GetWorld();
            if (IsValid(CacheCanvas) && IsValid(World))
            {
                const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
                if (IsValid(UIWorldSubsystem))
                {
                    UIWorldSubsystem->AddRaycastTarget(CacheCanvas, this);
                }
            }
        }
        else
        {
            const auto CacheCanvas = GetCanvas();
            const auto World = GetWorld();
            if (IsValid(CacheCanvas)  && IsValid(World))
            {
                const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
                if (IsValid(UIWorldSubsystem))
                {
                    UIWorldSubsystem->RemoveRaycastTarget(CacheCanvas, this);
                }
            }
        }
    }
}

void UGraphicComponent::SetHidePrimitive(bool bInHidePrimitive)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetHidePrimitive(bInHidePrimitive);
    }
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- Raycast"), STAT_UnrealGUI_GraphicRaycast, STATGROUP_UnrealGUI);
bool UGraphicComponent::Raycast(const FVector& WorldRayOrigin, const FVector& WorldRayDir)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GraphicRaycast);
    
    if (!IsActiveAndEnabled())
        return false;

    if (!IsBlockRaycastsInHierarchy())
        return false;
    
    USceneComponent* CurSceneComp = this;
    
    bool bContinueTraversal = true;

    TArray<UObject*, TInlineAllocator<8>> Components;
    IMaskableGraphicElementInterface* MaskableGraphicElement = Cast<IMaskableGraphicElementInterface>(this);

    while (IsValid(CurSceneComp))
    {
        Components.Reset();

        const auto BehaviourComp = Cast<UBehaviourComponent>(CurSceneComp);
        if (IsValid(BehaviourComp))
        {
            BehaviourComp->GetComponents(Components);
        }
        else
        {
            Components.Add(CurSceneComp);
        }

        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            const auto CanvasComp = Cast<UCanvasSubComponent>(Components[Index]);
            if (IsValid(CanvasComp) && CanvasComp->IsOverrideSorting())
                bContinueTraversal = false;

            const auto Filter = Cast<ICanvasRaycastFilterInterface>(Components[Index]); ;

            if (!Filter)
                continue;

            const bool bRaycastValid = Filter->IsRaycastLocationValid(MaskableGraphicElement, WorldRayOrigin, WorldRayDir, IsIgnoreReversedGraphicsInHierarchy());
            if (!bRaycastValid)
            {
                return false;
            }
        }

        CurSceneComp = bContinueTraversal ? CurSceneComp->GetAttachParent() : nullptr;
    }
    
    return true;
}

void UGraphicComponent::SetLayoutDirty()
{
    if (!IsActiveAndEnabled())
        return;

    FLayoutRebuilder::MarkLayoutForRebuild(this);
    OnDirtyLayoutCallback.Broadcast();
}

void UGraphicComponent::SetVerticesDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsVertsDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    OnDirtyVertsCallback.Broadcast();
}

void UGraphicComponent::SetGraphicEffectsDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsGraphicsEffectDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
}

void UGraphicComponent::SetRenderOpacityDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsRenderOpacityDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
}

void UGraphicComponent::SetMaterialDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsMaterialDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    OnDirtyMaterialCallback.Broadcast();
}

void UGraphicComponent::UpdateMaterial()
{
    if (!IsActiveAndEnabled())
        return;

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetMaterial(GetMaterial());
        CanvasRendererComp->SetTexture(GetMainTexture());
    }
}

void UGraphicComponent::UpdateGraphicEffects()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        // UV1(bGraying, bInvertColor)
        CanvasRendererComp->UpdateMeshUV1(GetUV1FromGraphicEffects());
    }
}

void UGraphicComponent::UpdateRenderOpacity()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetInheritedAlpha(GetRenderOpacityInHierarchy());
    }
}

void UGraphicComponent::UpdateGeometry()
{
    DoMeshGeneration();
}

FVector2D UGraphicComponent::GetUV1FromGraphicEffects() const
{
    FVector2D UV1 = FVector2D::ZeroVector;
    if (IsGrayingInHierarchy())
    {
        UV1.X = 2;
    }

    if (IsInvertColorInHierarchy())
    {
        UV1.Y = 2;
    }

    return UV1;
}

void UGraphicComponent::DoMeshGeneration()
{
    if (Rect.Width >= 0 && Rect.Height >= 0)
    {
        OnPopulateMesh(StaticVertexHelper);
    }
    else
    {
        StaticVertexHelper.Reset();
    }

    TArray<IMeshModifierInterface*, TInlineAllocator<8>> Components;
    GetComponents(Components);

    for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
    {
        Components[Index]->ModifyMesh(StaticVertexHelper);
    }

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->FillMesh(StaticVertexHelper);
    }
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- OnPopulateMesh"), STAT_UnrealGUI_GraphicOnPopulateMesh, STATGROUP_UnrealGUI);
void UGraphicComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GraphicOnPopulateMesh);
    
    const auto& FinalRect = GetPixelAdjustedRect();
    const float BottomLeftX = FinalRect.XMin;
    const float BottomLeftY = FinalRect.YMin;
    const float TopRightX = BottomLeftX + FinalRect.Width;
    const float TopRightY = BottomLeftY + FinalRect.Height;

    VertexHelper.Empty();

    const FVector2D UV1 = GetUV1FromGraphicEffects();

    if (!bAntiAliasing)
    {
        VertexHelper.Reserve(4, 6);

        VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(0, 1), UV1);
        VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(1, 1), UV1);
        VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(1, 0), UV1);
        VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(0, 0), UV1);

        VertexHelper.AddTriangle(0, 1, 2);
        VertexHelper.AddTriangle(2, 3, 0);

    }
    else
    {
        VertexHelper.Reserve(5, 12);

        VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(0, 1), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(1, 1), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(BottomLeftX + (TopRightX - BottomLeftX) * 0.5, BottomLeftY + (TopRightY - BottomLeftY) * 0.5, 0),
            Color, FVector2D(0.5, 0.5), UV1, FVector2D(0, 0));
        VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(1, 0), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(0, 0), UV1, FVector2D(1, 0));

        VertexHelper.AddTriangle(2, 0, 1);
        VertexHelper.AddTriangle(2, 1, 3);
        VertexHelper.AddTriangle(2, 3, 4);
        VertexHelper.AddTriangle(2, 4, 0);
    }
}

FVector2D UGraphicComponent::PixelAdjustPoint(FVector2D Point)
{
    const auto CurCanvas = GetCanvas();
    if (!IsValid(CurCanvas) || CurCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || CurCanvas->GetScaleFactor() == 0.0f || !CurCanvas->GetPixelPerfect())
        return Point;
    return FRectTransformUtility::PixelAdjustPoint(Point, this, CurCanvas);
}

FRect UGraphicComponent::GetPixelAdjustedRect()
{
    const auto CurCanvas = GetCanvas();
    if (!IsValid(CurCanvas) || CurCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || CurCanvas->GetScaleFactor() == 0.0f || !CurCanvas->GetPixelPerfect())
        return Rect;
    return FRectTransformUtility::PixelAdjustRect(this, CurCanvas);
}

void UGraphicComponent::CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha, bool bUseRGB)
{
    if (!bUseRGB && !bUseAlpha)
        return;

    const auto& CurrentColor = IsValid(GetCanvasRenderer()) ? GetCanvasRenderer()->GetColor() : FLinearColor::White;
    if (CurrentColor == TargetColor)
    {
        if (ColorTweenRunner.IsValid())
        {
            ColorTweenRunner->StopTween();
        }
        return;
    }

    const EColorTweenMode Mode = (bUseRGB && bUseAlpha ? EColorTweenMode::All :
        (bUseRGB ? EColorTweenMode::RGB : EColorTweenMode::Alpha));

    if (!ColorTweenRunner.IsValid())
    {
        ColorTweenRunner = MakeShareable(new FTweenRunner());
        ColorTweenRunner->Init(MakeShareable(new FColorTween()));
    }

    FColorTween* ColorTween = static_cast<FColorTween*>(ColorTweenRunner->GetTweenValue());
    ColorTween->SetDuration(Duration);
    ColorTween->StartColor = CurrentColor;
    ColorTween->TargetColor = TargetColor;
    ColorTween->bIgnoreTimeScale = bIgnoreTimeScale;
    ColorTween->TweenMode = Mode;

    ColorTween->OnColorTweenCallback.Clear();
    ColorTween->OnColorTweenCallback.AddUObject(this, &UGraphicComponent::SetCanvasRendererColor);

    ColorTweenRunner->StartTween(GetWorld(), IsActiveAndEnabled());
}

/////////////////////////////////////////////////////
