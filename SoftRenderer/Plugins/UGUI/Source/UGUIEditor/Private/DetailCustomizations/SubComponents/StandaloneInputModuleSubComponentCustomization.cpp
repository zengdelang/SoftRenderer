#include "DetailCustomizations/SubComponents/StandaloneInputModuleSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FStandaloneInputModuleSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FStandaloneInputModuleSubComponentCustomization);
}

void FStandaloneInputModuleSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("HorizontalAxis"), CategoryBuilder);
	AddProperty(TEXT("VerticalAxis"), CategoryBuilder);
	AddProperty(TEXT("SubmitButton"), CategoryBuilder);
	AddProperty(TEXT("CancelButton"), CategoryBuilder);
	AddProperty(TEXT("InputActionsPerSecond"), CategoryBuilder);
	AddProperty(TEXT("RepeatDelay"), CategoryBuilder);
	AddProperty(TEXT("bForceModuleActive"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
