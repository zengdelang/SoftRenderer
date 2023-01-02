#include "DetailCustomizations/ButtonComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "Core/Widgets/ButtonComponent.h"
#include "Widgets/Input/SSlider.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FButtonComponentDetails::MakeInstance()
{
	return MakeShareable(new FButtonComponentDetails);
}

void FButtonComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("Selectable"));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UButtonComponent>(TargetObjects[0].Get());

	IDetailCategoryBuilder& ButtonCategory = DetailBuilder.EditCategory(TEXT("Button"), FText(LOCTEXT("ButtonComponentCategory", "Button")), ECategoryPriority::Important);

	AddRowHeaderContent(ButtonCategory, DetailBuilder);

	ButtonCategory.AddProperty(TEXT("bInteractable"), USelectableComponent::StaticClass());
	
	ButtonCategory.AddProperty(TEXT("Transition"), USelectableComponent::StaticClass());

	const auto TransitionProperty = DetailBuilder.GetProperty(TEXT("Transition"), USelectableComponent::StaticClass());
	TransitionProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr->GetTransition() == ESelectableTransition::Transition_ColorTint)
	{
		const auto ColorSpriteBlockProperty = DetailBuilder.GetProperty(TEXT("ColorSpriteBlock"), USelectableComponent::StaticClass());
		
		auto& ColorTintGroup = ButtonCategory.AddGroup(TEXT("ColorTint"), LOCTEXT("ColorSpriteBlock_ColorTint", "ColorTint"), false, true);

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
		
		auto& SpriteGroup = ButtonCategory.AddGroup(TEXT("Sprite"), LOCTEXT("ColorSpriteBlock_Sprite", "Sprite"), false, true);

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
		
		auto& ColorTintAndSpriteGroup = ButtonCategory.AddGroup(TEXT("ColorTintAndSprite"), LOCTEXT("ColorSpriteBlock_ColorTintAndSprite", "ColorTintAndSprite"), false, true);

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
	
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnClicked"));
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnDoubleClicked"));
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnPressed"));
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnReleased"));
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnHovered"));
	AddEventProperty(ButtonCategory, DetailBuilder, TEXT("OnUnhovered"));
}

void FButtonComponentDetails::MakeColorMultiplierRow(IDetailGroup& ColorTintAndSpriteGroup, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const
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
