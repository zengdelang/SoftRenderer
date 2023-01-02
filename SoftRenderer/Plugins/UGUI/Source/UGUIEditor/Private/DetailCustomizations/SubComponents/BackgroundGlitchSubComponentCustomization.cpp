#include "DetailCustomizations/SubComponents/BackgroundGlitchSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FBackgroundGlitchSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FBackgroundGlitchSubComponentCustomization);
}

void FBackgroundGlitchSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("bUseGlitch"), CategoryBuilder);
	AddProperty(TEXT("Strength"), CategoryBuilder);
	AddProperty(TEXT("Method"), CategoryBuilder);
	AddProperty(TEXT("DownSampleAmount"), CategoryBuilder);
	AddProperty(TEXT("UIGlitchAnalogNoiseAndRGBSplitSet"), CategoryBuilder);
	AddProperty(TEXT("UIGlitchImageBlockSet"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
