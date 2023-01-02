#include "DetailCustomizations/SubComponents/CanvasScalerSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "Widgets/Input/SSlider.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FCanvasScalerSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FCanvasScalerSubComponentCustomization);
}

void FCanvasScalerSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("UIScaleMode"), CategoryBuilder)
		->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TargetScriptPtr = Cast<UCanvasScalerSubComponent>(SubComponent);

	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->GetUIScaleMode() == ECanvasScalerScaleMode::ScaleMode_ConstantPixelSize)
		{
			AddProperty(TEXT("ScaleFactor"), CategoryBuilder);
		}
		else if (TargetScriptPtr->GetUIScaleMode() == ECanvasScalerScaleMode::ScaleMode_ScaleWithScreenSize)
		{
			AddProperty(TEXT("ReferenceResolution"), CategoryBuilder);
			AddProperty(TEXT("ScreenMatchMode"), CategoryBuilder)
				->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
			
			if (TargetScriptPtr->GetScreenMatchMode() == ECanvasScalerScreenMatchMode::ScreenMatchMode_MatchWidthOrHeight)
			{
				const auto MatchWidthOrHeightProperty = AddProperty(TEXT("MatchWidthOrHeight"), CategoryBuilder);
				MatchWidthOrHeightProperty->Visibility(EVisibility::Collapsed);

				auto MatchWidthOrHeightPropertyHandle = MatchWidthOrHeightProperty->GetPropertyHandle();
				
				const TAttribute<float> SliderValue = TAttribute<float>::Create([&, MatchWidthOrHeightPropertyHandle] 
				{
					float Value = 0;
					MatchWidthOrHeightPropertyHandle->GetValue(Value);
					return Value;
				});

				const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, MatchWidthOrHeightPropertyHandle](float NewValue)
				{
					MatchWidthOrHeightPropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
				});

				const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, MatchWidthOrHeightPropertyHandle]()
				{
					float Value = 0;
					MatchWidthOrHeightPropertyHandle->GetValue(Value);
					MatchWidthOrHeightPropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
				});
				
				CategoryBuilder.AddCustomRow(LOCTEXT("CanvasScalerSubComponent_MatchWidthOrHeightRow", "MatchWidthOrHeight"))
				.NameContent()
				[
					MatchWidthOrHeightProperty->GetPropertyHandle()->CreatePropertyNameWidget()
				]
				.ValueContent()
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1)
					.Padding(0, 0, 3, 0)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Fill)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						[
							SNew(SSlider)
							.Value(SliderValue)
							.OnValueChanged(OnSliderValueChanged)
							.OnMouseCaptureEnd(OnMouseCaptureEnd)
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.Padding(0, 0, 0, 0)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Left)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CanvasScalerSubComponent_Width", "Width"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.VAlign(VAlign_Top)
							.HAlign(HAlign_Right)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CanvasScalerSubComponent_Height", "Height"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Top)
					.HAlign(HAlign_Right)
					[
						SNew(SBox)
						.WidthOverride(50)
						[
							MatchWidthOrHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
						]
					]
				];
			}
		}
		else if (TargetScriptPtr->GetUIScaleMode() == ECanvasScalerScaleMode::ScaleMode_ConstantPhysicalSize)
		{
			AddProperty(TEXT("PhysicalUnit"), CategoryBuilder);
			AddProperty(TEXT("FallbackScreenDPI"), CategoryBuilder);
			AddProperty(TEXT("DefaultSpriteDPI"), CategoryBuilder);		
		}

		AddProperty(TEXT("ReferencePixelsPerUnit"), CategoryBuilder);
	}
}

#undef LOCTEXT_NAMESPACE
