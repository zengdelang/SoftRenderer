#include "Core/Layout/LayoutGroupSubComponent.h"
#include "Core/Layout/LayoutIgnorerInterface.h"

/////////////////////////////////////////////////////
// ULayoutGroupSubComponent

ULayoutGroupSubComponent::ULayoutGroupSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    ChildAlignment = ETextAnchor::TextAnchor_UpperLeft; 
	TotalMinSize = FVector2D::ZeroVector;
	TotalPreferredSize = FVector2D::ZeroVector;
	TotalFlexibleSize = FVector2D::ZeroVector;
}

#if WITH_EDITORONLY_DATA

void ULayoutGroupSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    SetDirty();
    Super::PostEditChangeProperty(PropertyChangedEvent);
}

void ULayoutGroupSubComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    SetDirty();
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif

void ULayoutGroupSubComponent::OnEnable()
{
    Super::OnEnable();
	
    SetDirty();
}

void ULayoutGroupSubComponent::OnDisable()
{
	FLayoutRebuilder::MarkLayoutForRebuild(AttachTransform);
	
    Super::OnDisable();
}

void ULayoutGroupSubComponent::OnDestroy()
{
    FCoreDelegates::OnEndFrame.Remove(EndFrameDelegateHandle);
    EndFrameDelegateHandle.Reset();
    
    Super::OnDestroy();
}

void ULayoutGroupSubComponent::OnRectTransformDimensionsChange()
{
    Super::OnRectTransformDimensionsChange();
	
    if (IsRootLayoutGroup())
		SetDirty();
}

void ULayoutGroupSubComponent::OnTransformParentChanged()
{
    Super::OnTransformParentChanged();

    SetDirty();
}

void ULayoutGroupSubComponent::OnChildAttachmentChanged()
{
	Super::OnChildAttachmentChanged();

	SetDirty();
}

void ULayoutGroupSubComponent::CalculateLayoutInputHorizontal()
{
    if (!IsValid(AttachTransform))
        return;
    
    const auto& RectAttachChildren = AttachTransform->GetAttachChildren();

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

void ULayoutGroupSubComponent::CalculateLayoutInputVertical()
{
}

float ULayoutGroupSubComponent::GetMinWidth()
{
    return GetTotalMinSize(0);
}

float ULayoutGroupSubComponent::GetPreferredWidth()
{
    return GetTotalPreferredSize(0);
}

float ULayoutGroupSubComponent::GetFlexibleWidth()
{
    return GetTotalFlexibleSize(0);
}

float ULayoutGroupSubComponent::GetMinHeight()
{
    return GetTotalMinSize(1);
}

float ULayoutGroupSubComponent::GetPreferredHeight()
{
    return GetTotalPreferredSize(1);
}

float ULayoutGroupSubComponent::GetFlexibleHeight()
{
    return GetTotalFlexibleSize(1);
}

int32 ULayoutGroupSubComponent::GetLayoutPriority()
{
    return 0;
}

void ULayoutGroupSubComponent::SetLayoutHorizontal()
{
}

void ULayoutGroupSubComponent::SetLayoutVertical()
{
}

/////////////////////////////////////////////////////
