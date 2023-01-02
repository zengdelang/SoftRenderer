#include "Core/Layout/HorizontalOrVerticalLayoutGroupComponent.h"
#include "Core/Layout/LayoutUtility.h"

/////////////////////////////////////////////////////
// UHorizontalOrVerticalLayoutGroupComponent

UHorizontalOrVerticalLayoutGroupComponent::UHorizontalOrVerticalLayoutGroupComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Spacing = 0;
	bChildForceExpandWidth = true;
	bChildForceExpandHeight = true;
	bChildControlWidth = false;
	bChildControlHeight = false;
	bChildScaleWidth = false;
	bChildScaleHeight = false;
}

void UHorizontalOrVerticalLayoutGroupComponent::CalcAlongAxis(int32 Axis, bool bIsVertical)
{
    const float CombinedPadding = (Axis == 0 ? Padding.GetTotalSpaceAlong<Orient_Horizontal>() : Padding.GetTotalSpaceAlong<Orient_Vertical>());
    const bool bControlSize = (Axis == 0 ? bChildControlWidth : bChildControlHeight);
    const bool bUseScale = (Axis == 0 ? bChildScaleWidth : bChildScaleHeight);
    const bool bChildForceExpand = (Axis == 0 ? bChildForceExpandWidth : bChildForceExpandHeight);

    float TotalMin = CombinedPadding;
    float TotalPreferred = CombinedPadding;
    float TotalFlexible = 0;

    const bool bAlongOtherAxis = (bIsVertical ^ (Axis == 1));
    for (int32 Index = 0, Count = RectChildren.Num(); Index < Count; ++Index)
    {
        const auto RectChild = RectChildren[Index];
        float Min, Preferred, Flexible;
        GetChildSizes(RectChild, Axis, bControlSize, bChildForceExpand, Min, Preferred, Flexible);

        if (bUseScale)
        {
            const float ScaleFactor = RectChild->GetRelativeScale3D()[Axis];
            Min *= ScaleFactor;
            Preferred *= ScaleFactor;
            Flexible *= ScaleFactor;
        }

        if (bAlongOtherAxis)
        {
            TotalMin = FMath::Max(Min + CombinedPadding, TotalMin);
            TotalPreferred = FMath::Max(Preferred + CombinedPadding, TotalPreferred);
            TotalFlexible = FMath::Max(Flexible, TotalFlexible);
        }
        else
        {
            TotalMin += Min + Spacing;
            TotalPreferred += Preferred + Spacing;

            // Increment flexible size with element's flexible size.
            TotalFlexible += Flexible;
        }
    }

    if (!bAlongOtherAxis && RectChildren.Num() > 0)
    {
        TotalMin -= Spacing;
        TotalPreferred -= Spacing;
    }
	
    TotalPreferred = FMath::Max(TotalMin, TotalPreferred);
    SetLayoutInputForAxis(TotalMin, TotalPreferred, TotalFlexible, Axis);
}

void UHorizontalOrVerticalLayoutGroupComponent::SetChildrenAlongAxis(int32 Axis, bool bIsVertical)
{
    const float Size = Rect.GetSize()[Axis];
    const bool bControlSize = (Axis == 0 ? bChildControlWidth : bChildControlHeight);
    const bool bUseScale = (Axis == 0 ? bChildScaleWidth : bChildScaleHeight);
    const bool bChildForceExpandSize = (Axis == 0 ? bChildForceExpandWidth : bChildForceExpandHeight);
    const float AlignmentOnAxis = GetAlignmentOnAxis(Axis);

    const bool bAlongOtherAxis = (bIsVertical ^ (Axis == 1));
    if (bAlongOtherAxis)
    {
        const float InnerSize = Size - (Axis == 0 ? Padding.GetTotalSpaceAlong<Orient_Horizontal>() : Padding.GetTotalSpaceAlong<Orient_Vertical>());
        for (int32 Index = 0, Count = RectChildren.Num(); Index < Count; ++Index)
        {
            const auto RectChild = RectChildren[Index];
            float Min, Preferred, Flexible;
            GetChildSizes(RectChild, Axis, bControlSize, bChildForceExpandSize, Min, Preferred, Flexible);
            const float ScaleFactor = bUseScale ? RectChild->GetRelativeScale3D()[Axis] : 1;

            const float RequiredSpace = FMath::Clamp(InnerSize, Min, Flexible > 0 ? Size : Preferred);
            const float StartOffset = GetStartOffset(Axis, RequiredSpace * ScaleFactor);
            if (bControlSize)
            {
                SetChildAlongAxisWithScale(RectChild, Axis, StartOffset, RequiredSpace, ScaleFactor);
            }
            else
            {
                const float OffsetInCell = (RequiredSpace - RectChild->GetSizeDelta()[Axis]) * AlignmentOnAxis;
                SetChildAlongAxisWithScale(RectChild, Axis, StartOffset + OffsetInCell, ScaleFactor);
            }
        }
    }
    else
    {
        float Pos = (Axis == 0 ? Padding.Left : Padding.Top);
        float ItemFlexibleMultiplier = 0;
        const float SurplusSpace = Size - GetTotalPreferredSize(Axis);

        if (SurplusSpace > 0)
        {
            if (GetTotalFlexibleSize(Axis) == 0)
                Pos = GetStartOffset(Axis, GetTotalPreferredSize(Axis) - (Axis == 0 ? Padding.GetTotalSpaceAlong<Orient_Horizontal>() : Padding.GetTotalSpaceAlong<Orient_Vertical>()));
            else if (GetTotalFlexibleSize(Axis) > 0)
                ItemFlexibleMultiplier = SurplusSpace / GetTotalFlexibleSize(Axis);
        }

        float MinMaxLerp = 0;
        if (GetTotalMinSize(Axis) != GetTotalPreferredSize(Axis))
            MinMaxLerp = FMath::Clamp((Size - GetTotalMinSize(Axis)) / (GetTotalPreferredSize(Axis) - GetTotalMinSize(Axis)), 0.0f, 1.0f);

        for (int32 Index = 0, Count = RectChildren.Num(); Index < Count; ++Index)
        {
            const auto RectChild = RectChildren[Index];
            float Min, Preferred, Flexible;
            GetChildSizes(RectChild, Axis, bControlSize, bChildForceExpandSize,  Min,  Preferred,  Flexible);
            const float ScaleFactor = bUseScale ? RectChild->GetRelativeScale3D()[Axis] : 1;

            float ChildSize = FMath::Lerp(Min, Preferred, MinMaxLerp);
            ChildSize += Flexible * ItemFlexibleMultiplier;
            if (bControlSize)
            {
                SetChildAlongAxisWithScale(RectChild, Axis, Pos, ChildSize, ScaleFactor);
            }
            else
            {
                const float OffsetInCell = (ChildSize - RectChild->GetSizeDelta()[Axis]) * AlignmentOnAxis;
                SetChildAlongAxisWithScale(RectChild, Axis, Pos + OffsetInCell, ScaleFactor);
            }
            Pos += ChildSize * ScaleFactor + Spacing;
        }
    }
}

void UHorizontalOrVerticalLayoutGroupComponent::GetChildSizes(URectTransformComponent* Child, int32 Axis, bool bControlSize, bool bChildForceExpand, float& Min, float& Preferred, float& Flexible) const
{
    if (!bControlSize)
    {
        Min = Child->GetSizeDelta()[Axis];
        Preferred = Min;
        Flexible = 0;
    }
    else
    {
        Min = FLayoutUtility::GetMinSize(Child, Axis);
        Preferred = FLayoutUtility::GetPreferredSize(Child, Axis);
        Flexible = FLayoutUtility::GetFlexibleSize(Child, Axis);
    }

    if (bChildForceExpand)
    {
        Flexible = FMath::Max(Flexible, 1.0f);
    }
}

/////////////////////////////////////////////////////
