#include "DetailCustomizations/ToggleGroupComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FToggleGroupComponentDetails::MakeInstance()
{
	return MakeShareable(new FToggleGroupComponentDetails);
}

void FToggleGroupComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	
	IDetailCategoryBuilder& ToggleGroupCategory = DetailBuilder.EditCategory(TEXT("Toggle Group"), FText(LOCTEXT("ToggleGroupComponentCategory", "Toggle Group")));

	AddRowHeaderContent(ToggleGroupCategory, DetailBuilder);
	
	ToggleGroupCategory.AddProperty(TEXT("bAllowSwitchOff"));
}

#undef LOCTEXT_NAMESPACE