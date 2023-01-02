#include "Core/Widgets/GraphicSubComponent.h"
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
// UGraphicSubComponent

/** Watches for culture changes and updates all live UGraphicSubComponent components */
class FGraphicSubComponentCultureChangedFixUp
{
public:
    FGraphicSubComponentCultureChangedFixUp()
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
            for (UGraphicSubComponent* GraphicSubComponent : TObjectRange<UGraphicSubComponent>())
            {
                GraphicSubComponent->OnTextLocalizationChanged();
            }
        }
    };

    TSharedPtr<FImpl> ImplPtr;
};

/////////////////////////////////////////////////////

UGraphicSubComponent::UGraphicSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Color = FLinearColor::White;
    Material = nullptr;

    Canvas = nullptr;
    CanvasRenderer = nullptr;

    bIsVertsDirty = false;
    bIsMaterialDirty = false;
    bIsGraphicsEffectDirty = false;
    bIsRenderOpacityDirty = false;

    bRaycastTarget = true;
    bAntiAliasing = false;
    
    bGraying = false;
    bCurrentGraying = false;
    bInvertColor = false;
    bCurrentInvertColor = false;

#if WITH_EDITOR
    bRegisterGraphicForEditor = false;
#endif

    RenderOpacity = 1;
    CurrentRenderOpacity = 1;

    if (!IsRunningDedicatedServer())
    {
        {
            // Static used to watch for culture changes and update all live UGraphicSubComponent components
            // In this constructor so that it has a known initialization order, and is only created when we need it
            static FGraphicSubComponentCultureChangedFixUp GraphicSubComponentCultureChangedFixUp;
        }
    }
}

#if WITH_EDITORONLY_DATA

void UGraphicSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
    if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UGraphicSubComponent, bRaycastTarget))
    {
        const bool bNewRaycastTarget = bRaycastTarget;
        bRaycastTarget = !bNewRaycastTarget;
        SetRaycastTarget(bNewRaycastTarget);
    }

    SetAllDirty();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UGraphicSubComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    SetAllDirty();
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif

void UGraphicSubComponent::OnEnable()
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

void UGraphicSubComponent::OnDisable()
{
	if (bRaycastTarget)
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

    FLayoutRebuilder::MarkLayoutForRebuild(AttachTransform);

    Super::OnDisable();
}

void UGraphicSubComponent::OnDestroy()
{
    if (ColorTweenRunner.IsValid())
    {
        ColorTweenRunner->StopTween();
        ColorTweenRunner.Reset();
    }

    Super::OnDestroy();
}

void UGraphicSubComponent::OnRectTransformDimensionsChange()
{
    Super::OnRectTransformDimensionsChange();

    if (IsActiveAndEnabled())
    {
        // prevent double dirtying...
        if (FCanvasUpdateRegistry::IsRebuildingLayout())
            SetVerticesDirty();
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


void UGraphicSubComponent::OnTransformParentChanged()
{
    Super::OnTransformParentChanged();

    FLayoutRebuilder::MarkLayoutForRebuild(AttachTransform);

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

void UGraphicSubComponent::OnTransformChanged()
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

void UGraphicSubComponent::OnBlocksRaycastsStateChanged()
{
    Super::OnBlocksRaycastsStateChanged();

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

void UGraphicSubComponent::Rebuild(ECanvasUpdate Executing)
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

void UGraphicSubComponent::OnGrayingStateChanged()
{
    SetGraphicEffectsDirty();
}

void UGraphicSubComponent::OnInvertColorStateChanged()
{
    SetGraphicEffectsDirty();
}

void UGraphicSubComponent::OnRenderOpacityChanged()
{
    SetRenderOpacityDirty();
}

void UGraphicSubComponent::OnCanvasHierarchyChanged()
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

void UGraphicSubComponent::OnCullingChanged()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if ((bIsVertsDirty || bIsMaterialDirty || bIsRenderOpacityDirty || bIsGraphicsEffectDirty) && IsValid(CanvasRendererComp) && !CanvasRendererComp->IsShouldCull() && !CanvasRendererComp->IsHidePrimitive())
    {
        // When we were culled, we potentially skipped calls to Rebuild
        FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    }
}

UCanvasSubComponent* UGraphicSubComponent::GetCanvas()
{
    if (!IsValid(Canvas))
    {
        CacheCanvas();
    }
    return Canvas;
}

void UGraphicSubComponent::CacheCanvas()
{
    if (AttachTransform)
    {
        const auto OwnerCanvas = AttachTransform->GetOwnerCanvas();

        if (IsValid(OwnerCanvas))
        {
            Canvas = OwnerCanvas->GetOrderOverrideCanvas();
        }
        else
        {
            Canvas = nullptr;
        }
    }
}

void UGraphicSubComponent::InternalUpdateGrayingState()
{
    if (!HasBeenAwaken())
        return;

    const bool bGrayingInHierarchy = IsGrayingInHierarchy();
    if (bGrayingInHierarchy != bCurrentGraying)
    {
        bCurrentGraying = bGrayingInHierarchy;
        OnGrayingStateChanged();
    }
}

void UGraphicSubComponent::InternalUpdateInvertColorState()
{
    if (!HasBeenAwaken())
        return;

    const bool bInvertColorInHierarchy = IsInvertColorInHierarchy();
    if (bInvertColorInHierarchy != bCurrentInvertColor)
    {
        bCurrentInvertColor = bInvertColorInHierarchy;
        OnInvertColorStateChanged();
    }
}

void UGraphicSubComponent::InternalUpdateRenderOpacity()
{
    if (!HasBeenAwaken())
        return;

    const float ParentRenderOpacity = GetRenderOpacityInHierarchy();
    if (!FMath::IsNearlyEqual(ParentRenderOpacity, CurrentRenderOpacity))
    {
        CurrentRenderOpacity = ParentRenderOpacity;
        OnRenderOpacityChanged();
    }
}

void UGraphicSubComponent::SetGraying(bool bInGraying)
{
    if (bGraying != bInGraying)
    {
        bGraying = bInGraying;
        InternalUpdateGrayingState();
    }
}

bool UGraphicSubComponent::IsGrayingInHierarchy() const
{
    const auto Owner = Cast<UBehaviourComponent>(GetOuter());
    if (IsValid(Owner))
    {
        return bGraying && Owner->IsGrayingInHierarchy();
    }
    return bGraying;
}

void UGraphicSubComponent::SetInvertColor(bool bInInvertColor)
{
    if (bInvertColor != bInInvertColor)
    {
        bInvertColor = bInInvertColor;
        InternalUpdateInvertColorState();
    }
}

bool UGraphicSubComponent::IsInvertColorInHierarchy() const
{
    const auto Owner = Cast<UBehaviourComponent>(GetOuter());
    if (IsValid(Owner))
    {
        return bInvertColor && Owner->IsInvertColorInHierarchy();
    }
    return bInvertColor;
}

void UGraphicSubComponent::SetRenderOpacity(float InRenderOpacity)
{
    if (RenderOpacity != InRenderOpacity)
    {
        RenderOpacity = InRenderOpacity;
        InternalUpdateRenderOpacity();
    }
}

float UGraphicSubComponent::GetRenderOpacityInHierarchy() const
{
    const auto Owner = Cast<UBehaviourComponent>(GetOuter());
    if (IsValid(Owner))
    {
        return RenderOpacity * Owner->GetRenderOpacityInHierarchy();
    }
    return RenderOpacity;
}

void UGraphicSubComponent::SetAntiAliasing(bool bInAntiAliasing)
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

void UGraphicSubComponent::SetRaycastTarget(bool bInRaycastTarget)
{
    if (bRaycastTarget != bInRaycastTarget)
    {
        bRaycastTarget = bInRaycastTarget;

        if (bRaycastTarget)
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
            if (IsValid(CacheCanvas) && IsValid(World))
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

void UGraphicSubComponent::SetHidePrimitive(bool bInHidePrimitive)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetHidePrimitive(bInHidePrimitive);
    }
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- Raycast"), STAT_UnrealGUI_Sub_GraphicRaycast, STATGROUP_UnrealGUI);
bool UGraphicSubComponent::Raycast(const FVector& WorldRayOrigin, const FVector& WorldRayDir)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Sub_GraphicRaycast);
    
    if (!IsActiveAndEnabled())
        return false;

    if (!IsBlockRaycastsInHierarchy())
        return false;
    
    USceneComponent* CurSceneComp = AttachTransform;
    
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
            {
                continue;
            }

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

void UGraphicSubComponent::SetLayoutDirty()
{
    if (!IsActiveAndEnabled())
        return;

    FLayoutRebuilder::MarkLayoutForRebuild(AttachTransform);
    OnDirtyLayoutCallback.Broadcast();
}

void UGraphicSubComponent::SetVerticesDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsVertsDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    OnDirtyVertsCallback.Broadcast();
}

void UGraphicSubComponent::SetGraphicEffectsDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsGraphicsEffectDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
}

void UGraphicSubComponent::SetRenderOpacityDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsRenderOpacityDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
}

void UGraphicSubComponent::SetMaterialDirty()
{
    if (!IsActiveAndEnabled())
        return;

    bIsMaterialDirty = true;
    FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(this);
    OnDirtyMaterialCallback.Broadcast();
}

void UGraphicSubComponent::UpdateMaterial()
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

void UGraphicSubComponent::UpdateGraphicEffects()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        // UV1(bGraying, bInvertColor)
        CanvasRendererComp->UpdateMeshUV1(GetUV1FromGraphicEffects());
    }
}

void UGraphicSubComponent::UpdateRenderOpacity()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetInheritedAlpha(GetRenderOpacityInHierarchy());
    }
}

void UGraphicSubComponent::UpdateGeometry()
{
    DoMeshGeneration();
}

FVector2D UGraphicSubComponent::GetUV1FromGraphicEffects() const
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

void UGraphicSubComponent::DoMeshGeneration()
{
	if (!IsValid(AttachTransform))
		return;
		
    const auto& TargetRect = AttachTransform->GetRect();
    if (TargetRect.Width >= 0 && TargetRect.Height >= 0)
    {
        OnPopulateMesh(StaticVertexHelper);
    }
    else
    {
        StaticVertexHelper.Reset();
    }

    UBehaviourComponent* OwnerComp = Cast<UBehaviourComponent>(GetOuter());
    if (IsValid(OwnerComp))
    {
        TArray<IMeshModifierInterface*, TInlineAllocator<8>> Components;
        OwnerComp->GetComponents(Components);

        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            Components[Index]->ModifyMesh(StaticVertexHelper);
        }
    }

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->FillMesh(StaticVertexHelper);
    }
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- OnPopulateMesh"), STAT_UnrealGUI_Sub_GraphicOnPopulateMesh, STATGROUP_UnrealGUI);
void UGraphicSubComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Sub_GraphicOnPopulateMesh);
    
    const auto& FinalRect = GetPixelAdjustedRect();
    const float BottomLeftX = FinalRect.XMin;
    const float BottomLeftY = FinalRect.YMin;
    const float TopRightX = BottomLeftX + FinalRect.Width;
    const float TopRightY = BottomLeftY + FinalRect.Height;

    const FVector2D UV1 = GetUV1FromGraphicEffects();

    VertexHelper.Empty();

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

FVector2D UGraphicSubComponent::PixelAdjustPoint(FVector2D Point)
{
    const auto OwnerCanvas = GetCanvas();
    if (!IsValid(OwnerCanvas) || OwnerCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || OwnerCanvas->GetScaleFactor() == 0.0f || !OwnerCanvas->GetPixelPerfect())
        return Point;
    return FRectTransformUtility::PixelAdjustPoint(Point, AttachTransform, OwnerCanvas);
}

FRect UGraphicSubComponent::GetPixelAdjustedRect()
{
    const auto OwnerCanvas = GetCanvas();
    if (!IsValid(OwnerCanvas) || OwnerCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace || OwnerCanvas->GetScaleFactor() == 0.0f || !OwnerCanvas->GetPixelPerfect())
        return AttachTransform->GetRect();
    return FRectTransformUtility::PixelAdjustRect(AttachTransform, OwnerCanvas);
}

void UGraphicSubComponent::CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha, bool bUseRGB)
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
    ColorTween->OnColorTweenCallback.AddUObject(this, &UGraphicSubComponent::SetCanvasRendererColor);

    ColorTweenRunner->StartTween(GetWorld(), IsActiveAndEnabled());
}

/////////////////////////////////////////////////////
