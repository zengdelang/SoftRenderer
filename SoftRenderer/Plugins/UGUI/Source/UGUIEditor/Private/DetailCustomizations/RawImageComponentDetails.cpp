#include "DetailCustomizations/RawImageComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UIBlueprintEditorModule.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/RawImageComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FRawImageComponentDetails::MakeInstance()
{
	return MakeShareable(new FRawImageComponentDetails);
}

void FRawImageComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));
	
	/// Image Category
	IDetailCategoryBuilder& ImageCategory = DetailBuilder.EditCategory(TEXT("Raw Image"), FText(LOCTEXT("RawImageComponentCategory", "Raw Image")));

	AddRowHeaderContent(ImageCategory, DetailBuilder);
	
	const auto TextureProperty = DetailBuilder.GetProperty(TEXT("Texture"));
	TextureProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	ImageCategory.AddProperty(TEXT("Texture"));
	ImageCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("Material"), UGraphicComponent::StaticClass());	
	ImageCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("UVRect"));

	DetailBuilder.GetProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass())->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<URawImageComponent>(TargetObjects[0].Get());

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsAntiAliasing())
	{
		ImageCategory.AddProperty(TEXT("ImageMaskType"));

		DetailBuilder.GetProperty(TEXT("ImageMaskType"))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

		if (TargetScriptPtr->GetImageMaskMode() == ERawImageMaskMode::MaskMode_CircleRing)
		{
			ImageCategory.AddProperty(TEXT("Thickness"));
		}
	}

	if (TargetScriptPtr.IsValid() && IsValid(TargetScriptPtr->GetTexture()))
	{
		ImageCategory.AddProperty(TEXT("bPreserveAspect"));
		DetailBuilder.GetProperty(TEXT("bPreserveAspect"))->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		if (TargetScriptPtr->GetPreserveAspect())
		{
			ImageCategory.AddProperty(TEXT("bFillViewRect"));
			ImageCategory.AddProperty(TEXT("OverrideTextureSize"));
		}
		
		/// SetNativeSize
		const auto OnClickedLambda = FOnClicked::CreateLambda([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("RectTransformComponentDetailsSetNativeSize", "ChangeSetNativeSize", "Change SetNativeSize"));
		
				const auto AnchorMin = TargetScriptPtr->GetAnchorMin();
				const auto NativeSize = TargetScriptPtr->GetNativeSize();

				const auto AnchorMaxProperty = DetailBuilder.GetProperty(TEXT("AnchorMax"), URectTransformComponent::StaticClass());
				AnchorMaxProperty->SetValue(AnchorMin, EPropertyValueSetFlags::NotTransactable);

				const auto SizeDeltaProperty = DetailBuilder.GetProperty(TEXT("SizeDelta"), URectTransformComponent::StaticClass());
				SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);
				SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);
				
				IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
			}
			return FReply::Handled();
		});
		
		ImageCategory.AddCustomRow(LOCTEXT("RawImageSetNativeSizeRow", ""))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RawImageSetNativeSizeTitle", ""))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Light")
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked(OnClickedLambda)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RawImageSetNativeSize", "Set Native Size"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
	}
}

#undef LOCTEXT_NAMESPACE
