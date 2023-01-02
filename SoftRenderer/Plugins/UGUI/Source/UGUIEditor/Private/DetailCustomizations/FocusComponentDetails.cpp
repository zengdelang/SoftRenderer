#include "DetailCustomizations/FocusComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FFocusComponentDetails::MakeInstance()
{
	return MakeShareable(new FFocusComponentDetails);
}

void FFocusComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& FocusCategory = DetailBuilder.EditCategory(TEXT("Focus"), FText(LOCTEXT("FocusComponentCategory", "Focus")));

	AddRowHeaderContent(FocusCategory, DetailBuilder);
	FocusCategory.AddCustomRow(FText::GetEmpty());
}

#undef LOCTEXT_NAMESPACE