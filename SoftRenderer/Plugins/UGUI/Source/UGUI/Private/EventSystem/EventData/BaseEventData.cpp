#include "EventSystem/EventData/BaseEventData.h"
#include "EventSystem/EventSystemComponent.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UBaseEventData

UBaseEventData::UBaseEventData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UBaseInputModuleSubComponent* UBaseEventData::GetCurrentInputModule() const
{
	if (IsValid(EventSystem))
	{
		return EventSystem->GetCurrentInputModule();
	}
	return nullptr;
}

USceneComponent* UBaseEventData::GetSelectedObject() const
{
	if (IsValid(EventSystem))
	{
		return EventSystem->GetCurrentSelectedGameObject();
	}
	return nullptr;
}

void UBaseEventData::SetSelectedObject(USceneComponent* InSelectedObject)
{
	if (IsValid(EventSystem))
	{
		EventSystem->SetSelectedGameObject(InSelectedObject, this);
	}
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
