#include "DetailCustomizations/SubComponents/PositionAsUV1SubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FPositionAsUV1SubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FPositionAsUV1SubComponentCustomization);
}

void FPositionAsUV1SubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	CategoryBuilder.AddCustomRow(FText::GetEmpty());
}

#undef LOCTEXT_NAMESPACE
