#include "DetailCustomizations/SubComponents/BackgroundBlurSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FBackgroundBlurSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FBackgroundBlurSubComponentCustomization);
}

void FBackgroundBlurSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("Material"), CategoryBuilder);
	AddProperty(TEXT("Color"), CategoryBuilder);
	
	AddProperty(TEXT("BlurStrength"), CategoryBuilder);
	AddProperty(TEXT("BlurMaskType"), CategoryBuilder);
	AddProperty(TEXT("MaskTexture"), CategoryBuilder);
	AddProperty(TEXT("bApplyAlphaToBlur"), CategoryBuilder);

	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bAntiAliasing"), CategoryBuilder);
	AddProperty(TEXT("bMaskable"), CategoryBuilder);
	
	AddProperty(TEXT("BlurRadius"), CategoryBuilder, EPropertyLocation::Type::Advanced);
	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
