#include "DetailCustomizations/SubComponents/DropdownItemSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FDropdownItemSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FDropdownItemSubComponentCustomization);
}

void FDropdownItemSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("TogglePath"), CategoryBuilder);
	AddProperty(TEXT("ItemTextPath"), CategoryBuilder);
	AddProperty(TEXT("ItemImagePath"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
