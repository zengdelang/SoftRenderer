#include "DetailCustomizations/SliderComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Core/Widgets/SelectableComponent.h"
#include "Core/Widgets/SliderComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FSliderComponentDetails::MakeInstance()
{
	return MakeShareable(new FSliderComponentDetails);
}

void FSliderComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("Selectable"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<USliderComponent>(TargetObjects[0].Get());

	IDetailCategoryBuilder& SliderCategory = DetailBuilder.EditCategory(TEXT("Slider"), FText(LOCTEXT("SliderComponentCategory", "Slider")), ECategoryPriority::Important);

	AddRowHeaderContent(SliderCategory, DetailBuilder);

	SliderCategory.AddProperty(TEXT("bInteractable"), USelectableComponent::StaticClass());
	
	SliderCategory.AddProperty(TEXT("Transition"), USelectableComponent::StaticClass());

	const auto TransitionProperty = DetailBuilder.GetProperty(TEXT("Transition"), USelectableComponent::StaticClass());
	TransitionProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_ColorTint)
	{
		const auto ColorSpriteBlockProperty = DetailBuilder.GetProperty(TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());
		
		auto& ColorTintGroup = SliderCategory.AddGroup(TEXT("ColorTint"), LOCTEXT("ColorSpriteBlock_ColorTint", "ColorTint"), false, true);

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
		
		auto& SpriteGroup = SliderCategory.AddGroup(TEXT("Sprite"), LOCTEXT("ColorSpriteBlock_Sprite", "Sprite"), false, true);

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
		
		auto& ColorTintAndSpriteGroup = SliderCategory.AddGroup(TEXT("ColorTintAndSprite"), LOCTEXT("ColorSpriteBlock_ColorTintAndSprite", "ColorTintAndSprite"), false, true);

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

	SliderCategory.AddProperty(TEXT("HandleRectPath"));
	SliderCategory.AddProperty(TEXT("FillRectPath"));

	// TODO Unity Direction
	SliderCategory.AddProperty(TEXT("Direction"));

	const auto WholeNumbersPropertyHandle = DetailBuilder.GetProperty(TEXT("bWholeNumbers"));
	WholeNumbersPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	SliderCategory.AddProperty(TEXT("MinValue")).Visibility(EVisibility::Collapsed);
	SliderCategory.AddProperty(TEXT("MaxValue")).Visibility(EVisibility::Collapsed);
	
	const auto MinValuePropertyHandle = DetailBuilder.GetProperty(TEXT("MinValue"));
	const auto MaxValuePropertyHandle = DetailBuilder.GetProperty(TEXT("MaxValue"));
	
	float SliderHandleMinValue = 0;
	MinValuePropertyHandle->GetValue(SliderHandleMinValue);
		
	float SliderHandleMaxValue = 0;
	MaxValuePropertyHandle->GetValue(SliderHandleMaxValue);

	if (TargetScriptPtr->IsWholeNumbers())
	{
		SliderHandleMinValue = FMath::RoundToFloat(SliderHandleMinValue);
		SliderHandleMaxValue = FMath::RoundToFloat(SliderHandleMaxValue);
	}
	
	{
		const TAttribute<TOptional<float>> SpinValue = TAttribute<TOptional<float>>::Create([&, MinValuePropertyHandle] 
		{
			float NumericVal;
			if (MinValuePropertyHandle->GetValue(NumericVal) == FPropertyAccess::Success)
			{
				return TOptional<float>(TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers() ? FMath::RoundToFloat(NumericVal) : NumericVal);
			}
			return TOptional<float>();
		});

		const auto OnSpinValueChanged = SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, MinValuePropertyHandle, MaxValuePropertyHandle](float NewValue)
		{
			float SliderMaxValue = 0;
			MaxValuePropertyHandle->GetValue(SliderMaxValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				SliderMaxValue = FMath::RoundToFloat(SliderMaxValue);
			}
			
			NewValue = FMath::Min(NewValue, SliderMaxValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				NewValue = FMath::RoundToFloat(NewValue);
			}
			
			MinValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnSpinValueCommitted = SNumericEntryBox<float>::FOnValueCommitted::CreateLambda([&, MinValuePropertyHandle, MaxValuePropertyHandle](float NewValue, ETextCommit::Type CommitType)
		{
			float SliderMaxValue = 0;
			MaxValuePropertyHandle->GetValue(SliderMaxValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				SliderMaxValue = FMath::RoundToFloat(SliderMaxValue);
			}
						
			NewValue = FMath::Min(NewValue, SliderMaxValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				NewValue = FMath::RoundToFloat(NewValue);
			}
			
			MinValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::DefaultFlags);
			DetailBuilder.ForceRefreshDetails();
		});
		
		const TOptional<float> MinValue, MaxValue, SliderMinValue, SliderMaxValue;
		constexpr float SliderExponent = 1;
		constexpr float Delta = 0;
		constexpr int32 ShiftMouseMovePixelPerDelta = 1;
		constexpr bool SupportDynamicSliderMaxValue = false;
		constexpr bool SupportDynamicSliderMinValue = false;
				
		SliderCategory.AddCustomRow(LOCTEXT("SliderComponent_MinValueRow", "MinValue"))
		.NameContent()
		[
			MinValuePropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Left)
		[
			SNew(SNumericEntryBox<float>)
			.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.Value(SpinValue)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.OnValueChanged(OnSpinValueChanged)
			.OnValueCommitted(OnSpinValueCommitted)
			.LabelVAlign(VAlign_Center)
			// Only allow spin on handles with one object.  Otherwise it is not clear what value to spin
			.AllowSpin(MinValuePropertyHandle->GetNumOuterObjects() < 2)
			.ShiftMouseMovePixelPerDelta(ShiftMouseMovePixelPerDelta)
			.SupportDynamicSliderMaxValue(SupportDynamicSliderMaxValue)
			.SupportDynamicSliderMinValue(SupportDynamicSliderMinValue)
			.MinValue(MinValue)
			.MaxValue(MaxValue)
			.MinSliderValue(SliderMinValue)
			.MaxSliderValue(SliderMaxValue)
			.SliderExponent(SliderExponent)
			.Delta(Delta)
		];
	}

	{
		const TAttribute<TOptional<float>> SpinValue = TAttribute<TOptional<float>>::Create([&, MaxValuePropertyHandle] 
		{
			float NumericVal;
			if (MaxValuePropertyHandle->GetValue(NumericVal) == FPropertyAccess::Success)
			{
				return TOptional<float>(TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers() ? FMath::RoundToFloat(NumericVal) : NumericVal);
			}
			return TOptional<float>();
		});

		const auto OnSpinValueChanged = SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, MinValuePropertyHandle, MaxValuePropertyHandle](float NewValue)
		{
			float SliderMinValue = 0;
			MinValuePropertyHandle->GetValue(SliderMinValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				SliderMinValue = FMath::RoundToFloat(SliderMinValue);
			}
			
			NewValue = FMath::Max(NewValue, SliderMinValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				NewValue = FMath::RoundToFloat(NewValue);
			}
			
			MaxValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnSpinValueCommitted = SNumericEntryBox<float>::FOnValueCommitted::CreateLambda([&, MinValuePropertyHandle, MaxValuePropertyHandle](float NewValue, ETextCommit::Type CommitType)
		{
			float SliderMinValue = 0;
			MinValuePropertyHandle->GetValue(SliderMinValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				SliderMinValue = FMath::RoundToFloat(SliderMinValue);
			}
			
			NewValue = FMath::Max(NewValue, SliderMinValue);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				NewValue = FMath::RoundToFloat(NewValue);
			}

			MaxValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::DefaultFlags);
			DetailBuilder.ForceRefreshDetails();
		});
		
		const TOptional<float> MinValue, MaxValue, SliderMinValue, SliderMaxValue;
		constexpr float SliderExponent = 1;
		constexpr float Delta = 0;
		constexpr int32 ShiftMouseMovePixelPerDelta = 1;
		constexpr bool SupportDynamicSliderMaxValue = false;
		constexpr bool SupportDynamicSliderMinValue = false;
				
		SliderCategory.AddCustomRow(LOCTEXT("SliderComponent_MaxValueRow", "MaxValue"))
		.NameContent()
		[
			MaxValuePropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.HAlign(HAlign_Left)
		[
			SNew(SNumericEntryBox<float>)
			.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.Value(SpinValue)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.OnValueChanged(OnSpinValueChanged)
			.OnValueCommitted(OnSpinValueCommitted)
			.LabelVAlign(VAlign_Center)
			// Only allow spin on handles with one object.  Otherwise it is not clear what value to spin
			.AllowSpin(MaxValuePropertyHandle->GetNumOuterObjects() < 2)
			.ShiftMouseMovePixelPerDelta(ShiftMouseMovePixelPerDelta)
			.SupportDynamicSliderMaxValue(SupportDynamicSliderMaxValue)
			.SupportDynamicSliderMinValue(SupportDynamicSliderMinValue)
			.MinValue(MinValue)
			.MaxValue(MaxValue)
			.MinSliderValue(SliderMinValue)
			.MaxSliderValue(SliderMaxValue)
			.SliderExponent(SliderExponent)
			.Delta(Delta)
		];
	}
	
	SliderCategory.AddProperty(TEXT("bWholeNumbers"));
	
	{
		SliderCategory.AddProperty(TEXT("Value")).Visibility(EVisibility::Collapsed);
		
		auto ValuePropertyHandle = DetailBuilder.GetProperty(TEXT("Value"));
 
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, ValuePropertyHandle] 
		{
			float Value = 0;
			ValuePropertyHandle->GetValue(Value);
			return Value;
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, ValuePropertyHandle](float NewValue)
		{
			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				NewValue = FMath::RoundToFloat(NewValue);
			}
			ValuePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, ValuePropertyHandle]()
		{
			float Value = 0;
			ValuePropertyHandle->GetValue(Value);

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsWholeNumbers())
			{
				Value = FMath::RoundToFloat(Value);
			}
			ValuePropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});

		float StepSize = 0.01f;
		if (TargetScriptPtr->IsWholeNumbers())
		{
			StepSize = 1;
		}
		
		SliderCategory.AddCustomRow(LOCTEXT("SliderComponent_ValueRow", "Value"))
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
				.MinValue(SliderHandleMinValue)
				.MaxValue(SliderHandleMaxValue)
				.StepSize(StepSize)
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

	AddEventProperty(SliderCategory, DetailBuilder, TEXT("OnValueChanged"));
}

void FSliderComponentDetails::MakeColorMultiplierRow(IDetailGroup& ColorTintAndSpriteGroup, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const
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