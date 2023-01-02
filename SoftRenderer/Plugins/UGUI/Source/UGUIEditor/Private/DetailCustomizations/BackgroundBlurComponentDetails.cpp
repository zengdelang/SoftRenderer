#include "DetailCustomizations/BackgroundBlurComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FBackgroundBlurComponentDetails::MakeInstance()
{
	return MakeShareable(new FBackgroundBlurComponentDetails);
}

void FBackgroundBlurComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// Background Blur Category
	IDetailCategoryBuilder& BackgroundBlurCategory = DetailBuilder.EditCategory(TEXT("Background Blur"), FText(LOCTEXT("BackgroundBlurComponentCategory", "Background Blur")));

	AddRowHeaderContent(BackgroundBlurCategory, DetailBuilder);
	
	BackgroundBlurCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass());
	BackgroundBlurCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	BackgroundBlurCategory.AddProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass());
	BackgroundBlurCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());

	BackgroundBlurCategory.AddProperty(TEXT("BlurStrength"));
	BackgroundBlurCategory.AddProperty(TEXT("bApplyAlphaToBlur"));
	BackgroundBlurCategory.AddProperty(TEXT("BlurRadius"), nullptr, NAME_None, EPropertyLocation::Type::Advanced);
	BackgroundBlurCategory.AddProperty(TEXT("BlurMaskType"));
	BackgroundBlurCategory.AddProperty(TEXT("MaskTexture"));
}

#undef LOCTEXT_NAMESPACE
