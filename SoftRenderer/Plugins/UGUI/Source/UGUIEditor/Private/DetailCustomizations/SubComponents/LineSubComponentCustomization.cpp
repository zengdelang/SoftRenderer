#include "DetailCustomizations/SubComponents//LineSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/LineSubComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FLineSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FLineSubComponentCustomization);
}

void FLineSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<ULineSubComponent>(SubComponent.Get());
	
	AddProperty(TEXT("Material"), CategoryBuilder);

	AddProperty(TEXT("PointColor"), CategoryBuilder);
	AddProperty(TEXT("PointSize"), CategoryBuilder);
	AddProperty(TEXT("PointMode"), CategoryBuilder);
	
	AddProperty(TEXT("Color"), CategoryBuilder)->DisplayName(FText(LOCTEXT("LineComponentCategory", "Line Color")));
	AddProperty(TEXT("LineThickness"), CategoryBuilder);
	
	AddProperty(TEXT("LineType"), CategoryBuilder)
			->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	AddProperty(TEXT("DrawLineTolerance"), CategoryBuilder);
	AddProperty(TEXT("DefaultSprite2D"), CategoryBuilder);
	
	if (TargetScriptPtr.IsValid() && (TargetScriptPtr->GetLineType() == ELineType::CustomPoints || TargetScriptPtr->GetLineType() == ELineType::RectCustomPoints))
	{
		AddProperty(TEXT("LinePoints"), CategoryBuilder);
	}

	AddProperty(TEXT("bClosedLine"), CategoryBuilder);
	
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bAntiAliasing"), CategoryBuilder);	
	AddProperty(TEXT("bMaskable"), CategoryBuilder);

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
