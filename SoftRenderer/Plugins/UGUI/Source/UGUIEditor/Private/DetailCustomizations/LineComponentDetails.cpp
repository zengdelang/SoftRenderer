#include "DetailCustomizations/LineComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/LineComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FLineComponentDetails::MakeInstance()
{
	return MakeShareable(new FLineComponentDetails);
}

void FLineComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<ULineComponent>(TargetObjects[0].Get());
	
	/// Line Category
	IDetailCategoryBuilder& LineCategory = DetailBuilder.EditCategory(TEXT("Line"), FText(LOCTEXT("LineComponentCategory", "Line")));

	AddRowHeaderContent(LineCategory, DetailBuilder);
	
	LineCategory.AddProperty(TEXT("Material"), UGraphicComponent::StaticClass());
	LineCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	LineCategory.AddProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass());
	LineCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	LineCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass()).DisplayName(FText(LOCTEXT("LineComponentCategoryColor", "Line Color")));
	LineCategory.AddProperty(TEXT("bClosedLine"), ULineComponent::StaticClass());
	LineCategory.AddProperty(TEXT("LineThickness"), ULineComponent::StaticClass());
	LineCategory.AddProperty(TEXT("PointColor"), ULineComponent::StaticClass());
	LineCategory.AddProperty(TEXT("PointSize"), ULineComponent::StaticClass());
	LineCategory.AddProperty(TEXT("PointMode"), ULineComponent::StaticClass());

	LineCategory.AddProperty(TEXT("LineType"), ULineComponent::StaticClass());
	DetailBuilder.GetProperty(TEXT("LineType"))
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	LineCategory.AddProperty(TEXT("DrawLineTolerance"), ULineComponent::StaticClass());
	LineCategory.AddProperty(TEXT("Sprite"), ULineComponent::StaticClass());
	
	if (TargetScriptPtr.IsValid() && (TargetScriptPtr->GetLineType() == ELineType::CustomPoints || TargetScriptPtr->GetLineType() == ELineType::RectCustomPoints))
	{
		LineCategory.AddProperty(TEXT("LinePoints"), ULineComponent::StaticClass());
	}
}

#undef LOCTEXT_NAMESPACE
