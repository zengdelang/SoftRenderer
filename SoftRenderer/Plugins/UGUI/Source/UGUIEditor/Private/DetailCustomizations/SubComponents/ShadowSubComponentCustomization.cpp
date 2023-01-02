#include "DetailCustomizations/SubComponents/ShadowSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/VertexModifiers/ShadowSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FShadowSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FShadowSubComponentCustomization);
}

void FShadowSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("bUseExternalEffect"), CategoryBuilder)->GetPropertyHandle()
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TargetScriptPtr = Cast<UShadowSubComponent>(SubComponent.Get());
	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsUseExternalEffect())
	{
		AddProperty(TEXT("ExternalEffectType"), CategoryBuilder);
	}
	else
	{
		AddProperty(TEXT("EffectColor"), CategoryBuilder);
		AddProperty(TEXT("EffectDistance"), CategoryBuilder);
	}
	
	AddProperty(TEXT("bUseGraphicAlpha"), CategoryBuilder);
	
	AddProperty(TEXT("bFollowTransform"), CategoryBuilder)->GetPropertyHandle()
	->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsFollowTransform())
	{
		AddProperty(TEXT("FixedLocalLocation"), CategoryBuilder);
	}
}

#undef LOCTEXT_NAMESPACE
