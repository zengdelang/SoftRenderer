#include "Core/Widgets/MaskableGraphicSubComponent.h"
#include "Core/Culling/MaskUtilities.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Widgets/RectMask2DSubComponent.h"

/////////////////////////////////////////////////////
// UMaskableGraphicSubComponent

UMaskableGraphicSubComponent::UMaskableGraphicSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ParentMask = nullptr;
	bMaskable = true;
}

void UMaskableGraphicSubComponent::OnEnable()
{
    Super::OnEnable();

    UpdateClipParent();
    SetMaterialDirty();
}

void UMaskableGraphicSubComponent::OnDisable()
{
    Super::OnDisable();

    SetMaterialDirty();
    UpdateClipParent();
}

void UMaskableGraphicSubComponent::OnCanvasHierarchyChanged()
{
    Super::OnCanvasHierarchyChanged();

    if (!IsActiveAndEnabled())
        return;

    UpdateClipParent();
    SetMaterialDirty();
}

void UMaskableGraphicSubComponent::OnTransformParentChanged()
{
    Super::OnTransformParentChanged();

    if (!IsActiveAndEnabled())
        return;

    UpdateClipParent();
    SetMaterialDirty();
}

#if WITH_EDITORONLY_DATA

void UMaskableGraphicSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    UpdateClipParent();
    SetMaterialDirty();
}

#endif

void UMaskableGraphicSubComponent::RecalculateClipping()
{
    UpdateClipParent();
}

void UMaskableGraphicSubComponent::Cull(FRect ClipRect, bool bValidRect)
{
    const bool bCull = !bValidRect || !ClipRect.Overlaps(GetRootCanvasRect(), true);
    UpdateCull(bCull);
}

void UMaskableGraphicSubComponent::SetClipRect(FRect ClipRect, bool bValidRect, FRect ClipSoftnessRect)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        if (bValidRect)
        {
            CanvasRendererComp->EnableRectClipping(ClipRect, ClipSoftnessRect);
        }
        else
        {
            CanvasRendererComp->DisableRectClipping();
        }
    }
}

void UMaskableGraphicSubComponent::UpdateCull(bool bCull)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        if (CanvasRendererComp->IsShouldCull() != bCull)
        {
            CanvasRendererComp->SetShouldCull(bCull);
            OnCullStateChanged.Broadcast(bCull);
            OnCullingChanged();
        }
    }
}

FRect UMaskableGraphicSubComponent::GetRootCanvasRect()
{
	if (IsValid(GetCanvas()))
	{
        const auto RootCanvas = Canvas->GetRootCanvas();
        if (IsValid(RootCanvas) && RootCanvas->GetRectTransform())
        {
            FVector Corners[4];
            AttachTransform->GetWorldCorners(Corners);

            const auto& WorldToCanvas = RootCanvas->GetRectTransform()->GetComponentTransform().Inverse();
            for (int32 Index = 0; Index < 4; ++Index)
            {
                Corners[Index] = WorldToCanvas.TransformPosition(Corners[Index]);
            }

            // bounding box is now based on the min and max of all corners

            FVector2D Min = FVector2D(Corners[0]);
            FVector2D Max = FVector2D(Corners[0]);

            for (int32 Index = 0; Index < 4; ++Index)
            {
                Min.X = FMath::Min(Corners[Index].X, Min.X);
                Min.Y = FMath::Min(Corners[Index].Y, Min.Y);
                Max.X = FMath::Max(Corners[Index].X, Max.X);
                Max.Y = FMath::Max(Corners[Index].Y, Max.Y);
            }

            return FRect(Min, Max - Min);
        }
	}

    return FRect();
}

void UMaskableGraphicSubComponent::UpdateClipParent()
{
    URectMask2DSubComponent* NewParent = (bMaskable && IsActiveAndEnabled()) ? FMaskUtilities::GetRectMaskForClippable(this) : nullptr;

    // if the new parent is different OR is now inactive
    if (IsValid(ParentMask) && (NewParent != ParentMask || !NewParent->IsActiveAndEnabled()))
    {
        ParentMask->RemoveClippable(this);
        UpdateCull(false);
    }

    // don't re-add it if the new parent is inactive
    if (IsValid(NewParent) && NewParent->IsActiveAndEnabled())
    {
        NewParent->AddClippable(this);
    }
	
    ParentMask = NewParent;
}

/////////////////////////////////////////////////////
