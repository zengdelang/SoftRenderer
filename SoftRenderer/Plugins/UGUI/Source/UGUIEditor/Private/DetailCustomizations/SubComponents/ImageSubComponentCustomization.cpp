#include "DetailCustomizations/SubComponents/ImageSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UIBlueprintEditorModule.h"
#include "Core/Widgets/ImageSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FImageSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FImageSubComponentCustomization);
}

void FImageSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UImageSubComponent>(SubComponent.Get());
	
	AddProperty(TEXT("Sprite"), CategoryBuilder)
		->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	AddProperty(TEXT("Color"), CategoryBuilder);
	AddProperty(TEXT("Material"), CategoryBuilder);
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bMaskable"), CategoryBuilder);
	AddProperty(TEXT("bAntiAliasing"), CategoryBuilder)
				->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple)
		{
			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsAntiAliasing())
			{
				AddProperty(TEXT("ImageMaskType"), CategoryBuilder)
					->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

				if (TargetScriptPtr->GetImageMaskMode() == EImageMaskMode::MaskMode_CircleRing)
				{
					AddProperty(TEXT("Thickness"), CategoryBuilder);
				}
			}
		}
		else
		{
			AddProperty(TEXT("ImageMaskType"), CategoryBuilder)
				->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			if (TargetScriptPtr->GetImageMaskMode() == EImageMaskMode::MaskMode_CircleRing)
			{
				AddProperty(TEXT("Thickness"), CategoryBuilder);
			}
		}
	}
	
	AddProperty(TEXT("AlphaHitTestMinimumThreshold"), CategoryBuilder);

	if (TargetScriptPtr.IsValid() && IsValid(TargetScriptPtr->GetSprite()))
	{
		AddProperty(TEXT("ImageType"), CategoryBuilder)
			->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple)
		{
			/// Use Sprite Mesh
			// TODO
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Filled)
		{		
			/// FillMethod
			const auto FillMethodProperty = AddProperty(TEXT("FillMethod"), CategoryBuilder);
			FillMethodProperty->DisplayName(LOCTEXT("ImageSubFillMethodTitle", "\tFill Method"));
			FillMethodProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			/// FillOrigin
			const auto OriginHorizontalProperty = AddProperty(TEXT("OriginHorizontal"), CategoryBuilder);
			OriginHorizontalProperty->Visibility(EVisibility::Collapsed);
			const auto OriginVerticalProperty = AddProperty(TEXT("OriginVertical"), CategoryBuilder);
			OriginVerticalProperty->Visibility(EVisibility::Collapsed);
			const auto Origin90Property = AddProperty(TEXT("Origin90"), CategoryBuilder);
			Origin90Property->Visibility(EVisibility::Collapsed);
			const auto Origin180Property = AddProperty(TEXT("Origin180"), CategoryBuilder);
			Origin180Property->Visibility(EVisibility::Collapsed);
			const auto Origin360Property = AddProperty(TEXT("Origin360"), CategoryBuilder);
			Origin360Property->Visibility(EVisibility::Collapsed);
			CategoryBuilder.AddCustomRow(LOCTEXT("ImageSubFillOriginRow", "FillOrigin"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ImageSubFillOriginTitle", "\tFill Origin"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.ValueContent()
				[
					SNew(SWidgetSwitcher)
					.WidgetIndex(static_cast<int32>(TargetScriptPtr->GetFillMethod()))
					+ SWidgetSwitcher::Slot()
					[
						OriginHorizontalProperty->GetPropertyHandle()->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						OriginVerticalProperty->GetPropertyHandle()->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin90Property->GetPropertyHandle()->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin180Property->GetPropertyHandle()->CreatePropertyValueWidget()
					]
					+ SWidgetSwitcher::Slot()
					[
						Origin360Property->GetPropertyHandle()->CreatePropertyValueWidget()
					]
				];

			/// FillAmount
			AddProperty(TEXT("FillAmount"), CategoryBuilder)->DisplayName(LOCTEXT("ImageSubFillAmountTitle", "\tFill Amount"));

			/// Clockwise
			AddProperty(TEXT("bFillClockwise"), CategoryBuilder)->DisplayName(LOCTEXT("ImageSubClockwiseTitle", "\tClockwise"));
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Tiled ||
			TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Sliced)
		{
			/// FillCenter
			AddProperty(TEXT("bFillCenter"), CategoryBuilder)->DisplayName(LOCTEXT("ImageSubFillCenterTitle", "\tFill Center"));

			/// PixelsPerUnitMultiplier
			AddProperty(TEXT("PixelsPerUnitMultiplier"), CategoryBuilder)->DisplayName(LOCTEXT("ImageSubPixelsPerUnitMultiplierTitle", "\tPixels Per Unit Multiplier"));
			AddProperty(TEXT("OverrideBorder"), CategoryBuilder);
		}

		if (TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Simple ||
			TargetScriptPtr->GetImageType() == EImageFillType::ImageFillType_Filled)
		{
			/// PreserveAspect
			const auto PreserveAspectProperty = AddProperty(TEXT("bPreserveAspect"), CategoryBuilder);
			PreserveAspectProperty->DisplayName(LOCTEXT("ImageSubPreserveAspectTitle", "\tPreserve Aspect"));
			PreserveAspectProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
			
			/// FillViewRect
			if (TargetScriptPtr->GetPreserveAspect())
			{
				AddProperty(TEXT("bFillViewRect"), CategoryBuilder)->DisplayName(LOCTEXT("ImageSubFillViewRectTitle", "\tFill View Rect"));
			}

			/// SetNativeSize
			const auto OnClickedLambda = FOnClicked::CreateLambda([&]
			{
				if (TargetScriptPtr.IsValid())
				{
					const auto RectTransform = Cast<URectTransformComponent>(TargetScriptPtr->GetOuter());
					if (!IsValid(RectTransform))
					{
						return FReply::Handled();
					}

					IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(RectTransform, NSLOCTEXT("RectTransformComponentDetailsSetNativeSize", "ChangeSetNativeSize", "Change SetNativeSize"));
		
					const auto AnchorMin = RectTransform->GetAnchorMin();
					const auto NativeSize = TargetScriptPtr->GetNativeSize();

					const auto AnchorMaxProperty = DetailBuilder.GetProperty(TEXT("AnchorMax"), URectTransformComponent::StaticClass());
					AnchorMaxProperty->SetValue(AnchorMin, EPropertyValueSetFlags::NotTransactable);

					const auto SizeDeltaProperty = DetailBuilder.GetProperty(TEXT("SizeDelta"), URectTransformComponent::StaticClass());
					SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);
					SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);

					IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(RectTransform);
				}
				
				return FReply::Handled();
			});
			
			CategoryBuilder.AddCustomRow(LOCTEXT("ImageSubSetNativeSizeRow", ""))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ImageSubSetNativeSizeTitle", ""))
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
						.Text(LOCTEXT("ImageSubSetNativeSize", "Set Native Size"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				];
		}
	}

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
