#include "DetailCustomizations/SubComponents/RectMask2DSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FRectMask2DSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FRectMask2DSubComponentCustomization);
}

void FRectMask2DSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("Padding"), CategoryBuilder);
	AddProperty(TEXT("Softness"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
