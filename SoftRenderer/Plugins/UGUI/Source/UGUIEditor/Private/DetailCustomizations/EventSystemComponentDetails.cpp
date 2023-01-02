#include "DetailCustomizations/EventSystemComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FEventSystemComponentDetails::MakeInstance()
{
	return MakeShareable(new FEventSystemComponentDetails);
}

void FEventSystemComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("EventSystem"));

	IDetailCategoryBuilder& EventSystemCategory = DetailBuilder.EditCategory(TEXT("Event System"), FText(LOCTEXT("EventSystemComponentCategory", "Event System")));

	AddRowHeaderContent(EventSystemCategory, DetailBuilder);

	EventSystemCategory.AddProperty(TEXT("DragThreshold"));
	EventSystemCategory.AddProperty(TEXT("bSendNavigationEvents"));
	EventSystemCategory.AddProperty(TEXT("bCheckMultipleComponents"));
}

#undef LOCTEXT_NAMESPACE