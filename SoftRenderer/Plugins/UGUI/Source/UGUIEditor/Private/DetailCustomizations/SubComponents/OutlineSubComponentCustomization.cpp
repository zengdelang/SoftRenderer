#include "DetailCustomizations/SubComponents/OutlineSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/VertexModifiers/OutlineSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

class UOutlineSubComponent;

TSharedRef<ISubComponentDetailCustomization> FOutlineSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FOutlineSubComponentCustomization);
}

void FOutlineSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("bUseExternalEffect"), CategoryBuilder)->GetPropertyHandle()
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TargetScriptPtr = Cast<UOutlineSubComponent>(SubComponent.Get());
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
}

#undef LOCTEXT_NAMESPACE
