#include "DetailCustomizations/ImageComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UIBlueprintEditorModule.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/ImageComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FImageComponentDetails::MakeInstance()
{
	return MakeShareable(new FImageComponentDetails);
}

void FImageComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// Image Category
	IDetailCategoryBuilder& ImageCategory = DetailBuilder.EditCategory(TEXT("Image"), FText(LOCTEXT("ImageComponentCategory", "Image")));

	AddRowHeaderContent(ImageCategory, DetailBuilder);
	
	const auto SourceImageProperty = DetailBuilder.GetProperty(TEXT("Sprite"));
	SourceImageProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UImageComponent>(TargetObjects[0].Get());
	
	ImageCategory.AddProperty(TEXT("Sprite"));
	ImageCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("Material"), UGraphicComponent::StaticClass());	
	ImageCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	ImageCategory.AddProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass());
	
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple)
		{
			DetailBuilder.GetProperty(TEXT("bAntiAliasing"), UGraphicComponent::StaticClass())
				->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsAntiAliasing())
			{
				ImageCategory.AddProperty(TEXT("ImageMaskType"));

				DetailBuilder.GetProperty(TEXT("ImageMaskType"))
					->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

				if (TargetScriptPtr->GetImageMaskMode() == EImageMaskMode::MaskMode_CircleRing)
				{
					ImageCategory.AddProperty(TEXT("Thickness"));
				}
			}
		}
		else
		{
			ImageCategory.AddProperty(TEXT("ImageMaskType"));

			DetailBuilder.GetProperty(TEXT("ImageMaskType"))
				->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			if (TargetScriptPtr->GetImageMaskMode() == EImageMaskMode::MaskMode_CircleRing)
			{
				ImageCategory.AddProperty(TEXT("Thickness"));
			}
		}
	}
	
	ImageCategory.AddProperty(TEXT("AlphaHitTestMinimumThreshold"));
	
	if (TargetScriptPtr.IsValid() && IsValid(TargetScriptPtr->GetSprite()))
	{
		ImageCategory.AddProperty(TEXT("ImageType"));
		DetailBuilder.GetProperty(TEXT("ImageType"))
			->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple)
		{
			/// Use Sprite Mesh
			// TODO
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Filled)
		{
			/// FillMethod
			DetailBuilder.GetProperty(TEXT("FillMethod"))
				->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
			ImageCategory.AddProperty(TEXT("FillMethod")).DisplayName(LOCTEXT("ImageSubFillMethodTitle", "\tFill Method"));
			
			/// FillOrigin
			const auto OriginHorizontalProperty = DetailBuilder.GetProperty(TEXT("OriginHorizontal"));
			const auto OriginVerticalProperty = DetailBuilder.GetProperty(TEXT("OriginVertical"));
			const auto Origin90Property = DetailBuilder.GetProperty(TEXT("Origin90"));
			const auto Origin180Property = DetailBuilder.GetProperty(TEXT("Origin180"));
			const auto Origin360Property = DetailBuilder.GetProperty(TEXT("Origin360"));
			ImageCategory.AddCustomRow(LOCTEXT("ImageFillOriginRow", "FillOrigin"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ImageFillOriginTitle", "\tFill Origin"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.ValueContent()
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(static_cast<int32>(TargetScriptPtr->GetFillMethod()))
					+ SWidgetSwitcher::Slot()
					[
						OriginHorizontalProperty->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						OriginVerticalProperty->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin90Property->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin180Property->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin360Property->CreatePropertyValueWidget()
					]
				];

			/// FillAmount
			ImageCategory.AddProperty(TEXT("FillAmount")).DisplayName(LOCTEXT("ImageFillAmountTitle", "\tFill Amount"));
			
			/// Clockwise
			ImageCategory.AddProperty(TEXT("bFillClockwise")).DisplayName(LOCTEXT("ImageClockwiseTitle", "\tClockwise"));
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Tiled || 
			TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Sliced)
		{
			/// FillCenter
			ImageCategory.AddProperty(TEXT("bFillCenter")).DisplayName(LOCTEXT("ImageFillCenterTitle", "\tFill Center"));

			/// PixelsPerUnitMultiplier
			ImageCategory.AddProperty(TEXT("PixelsPerUnitMultiplier")).DisplayName(LOCTEXT("ImagePixelsPerUnitMultiplierTitle", "\tPixels Per Unit Multiplier"));

			ImageCategory.AddProperty(TEXT("OverrideBorder"));	
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple ||
			TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Filled)
		{
			/// PreserveAspect
			const auto PreserveAspectProperty = DetailBuilder.GetProperty(TEXT("bPreserveAspect"));
			PreserveAspectProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			ImageCategory.AddProperty(TEXT("bPreserveAspect")).DisplayName(LOCTEXT("ImagePreserveAspectTitle", "\tPreserve Aspect"));

			if (TargetScriptPtr->GetPreserveAspect())
			{
				/// FillViewRect
				ImageCategory.AddProperty(TEXT("bFillViewRect")).DisplayName(LOCTEXT("ImageFillViewRectTitle", "\tFill View Rect"));
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

			ImageCategory.AddCustomRow(LOCTEXT("ImageSetNativeSizeRow", ""))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ImageSetNativeSizeTitle", ""))
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
						.Text(LOCTEXT("ImageSetNativeSize", "Set Native Size"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				];
		}		
	}
}

#undef LOCTEXT_NAMESPACE
