#include "DetailCustomizations/ScrollbarComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Core/Widgets/ScrollbarComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FScrollbarComponentDetails::MakeInstance()
{
	return MakeShareable(new FScrollbarComponentDetails);
}

void FScrollbarComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("Selectable"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UScrollbarComponent>(TargetObjects[0].Get());

	IDetailCategoryBuilder& ScrollbarCategory = DetailBuilder.EditCategory(TEXT("Scrollbar"), FText(LOCTEXT("ScrollbarComponentCategory", "Scrollbar")), ECategoryPriority::Important);

	AddRowHeaderContent(ScrollbarCategory, DetailBuilder);

	ScrollbarCategory.AddProperty(TEXT("bInteractable"), USelectableComponent::StaticClass());
	
	ScrollbarCategory.AddProperty(TEXT("Transition"), USelectableComponent::StaticClass());

	const auto TransitionProperty = DetailBuilder.GetProperty(TEXT("Transition"), USelectableComponent::StaticClass());
	TransitionProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_ColorTint)
	{
		const auto ColorSpriteBlockProperty = DetailBuilder.GetProperty(TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());
		
		auto& ColorTintGroup = ScrollbarCategory.AddGroup(TEXT("ColorTint"), LOCTEXT("ColorSpriteBlock_ColorTint", "ColorTint"), false, true);

		const auto NormalColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("NormalColor"));
		ColorTintGroup.AddPropertyRow(NormalColorHandle.ToSharedRef());

		const auto HighlightedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("HighlightedColor"));
		ColorTintGroup.AddPropertyRow(HighlightedColorHandle.ToSharedRef());

		const auto PressedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("PressedColor"));
		ColorTintGroup.AddPropertyRow(PressedColorHandle.ToSharedRef());

		const auto SelectedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("SelectedColor"));
		ColorTintGroup.AddPropertyRow(SelectedColorHandle.ToSharedRef());

		const auto DisabledColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("DisabledColor"));
		ColorTintGroup.AddPropertyRow(DisabledColorHandle.ToSharedRef());
		
		MakeColorMultiplierRow(ColorTintGroup, ColorSpriteBlockProperty);
		
		const auto FadeDurationHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("FadeDuration"));
		ColorTintGroup.AddPropertyRow(FadeDurationHandle.ToSharedRef());
	}
	else if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_SpriteSwap)
	{
		const auto ColorSpriteBlockProperty = DetailBuilder.GetProperty(TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());
		
		auto& SpriteGroup = ScrollbarCategory.AddGroup(TEXT("Sprite"), LOCTEXT("ColorSpriteBlock_Sprite", "Sprite"), false, true);

		const auto NormalSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("NormalSprite"));
		SpriteGroup.AddPropertyRow(NormalSpriteHandle.ToSharedRef());

		const auto HighlightedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("HighlightedSprite"));
		SpriteGroup.AddPropertyRow(HighlightedSpriteHandle.ToSharedRef());

		const auto PressedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("PressedSprite"));
		SpriteGroup.AddPropertyRow(PressedSpriteHandle.ToSharedRef());

		const auto SelectedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("SelectedSprite"));
		SpriteGroup.AddPropertyRow(SelectedSpriteHandle.ToSharedRef());

		const auto DisabledSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("DisabledSprite"));
		SpriteGroup.AddPropertyRow(DisabledSpriteHandle.ToSharedRef());
	}
	else if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_ColorTintAndSpriteSwap)
	{
		const auto ColorSpriteBlockProperty = DetailBuilder.GetProperty(TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());
		
		auto& ColorTintAndSpriteGroup = ScrollbarCategory.AddGroup(TEXT("ColorTintAndSprite"), LOCTEXT("ColorSpriteBlock_ColorTintAndSprite", "ColorTintAndSprite"), false, true);

		const auto NormalColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("NormalColor"));
		ColorTintAndSpriteGroup.AddPropertyRow(NormalColorHandle.ToSharedRef());
		const auto NormalSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("NormalSprite"));
		ColorTintAndSpriteGroup.AddPropertyRow(NormalSpriteHandle.ToSharedRef());

		const auto HighlightedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("HighlightedColor"));
		ColorTintAndSpriteGroup.AddPropertyRow(HighlightedColorHandle.ToSharedRef());
		const auto HighlightedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("HighlightedSprite"));
		ColorTintAndSpriteGroup.AddPropertyRow(HighlightedSpriteHandle.ToSharedRef());
		
		const auto PressedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("PressedColor"));
		ColorTintAndSpriteGroup.AddPropertyRow(PressedColorHandle.ToSharedRef());
		const auto PressedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("PressedSprite"));
		ColorTintAndSpriteGroup.AddPropertyRow(PressedSpriteHandle.ToSharedRef());
		
		const auto SelectedColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("SelectedColor"));
		ColorTintAndSpriteGroup.AddPropertyRow(SelectedColorHandle.ToSharedRef());
		const auto SelectedSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("SelectedSprite"));
		ColorTintAndSpriteGroup.AddPropertyRow(SelectedSpriteHandle.ToSharedRef());
		
		const auto DisabledColorHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("DisabledColor"));
		ColorTintAndSpriteGroup.AddPropertyRow(DisabledColorHandle.ToSharedRef());
		const auto DisabledSpriteHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("DisabledSprite"));
		ColorTintAndSpriteGroup.AddPropertyRow(DisabledSpriteHandle.ToSharedRef());

		MakeColorMultiplierRow(ColorTintAndSpriteGroup, ColorSpriteBlockProperty);

		const auto FadeDurationHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("FadeDuration"));
		ColorTintAndSpriteGroup.AddPropertyRow(FadeDurationHandle.ToSharedRef());
	}
	
	// TODO Navigation

	ScrollbarCategory.AddProperty(TEXT("HandleRectPath"));

	// TODO Unity Direction
	ScrollbarCategory.AddProperty(TEXT("Direction"));
	
	ScrollbarCategory.AddProperty(TEXT("Value")).Visibility(EVisibility::Collapsed);
	ScrollbarCategory.AddProperty(TEXT("Size")).Visibility(EVisibility::Collapsed);
	ScrollbarCategory.AddProperty(TEXT("NumberOfSteps")).Visibility(EVisibility::Collapsed);

	const auto ValuePropertyHandle = DetailBuilder.GetProperty(TEXT("Value"));
	const auto SizePropertyHandle = DetailBuilder.GetProperty(TEXT("Size"));
	const auto NumberOfStepsPropertyHandle = DetailBuilder.GetProperty(TEXT("NumberOfSteps"));
	
	{
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, ValuePropertyHandle] 
		{
			float Value = 0;
			ValuePropertyHandle->GetValue(Value);
			return Value;
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, ValuePropertyHandle](float NewValue)
		{
			ValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, ValuePropertyHandle]()
		{
			float Value = 0;
			ValuePropertyHandle->GetValue(Value);
			ValuePropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});
				
		ScrollbarCategory.AddCustomRow(LOCTEXT("ScrollbarComponent_ValueRow", "Value"))
		.NameContent()
		[
			ValuePropertyHandle->CreatePropertyNameWidget()
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
				SNew(SSlider)
				.Value(SliderValue)
				.OnValueChanged(OnSliderValueChanged)
				.OnMouseCaptureEnd(OnMouseCaptureEnd)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.WidthOverride(50)
				[
					ValuePropertyHandle->CreatePropertyValueWidget()
				]
			]
		];
	}

	{
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, SizePropertyHandle] 
		{
			float Value = 0;
			SizePropertyHandle->GetValue(Value);
			return Value;
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, SizePropertyHandle](float NewValue)
		{
			SizePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, SizePropertyHandle]()
		{
			float Value = 0;
			SizePropertyHandle->GetValue(Value);
			SizePropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});
				
		ScrollbarCategory.AddCustomRow(LOCTEXT("ScrollbarComponent_SizeRow", "Size"))
		.NameContent()
		[
			SizePropertyHandle->CreatePropertyNameWidget()
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
				SNew(SSlider)
				.Value(SliderValue)
				.OnValueChanged(OnSliderValueChanged)
				.OnMouseCaptureEnd(OnMouseCaptureEnd)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.WidthOverride(50)
				[
					SizePropertyHandle->CreatePropertyValueWidget()
				]
			]
		];
	}

	{
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, NumberOfStepsPropertyHandle] 
		{
			int32 Value = 0;
			NumberOfStepsPropertyHandle->GetValue(Value);
			return static_cast<float>(Value);
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, NumberOfStepsPropertyHandle](float NewValue)
		{
			NumberOfStepsPropertyHandle->SetValue(static_cast<int32>(NewValue), EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, NumberOfStepsPropertyHandle]()
		{
			int32 Value = 0;
			NumberOfStepsPropertyHandle->GetValue(Value);
			NumberOfStepsPropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});
				
		ScrollbarCategory.AddCustomRow(LOCTEXT("ScrollbarComponent_NumberOfStepsRow", "NumberOfSteps"))
		.NameContent()
		[
			NumberOfStepsPropertyHandle->CreatePropertyNameWidget()
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
				SNew(SSlider)
				.Value(SliderValue)
				.MinValue(0)
				.MaxValue(11)
				.StepSize(1)
				.OnValueChanged(OnSliderValueChanged)
				.OnMouseCaptureEnd(OnMouseCaptureEnd)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Top)
			.HAlign(HAlign_Right)
			[
				SNew(SBox)
				.WidthOverride(50)
				[
					NumberOfStepsPropertyHandle->CreatePropertyValueWidget()
				]
			]
		];
	}

	AddEventProperty(ScrollbarCategory, DetailBuilder, TEXT("OnValueChanged"));
}

void FScrollbarComponentDetails::MakeColorMultiplierRow(IDetailGroup& ColorTintAndSpriteGroup, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const
{
	const auto ColorMultiplierHandle= ColorSpriteBlockProperty->GetChildHandle(TEXT("ColorMultiplier"));

	const TAttribute<float> SliderValue = TAttribute<float>::Create([&, ColorMultiplierHandle] 
	{
		float Value = 0;
		ColorMultiplierHandle->GetValue(Value);
		return Value;
	});

	const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, ColorMultiplierHandle](float NewValue)
	{
		ColorMultiplierHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
	});

	const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, ColorMultiplierHandle]()
	{
		float Value = 0;
		ColorMultiplierHandle->GetValue(Value);
		ColorMultiplierHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
	});
				
	ColorTintAndSpriteGroup.AddWidgetRow()
	.NameContent()
	[
		ColorMultiplierHandle->CreatePropertyNameWidget()
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
			SNew(SSlider)
			.Value(SliderValue)
			.MinValue(1)
			.MaxValue(5)
			.OnValueChanged(OnSliderValueChanged)
			.OnMouseCaptureEnd(OnMouseCaptureEnd)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Top)
		.HAlign(HAlign_Right)
		[
			SNew(SBox)
			.WidthOverride(50)
			[
				ColorMultiplierHandle->CreatePropertyValueWidget()
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE