#include "DetailCustomizations/SubComponents/CanvasRendererSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FCanvasRendererSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FCanvasRendererSubComponentCustomization);
}

void FCanvasRendererSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("bHidePrimitive"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
