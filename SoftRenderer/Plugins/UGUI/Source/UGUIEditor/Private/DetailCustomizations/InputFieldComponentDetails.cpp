#include "DetailCustomizations/InputFieldComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Core/Widgets/InputFieldComponent.h"
#include "Widgets/Input/SSlider.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FInputFieldComponentDetails::MakeInstance()
{
	return MakeShareable(new FInputFieldComponentDetails);
}

void FInputFieldComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("InputField"));
	DetailBuilder.HideCategory(TEXT("Selectable"));
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UInputFieldComponent>(TargetObjects[0].Get());

	IDetailCategoryBuilder& InputFieldCategory = DetailBuilder.EditCategory(TEXT("Input Field"), FText(LOCTEXT("InputFieldComponentCategory", "Input Field")), ECategoryPriority::Important);

	AddRowHeaderContent(InputFieldCategory, DetailBuilder);

	InputFieldCategory.AddProperty(TEXT("bInteractable"), USelectableComponent::StaticClass());
	
	InputFieldCategory.AddProperty(TEXT("Transition"), USelectableComponent::StaticClass());

	const auto TransitionProperty = DetailBuilder.GetProperty(TEXT("Transition"), USelectableComponent::StaticClass());
	TransitionProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_ColorTint)
	{
		const auto ColorSpriteBlockProperty= DetailBuilder.GetProperty(
			TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());

		auto& ColorTintGroup = InputFieldCategory.AddGroup(TEXT("ColorTint"), LOCTEXT("ColorSpriteBlock_ColorTint", "ColorTint"), false, true);

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
		
		auto& SpriteGroup = InputFieldCategory.AddGroup(TEXT("Sprite"), LOCTEXT("ColorSpriteBlock_Sprite", "Sprite"), false, true);

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
		
		auto& ColorTintAndSpriteGroup = InputFieldCategory.AddGroup(TEXT("ColorTintAndSprite"), LOCTEXT("ColorSpriteBlock_ColorTintAndSprite", "ColorTintAndSprite"), false, true);

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

	InputFieldCategory.AddProperty(TEXT("TextComponentPath"));
	InputFieldCategory.AddProperty(TEXT("Text"));
	InputFieldCategory.AddProperty(TEXT("CharacterLimit"));
	
	InputFieldCategory.AddProperty(TEXT("ContentType"));
	const auto ContentTypeProperty = DetailBuilder.GetProperty(TEXT("ContentType"));
	ContentTypeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->GetContentType() == EInputFieldContentType::Standard ||
		TargetScriptPtr->GetContentType() == EInputFieldContentType::AutoCorrected)
	{
		InputFieldCategory.AddProperty(TEXT("LineType")).DisplayName(LOCTEXT("InputFieldLineTypeTitle", "\tLine Type"));
	}
	else if (TargetScriptPtr->GetContentType() == EInputFieldContentType::Custom)
	{
		InputFieldCategory.AddProperty(TEXT("LineType")).DisplayName(LOCTEXT("InputFieldLineTypeTitle", "\tLine Type"));
		InputFieldCategory.AddProperty(TEXT("InputType")).DisplayName(LOCTEXT("InputFieldInputTypeTitle", "\tInput Type"));
		InputFieldCategory.AddProperty(TEXT("KeyboardType")).DisplayName(LOCTEXT("InputFieldKeyboardTypeTitle", "\tKeyboard Type"));
		InputFieldCategory.AddProperty(TEXT("CharacterValidation")).DisplayName(LOCTEXT("InputFieldCharacterValidationTitle", "\tCharacter Validation"));
	}
	
	InputFieldCategory.AddProperty(TEXT("PlaceholderPath"));

	{
		const auto CaretBlinkRatePropertyHandle = DetailBuilder.GetProperty(TEXT("CaretBlinkRate"));
		
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, CaretBlinkRatePropertyHandle] 
		{
			float Value = 0;
			CaretBlinkRatePropertyHandle->GetValue(Value);
			return Value;
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, CaretBlinkRatePropertyHandle](float NewValue)
		{
			CaretBlinkRatePropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, CaretBlinkRatePropertyHandle]()
		{
			float Value = 0;
			CaretBlinkRatePropertyHandle->GetValue(Value);
			CaretBlinkRatePropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});
				
		InputFieldCategory.AddCustomRow(LOCTEXT("InputFieldComponent_CaretBlinkRateRow", "CaretBlinkRate"))
		.NameContent()
		[
			CaretBlinkRatePropertyHandle->CreatePropertyNameWidget()
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
				.MaxValue(4)
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
					CaretBlinkRatePropertyHandle->CreatePropertyValueWidget()
				]
			]
		];
	}

	{
		const auto CaretWidthPropertyHandle = DetailBuilder.GetProperty(TEXT("CaretWidth"));
		
		const TAttribute<float> SliderValue = TAttribute<float>::Create([&, CaretWidthPropertyHandle] 
		{
			int32 Value = 0;
			CaretWidthPropertyHandle->GetValue(Value);
			return Value;
		});

		const auto OnSliderValueChanged = FOnFloatValueChanged::CreateLambda([&, CaretWidthPropertyHandle](float NewValue)
		{
			CaretWidthPropertyHandle->SetValue(static_cast<int32>(NewValue), EPropertyValueSetFlags::InteractiveChange);
		});

		const auto OnMouseCaptureEnd = FSimpleDelegate::CreateLambda([&, CaretWidthPropertyHandle]()
		{
			int32 Value = 0;
			CaretWidthPropertyHandle->GetValue(Value);
			CaretWidthPropertyHandle->SetValue(Value, EPropertyValueSetFlags::DefaultFlags);
		});
				
		InputFieldCategory.AddCustomRow(LOCTEXT("InputFieldComponent_CaretWidthRow", "CaretWidth"))
		.NameContent()
		[
			CaretWidthPropertyHandle->CreatePropertyNameWidget()
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
					CaretWidthPropertyHandle->CreatePropertyValueWidget()
				]
			]
		];
	}

	InputFieldCategory.AddProperty(TEXT("bCustomCaretColor"));
	const auto CustomCaretColorProperty = DetailBuilder.GetProperty(TEXT("bCustomCaretColor"));
	CustomCaretColorProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->IsCustomCaretColor())
	{
		InputFieldCategory.AddProperty(TEXT("CaretColor"));
	}
	
	InputFieldCategory.AddProperty(TEXT("SelectionColor"));
	InputFieldCategory.AddProperty(TEXT("bHideMobileInput"));
	InputFieldCategory.AddProperty(TEXT("bReadOnly"));
	
	AddEventProperty(InputFieldCategory, DetailBuilder, TEXT("OnValueChanged"));
	AddEventProperty(InputFieldCategory, DetailBuilder, TEXT("OnEndEdit"));
}

void FInputFieldComponentDetails::MakeColorMultiplierRow(IDetailGroup& ColorTintAndSpriteGroup, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const
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