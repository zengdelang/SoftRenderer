#include "DetailCustomizations/BackgroundGlitchComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FBackgroundGlitchComponentDetails::MakeInstance()
{
	return MakeShareable(new FBackgroundGlitchComponentDetails);
}

void FBackgroundGlitchComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// Background Glitch Category
	IDetailCategoryBuilder& BackgroundGlitchCategory = DetailBuilder.EditCategory(TEXT("Background Glitch"), FText(LOCTEXT("BackgroundGlitchComponentCategory", "Background Glitch")));

	AddRowHeaderContent(BackgroundGlitchCategory, DetailBuilder);
	
	BackgroundGlitchCategory.AddProperty(TEXT("bUseGlitch"));
	BackgroundGlitchCategory.AddProperty(TEXT("Strength"));
	BackgroundGlitchCategory.AddProperty(TEXT("Method"));
	BackgroundGlitchCategory.AddProperty(TEXT("DownSampleAmount"));
	BackgroundGlitchCategory.AddProperty(TEXT("UIGlitchAnalogNoiseAndRGBSplitSet"));
	BackgroundGlitchCategory.AddProperty(TEXT("UIGlitchImageBlockSet"));

}

#undef LOCTEXT_NAMESPACE
