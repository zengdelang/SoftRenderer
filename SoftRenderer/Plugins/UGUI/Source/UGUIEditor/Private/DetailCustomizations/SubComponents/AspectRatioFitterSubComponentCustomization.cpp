#include "DetailCustomizations/SubComponents/AspectRatioFitterSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FAspectRatioFitterSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAspectRatioFitterSubComponentCustomization);
}

void FAspectRatioFitterSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("AspectMode"), CategoryBuilder);
	AddProperty(TEXT("AspectRatio"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
