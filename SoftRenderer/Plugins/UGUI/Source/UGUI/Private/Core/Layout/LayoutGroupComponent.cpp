#include "Core/Layout/LayoutGroupComponent.h"
#include "Core/Layout/LayoutIgnorerInterface.h"

/////////////////////////////////////////////////////
// ULayoutGroupComponent

ULayoutGroupComponent::ULayoutGroupComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ChildAlignment = ETextAnchor::TextAnchor_UpperLeft; 
    TotalMinSize = FVector2D::ZeroVector;
    TotalPreferredSize = FVector2D::ZeroVector;
    TotalFlexibleSize = FVector2D::ZeroVector;
}

#if WITH_EDITORONLY_DATA

void ULayoutGroupComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    SetDirty();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ULayoutGroupComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    SetDirty();
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif

void ULayoutGroupComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    FCoreDelegates::OnEndFrame.Remove(EndFrameDelegateHandle);
    EndFrameDelegateHandle.Reset();
	
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void ULayoutGroupComponent::OnEnable()
{
    Super::OnEnable();
	
    SetDirty();
}

void ULayoutGroupComponent::OnDisable()
{
	FLayoutRebuilder::MarkLayoutForRebuild(this);
	
    Super::OnDisable();
}

void ULayoutGroupComponent::OnRectTransformDimensionsChange()
{
    Super::OnRectTransformDimensionsChange();
	
    if (IsRootLayoutGroup())
		SetDirty();
}

void ULayoutGroupComponent::OnTransformParentChanged()
{
    Super::OnTransformParentChanged();

    SetDirty();
}

void ULayoutGroupComponent::OnChildAttachmentChanged()
{
	Super::OnChildAttachmentChanged();

	SetDirty();
}

void ULayoutGroupComponent::CalculateLayoutInputHorizontal()
{
    const auto& RectAttachChildren = GetAttachChildren();

    RectChildren.Empty();
    RectChildren.Reserve(RectAttachChildren.Num());
	
    for (int32 Index = 0, Count = RectAttachChildren.Num(); Index < Count; ++Index)
    {
        const auto RectChild = Cast<URectTransformComponent>(RectAttachChildren[Index]);
        if (!IsValid(RectChild) || !RectChild->IsActiveAndEnabled())
            continue;

        TArray<ILayoutIgnorerInterface*, TInlineAllocator<8>> ToIgnoreList;
        RectChild->GetComponents(ToIgnoreList);

        if (ToIgnoreList.Num() == 0)
        {
            RectChildren.Add(RectChild);
            continue;
        }

        for (int32 IgnoreIndex = 0, IgnoreCount = ToIgnoreList.Num(); IgnoreIndex < IgnoreCount; ++IgnoreIndex)
        {
            const auto LayoutIgnorer = Cast<ILayoutIgnorerInterface>(ToIgnoreList[IgnoreIndex]);
            if (LayoutIgnorer && !LayoutIgnorer->IgnoreLayout())
            {
                RectChildren.Add(RectChild);
                break;
            }
        }
    }
}

void ULayoutGroupComponent::CalculateLayoutInputVertical()
{
}

float ULayoutGroupComponent::GetMinWidth()
{
    return GetTotalMinSize(0);
}

float ULayoutGroupComponent::GetPreferredWidth()
{
    return GetTotalPreferredSize(0);
}

float ULayoutGroupComponent::GetFlexibleWidth()
{
    return GetTotalFlexibleSize(0);
}

float ULayoutGroupComponent::GetMinHeight()
{
    return GetTotalMinSize(1);
}

float ULayoutGroupComponent::GetPreferredHeight()
{
    return GetTotalPreferredSize(1);
}

float ULayoutGroupComponent::GetFlexibleHeight()
{
    return GetTotalFlexibleSize(1);
}

int32 ULayoutGroupComponent::GetLayoutPriority()
{
    return 0;
}

void ULayoutGroupComponent::SetLayoutHorizontal()
{
}

void ULayoutGroupComponent::SetLayoutVertical()
{
}

/////////////////////////////////////////////////////
