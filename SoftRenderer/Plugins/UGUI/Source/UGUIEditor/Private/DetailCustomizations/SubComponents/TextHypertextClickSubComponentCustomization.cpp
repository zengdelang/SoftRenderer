#include "DetailCustomizations/SubComponents/TextHypertextClickSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FTextHypertextClickSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FTextHypertextClickSubComponentCustomization);
}

void FTextHypertextClickSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	CategoryBuilder.AddCustomRow(FText::GetEmpty());
}

#undef LOCTEXT_NAMESPACE
