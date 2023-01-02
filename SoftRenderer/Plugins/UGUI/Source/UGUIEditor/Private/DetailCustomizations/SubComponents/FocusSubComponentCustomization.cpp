#include "DetailCustomizations/SubComponents/FocusSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FFocusSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FFocusSubComponentCustomization);
}

void FFocusSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	CategoryBuilder.AddCustomRow(FText::GetEmpty());
}

#undef LOCTEXT_NAMESPACE
