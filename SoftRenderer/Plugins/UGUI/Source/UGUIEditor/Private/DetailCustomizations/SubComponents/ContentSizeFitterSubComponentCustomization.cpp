#include "DetailCustomizations/SubComponents/ContentSizeFitterSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FContentSizeFitterSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FContentSizeFitterSubComponentCustomization);
}

void FContentSizeFitterSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("HorizontalFit"), CategoryBuilder);
	AddProperty(TEXT("VerticalFit"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
