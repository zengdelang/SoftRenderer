#include "DetailCustomizations/SubComponents/LayoutElementSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/LayoutElementSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FLayoutElementSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FLayoutElementSubComponentCustomization);
}

void FLayoutElementSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<ULayoutElementSubComponent>(SubComponent.Get());
	
	const auto IgnoreLayoutProperty = AddProperty(TEXT("bIgnoreLayout"), CategoryBuilder);
	IgnoreLayoutProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IgnoreLayout())
	{
		CreateFloatPropertyWidget(TEXT("MinWidth"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_MinWidthRow", "MinWidth"));
		CreateFloatPropertyWidget(TEXT("MinHeight"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_MinHeightRow", "MinHeight"));
		CreateFloatPropertyWidget(TEXT("PreferredWidth"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_PreferredWidthRow", "PreferredWidth"));
		CreateFloatPropertyWidget(TEXT("PreferredHeight"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_PreferredHeightRow", "PreferredHeight"));
		CreateFloatPropertyWidget(TEXT("FlexibleWidth"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_FlexibleWidthRow", "FlexibleWidth"));
		CreateFloatPropertyWidget(TEXT("FlexibleHeight"), CategoryBuilder, LOCTEXT("LayoutElementSubComponent_FlexibleHeightRow", "FlexibleHeight"));
	}
	
	AddProperty(TEXT("LayoutPriority"), CategoryBuilder);
}

void FLayoutElementSubComponentCustomization::CreateFloatPropertyWidget(FName PropertyName, IDetailCategoryBuilder& CategoryBuilder, FText FilterString)
{
	const auto Property = AddProperty(PropertyName, CategoryBuilder);
	Property->Visibility(EVisibility::Collapsed);

	auto PropertyHandle = Property->GetPropertyHandle();
			
	const TAttribute<ECheckBoxState> IsChecked = TAttribute<ECheckBoxState>::Create([&, PropertyHandle] 
	{
		TOptional<ECheckBoxState> Result;
		float Value = 0;
		PropertyHandle->GetValue(Value);
		if (Value > -0.5f)
		{
			return ECheckBoxState::Checked;
		}
		return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
	});
			
	const auto HandlePropertyValue = FOnCheckStateChanged::CreateLambda([&, PropertyHandle](ECheckBoxState CheckState)
	{
		if (CheckState == ECheckBoxState::Checked)
		{
			PropertyHandle->SetValue(0.0f);
		}
		else if (CheckState == ECheckBoxState::Unchecked)
		{
			PropertyHandle->SetValue(-1.0f);
		}
	});

	const TAttribute<TOptional<float>> SpinValue = TAttribute<TOptional<float>>::Create([&, PropertyHandle] 
	{
		float NumericVal;
		if (PropertyHandle->GetValue(NumericVal) == FPropertyAccess::Success)
		{
			return TOptional<float>(NumericVal);
		}
		return TOptional<float>();
	});

	const auto OnSpinValueChanged = SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, PropertyHandle](float NewValue)
	{
		PropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::InteractiveChange);
	});

	const auto OnSpinValueCommitted = SNumericEntryBox<float>::FOnValueCommitted::CreateLambda([&, PropertyHandle](float NewValue, ETextCommit::Type CommitType)
	{
		PropertyHandle->SetValue(NewValue, EPropertyValueSetFlags::DefaultFlags);
	});

	const TAttribute<EVisibility> SpinVisibility = TAttribute<EVisibility>::Create([&, PropertyHandle] 
	{
		float Value = 0;
		PropertyHandle->GetValue(Value);
		if (Value > -0.5f)
		{
			return EVisibility::Visible;
		}
		return EVisibility::Collapsed;
	});

	const TOptional<float> MinValue, MaxValue, SliderMinValue, SliderMaxValue;
	constexpr float SliderExponent = 1;
	constexpr float Delta = 0;
	constexpr int32 ShiftMouseMovePixelPerDelta = 1;
	constexpr bool SupportDynamicSliderMaxValue = false;
	constexpr bool SupportDynamicSliderMinValue = false;
			
	CategoryBuilder.AddCustomRow(FilterString)
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		[
			SNew(SCheckBox)
			.IsChecked(IsChecked)
			.OnCheckStateChanged(HandlePropertyValue)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SNew(SNumericEntryBox<float>)
				.Visibility(SpinVisibility)
				.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
				.Value(SpinValue)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
				.OnValueChanged(OnSpinValueChanged)
				.OnValueCommitted(OnSpinValueCommitted)
				.LabelVAlign(VAlign_Center)
				// Only allow spin on handles with one object.  Otherwise it is not clear what value to spin
				.AllowSpin(PropertyHandle->GetNumOuterObjects() < 2)
				.ShiftMouseMovePixelPerDelta(ShiftMouseMovePixelPerDelta)
				.SupportDynamicSliderMaxValue(SupportDynamicSliderMaxValue)
				.SupportDynamicSliderMinValue(SupportDynamicSliderMinValue)
				.MinValue(MinValue)
				.MaxValue(MaxValue)
				.MinSliderValue(SliderMinValue)
				.MaxSliderValue(SliderMaxValue)
				.SliderExponent(SliderExponent)
				.Delta(Delta)
		]
	];
}

#undef LOCTEXT_NAMESPACE
