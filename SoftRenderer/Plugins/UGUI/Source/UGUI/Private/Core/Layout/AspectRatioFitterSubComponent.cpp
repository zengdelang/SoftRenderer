#include "Core/Layout/AspectRatioFitterSubComponent.h"

/////////////////////////////////////////////////////
// UAspectRatioFitterSubComponent

UAspectRatioFitterSubComponent::UAspectRatioFitterSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AspectMode = EAspectMode::AspectMode_None;
    AspectRatio = 1;
}

#if WITH_EDITORONLY_DATA

void UAspectRatioFitterSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    SetDirty();
}

void UAspectRatioFitterSubComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    SetDirty();
}

#endif

void UAspectRatioFitterSubComponent::UpdateRect() const
{
    if (!IsActiveAndEnabled())
        return;
	
    const auto RectTransform = Cast<URectTransformComponent>(GetOuter());
    if (!IsValid(RectTransform))
        return;
	
    switch (AspectMode)
    {
    case EAspectMode::AspectMode_HeightControlsWidth:
    {
        RectTransform->SetSizeWithCurrentAnchors(ERectTransformAxis::RectTransformAxis_Horizontal, RectTransform->GetRect().Height * AspectRatio);
        break;
    }
    case EAspectMode::AspectMode_WidthControlsHeight:
    {
        RectTransform->SetSizeWithCurrentAnchors(ERectTransformAxis::RectTransformAxis_Vertical, RectTransform->GetRect().Width / AspectRatio);
        break;
    }
    case EAspectMode::AspectMode_FitInParent:
    case EAspectMode::AspectMode_EnvelopeParent:
    {
        FVector2D SizeDelta = FVector2D::ZeroVector;
        const FVector2D ParentSize = GetParentSize();
        if ((ParentSize.Y * AspectRatio < ParentSize.X) ^ (AspectMode == EAspectMode::AspectMode_FitInParent))
        {
            SizeDelta.Y = GetSizeDeltaToProduceSize(ParentSize.X / AspectRatio, 1);
        }
        else
        {
            SizeDelta.X = GetSizeDeltaToProduceSize(ParentSize.Y * AspectRatio, 0);
        }

        RectTransform->SetAnchorAndSizeAndPosition(FVector2D::ZeroVector, FVector2D::UnitVector,
            SizeDelta, FVector2D::ZeroVector);
        break;
    }
    default: ;
    }
}

float UAspectRatioFitterSubComponent::GetSizeDeltaToProduceSize(float Size, int32 Axis) const
{
    const auto RectTransform = Cast<URectTransformComponent>(GetOuter());
    if (!IsValid(RectTransform))
        return Size;
	
    return Size - GetParentSize()[Axis] * (RectTransform->GetAnchorMax()[Axis] - RectTransform->GetAnchorMin()[Axis]);
}

FVector2D UAspectRatioFitterSubComponent::GetParentSize() const
{
    const auto RectTransform = Cast<URectTransformComponent>(GetOuter());
    if (!IsValid(RectTransform))
        return FVector2D::ZeroVector;

    const URectTransformComponent* Parent = Cast<URectTransformComponent>(RectTransform->GetAttachParent());
	if (!IsValid(Parent))
		return FVector2D::ZeroVector;

    return Parent->GetRect().GetSize();
}

/////////////////////////////////////////////////////
