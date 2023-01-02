#include "DetailCustomizations/SubComponents/TweenBaseSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "Animation/TweenBaseSubComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FTweenBaseSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FTweenBaseSubComponentCustomization);
}

void FTweenBaseSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
#if WITH_EDITOR
	AddProperty(TEXT("bEditPreview"), CategoryBuilder);
#endif
	AddProperty(TEXT("TweenGroupName"), CategoryBuilder);
	//AddProperty(TEXT("TweenGroup"), CategoryBuilder);
	AddProperty(TEXT("TweenGroupIndex"), CategoryBuilder);
	AddProperty(TEXT("bAutoPlay"), CategoryBuilder);
	//AddProperty(TEXT("bUseScreenPosition"), CategoryBuilder);
	AddProperty(TEXT("bFromCurrent"), CategoryBuilder)->GetPropertyHandle()
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	TargetScriptPtr = Cast<UTweenBaseSubComponent>(SubComponent.Get());
	if (TargetScriptPtr.IsValid() && !(TargetScriptPtr->IsFromCurrent()))
	{
		AddProperty(TEXT("From"), CategoryBuilder);
	}
	AddProperty(TEXT("To"), CategoryBuilder);
	AddProperty(TEXT("StartDelay"), CategoryBuilder);
	AddProperty(TEXT("Duration"), CategoryBuilder);
	AddProperty(TEXT("bIgnoreTimeScale"), CategoryBuilder);
	AddProperty(TEXT("Style"), CategoryBuilder);
	AddProperty(TEXT("EasingFunc"), CategoryBuilder)->GetPropertyHandle()
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr.IsValid() && ETweenEasingFunc::TweenEasingFunc_CustomCurve == TargetScriptPtr->GetEasingFunc())
	{
		AddProperty(TEXT("AnimationCurve"), CategoryBuilder);
	}
	AddProperty(TEXT("EndWay"), CategoryBuilder);
}

#undef LOCTEXT_NAMESPACE
