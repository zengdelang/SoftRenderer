#include "DetailCustomizations/SubComponents/TweenPlayerSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FTweenPlayerSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FTweenPlayerSubComponentCustomization);
}

void FTweenPlayerSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

#if WITH_EDITOR
	AddProperty(TEXT("bEditPreview"), CategoryBuilder);
#endif
	AddProperty(TEXT("TweenGroupName"), CategoryBuilder);
	//AddProperty(TEXT("TweenGroup"), CategoryBuilder);
	AddProperty(TEXT("bAutoPlay"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
