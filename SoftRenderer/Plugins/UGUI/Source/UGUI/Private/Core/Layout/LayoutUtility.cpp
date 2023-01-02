#include "Core/Layout/LayoutUtility.h"

float FLayoutUtility::GetMinSize(URectTransformComponent* Rect, int32 Axis)
{
    if (Axis == 0)
        return GetMinWidth(Rect);
    return GetMinHeight(Rect);
}

float FLayoutUtility::GetPreferredSize(URectTransformComponent* Rect, int32 Axis)
{
    if (Axis == 0)
        return GetPreferredWidth(Rect);
    return GetPreferredHeight(Rect);
}

float FLayoutUtility::GetFlexibleSize(URectTransformComponent* Rect, int32 Axis)
{
    if (Axis == 0)
        return GetFlexibleWidth(Rect);
    return GetFlexibleHeight(Rect);
}

float FLayoutUtility::GetMinWidth(URectTransformComponent* Rect)
{
    return GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float{ return Element->GetMinWidth(); }, 0);
}

float FLayoutUtility::GetPreferredWidth(URectTransformComponent* Rect)
{
    return FMath::Max(GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetMinWidth(); }, 0), GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetPreferredWidth(); }, 0));
}

float FLayoutUtility::GetFlexibleWidth(URectTransformComponent* Rect)
{
    return GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetFlexibleWidth(); }, 0);
}

float FLayoutUtility::GetMinHeight(URectTransformComponent* Rect)
{
    return GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetMinHeight(); }, 0);
}

float FLayoutUtility::GetPreferredHeight(URectTransformComponent* Rect)
{
    return FMath::Max(GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetMinHeight(); }, 0), GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetPreferredHeight(); }, 0));
}

float FLayoutUtility::GetFlexibleHeight(URectTransformComponent* Rect)
{
    return GetLayoutProperty(Rect, [](ILayoutElementInterface* Element)->float { return Element->GetFlexibleHeight(); }, 0);
}

float FLayoutUtility::GetLayoutProperty(URectTransformComponent* Rect, FPropertyFinder PropertyFinder, float DefaultValue)
{
    if (!IsValid(Rect) || !Rect->IsActiveAndEnabled())
        return 0;

    float Min = DefaultValue;
    int32 MaxPriority = MIN_int32;

    TArray<ILayoutElementInterface*, TInlineAllocator<16>> Components;
    Rect->GetComponents(Components);

    for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
    {
        const auto LayoutComp = Components[Index];

        const int32 Priority = LayoutComp->GetLayoutPriority();
        // If this layout components has lower priority than a previously used, ignore it.
        if (Priority < MaxPriority)
            continue;

        const float Prop = (*PropertyFinder)(LayoutComp);
        // If this layout property is set to a negative value, it means it should be ignored.
        if (Prop < 0)
            continue;

        // If this layout component has higher priority than all previous ones,
        // overwrite with this one's value.
        if (Priority > MaxPriority)
        {
            Min = Prop;
            MaxPriority = Priority;
        }
        // If the layout component has the same priority as a previously used,
        // use the largest of the values with the same priority.
        else if (Prop > Min)
        {
            Min = Prop;
        }
    }

    return Min;
}
