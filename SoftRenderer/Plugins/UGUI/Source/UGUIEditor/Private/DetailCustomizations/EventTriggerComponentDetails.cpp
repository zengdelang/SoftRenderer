#include "DetailCustomizations/EventTriggerComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "EventSystem/EventTriggerComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FEventTriggerComponentDetails::MakeInstance()
{
	return MakeShareable(new FEventTriggerComponentDetails);
}

void FEventTriggerComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	
	IDetailCategoryBuilder& EventTriggerCategory = DetailBuilder.EditCategory(TEXT("Event Trigger"),FText(LOCTEXT("EventTriggerComponentCategory","Event Trigger")),ECategoryPriority::Important);

	AddRowHeaderContent(EventTriggerCategory, DetailBuilder);
	
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnPointerEnterDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnPointerExitDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnPointerDownDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnPointerUpDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnPointerClickDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnInitializePotentialDragDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnBeginDragDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnDragDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnEndDragDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnDropDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnScrollDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnUpdateSelectedDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnSelectDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnDeselectDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnMoveDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnSubmitDelegate"));
	AddEventProperty(EventTriggerCategory, DetailBuilder, TEXT("OnCancelDelegate"));
}

#undef LOCTEXT_NAMESPACE