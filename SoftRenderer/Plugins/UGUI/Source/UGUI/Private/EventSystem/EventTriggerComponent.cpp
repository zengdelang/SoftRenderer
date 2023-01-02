#include "EventSystem/EventTriggerComponent.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UEventTrigger

UEventTriggerComponent::UEventTriggerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UEventTriggerComponent::OnPointerEnter(UPointerEventData* EventData)
{
	OnPointerEnterDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnPointerExit(UPointerEventData* EventData)
{
	OnPointerExitDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnPointerDown(UPointerEventData* EventData)
{
	OnPointerDownDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnPointerUp(UPointerEventData* EventData)
{
	OnPointerUpDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnPointerClick(UPointerEventData* EventData)
{
	OnPointerClickDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnInitializePotentialDrag(UPointerEventData* EventData)
{
	OnInitializePotentialDragDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnBeginDrag(UPointerEventData* EventData)
{
	OnBeginDragDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnDrag(UPointerEventData* EventData)
{
	OnDragDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnEndDrag(UPointerEventData* EventData)
{
	OnEndDragDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnDrop(UPointerEventData* EventData)
{
	OnDropDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnScroll(UPointerEventData* EventData)
{
	OnScrollDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnUpdateSelected(UBaseEventData* EventData)
{
	OnUpdateSelectedDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnSelect(UBaseEventData* EventData)
{
	OnSelectDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnDeselect(UBaseEventData* EventData)
{
	OnDeselectDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnMove(UAxisEventData* EventData)
{
	OnMoveDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnSubmit(UBaseEventData* EventData)
{
	OnSubmitDelegate.Broadcast(EventData);
}

void UEventTriggerComponent::OnCancel(UBaseEventData* EventData)
{
	OnCancelDelegate.Broadcast(EventData);
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
