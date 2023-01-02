#include "DetailCustomizations/RectTransformComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"
#include "UIBlueprintEditorModule.h"
#include "UIEditorPerProjectUserSettings.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Styling/SlateIconFinder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Slate/SUIAnchorPreset.h"
#include "Slate/SUIAnchorPresetPanel.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FRectTransformComponentDetails::MakeInstance()
{
	return MakeShareable(new FRectTransformComponentDetails);
}

void FRectTransformComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("TransformCommon"));
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("RectTransformCommon"));

	/// RectTransform Category
	IDetailCategoryBuilder& RectTransformCategory = DetailBuilder.EditCategory(TEXT("RectTransform"), FText(LOCTEXT("TransformCommon", "RectTransform")), ECategoryPriority::Transform);
	AddRowHeaderContent(RectTransformCategory, DetailBuilder);
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<URectTransformComponent>(TargetObjects[0].Get());

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
	{
		FString NodeNameStr = TargetScriptPtr->GetFName().ToString();
		NodeNameStr.ReplaceInline(TEXT("_GEN_VARIABLE"), TEXT(""));

		NodeName = NAME_None;
		
		if (const auto ComponentClass = Cast<UBlueprintGeneratedClass>(TargetScriptPtr->GetOuter()))
		{
			if (const auto SCSNode = ComponentClass->SimpleConstructionScript->FindSCSNode(FName(NodeNameStr)))
			{
				if (const auto ParentSCSNode = ComponentClass->SimpleConstructionScript->FindParentNode(SCSNode))
				{
					NodeName = ParentSCSNode->GetVariableName();
				}
			}
		}
	}
	
	const TAttribute<EVisibility> AnyStretchXVisibility = TAttribute<EVisibility>::Create([&] 
	{
		const URectTransformComponent* Parent = GetParentComponent();
		if (Parent == nullptr)
		{
			return EVisibility::Collapsed;
		}
		
		if (TargetScriptPtr.IsValid())
		{
			if (TargetScriptPtr->GetAnchorMin().X != TargetScriptPtr->GetAnchorMax().X)
			{
				return EVisibility::Visible;
			}
		}
		return EVisibility::Collapsed;
	});
	
	const TAttribute<EVisibility> AnyNonStretchXVisibility = TAttribute<EVisibility>::Create([&] 
	{
		const URectTransformComponent* Parent = GetParentComponent();
		if (Parent == nullptr)
		{
			return EVisibility::Visible;
		}
		
		if (TargetScriptPtr.IsValid())
		{
			if (TargetScriptPtr->GetAnchorMin().X == TargetScriptPtr->GetAnchorMax().X)
			{
				return EVisibility::Visible;
			}
		}
		return EVisibility::Collapsed;
	});

	const TAttribute<EVisibility> AnyStretchYVisibility = TAttribute<EVisibility>::Create([&] 
	{
		const URectTransformComponent* Parent = GetParentComponent();
		if (Parent == nullptr)
		{
			return EVisibility::Collapsed;
		}
		
		if (TargetScriptPtr.IsValid())
		{
			if (TargetScriptPtr->GetAnchorMin().Y != TargetScriptPtr->GetAnchorMax().Y)
			{
				return EVisibility::Visible;
			}
		}
		return EVisibility::Collapsed;
	});
	
	const TAttribute<EVisibility> AnyNonStretchYVisibility = TAttribute<EVisibility>::Create([&] 
	{
		const URectTransformComponent* Parent = GetParentComponent();
		if (Parent == nullptr)
		{
			return EVisibility::Visible;
		}
		
		if (TargetScriptPtr.IsValid())
		{
			if (TargetScriptPtr->GetAnchorMin().Y == TargetScriptPtr->GetAnchorMax().Y)
			{
				return EVisibility::Visible;
			}
		}
		return EVisibility::Collapsed;
	});
	
	const TAttribute<FText> WidthText = TAttribute<FText>::Create([&]
	{
		if (TargetScriptPtr.IsValid())
		{
			const URectTransformComponent* Parent = GetParentComponent();
			if (Parent == nullptr)
			{
				if (TargetScriptPtr->GetAnchorMin().X != TargetScriptPtr->GetAnchorMax().X)
				{
					return FText(LOCTEXT("RectTransformComponent_AnchorsWDelta", "W Delta"));
				}
			}
		}
		
		return FText(LOCTEXT("RectTransformComponent_AnchorsWidth", "Width"));
	});

	const TAttribute<FText> HeightText = TAttribute<FText>::Create([&]
	{
		if (TargetScriptPtr.IsValid())
		{
			const URectTransformComponent* Parent = GetParentComponent();
			if (Parent == nullptr)
			{
				if (TargetScriptPtr->GetAnchorMin().Y != TargetScriptPtr->GetAnchorMax().Y)
				{
					return FText(LOCTEXT("RectTransformComponent_AnchorsHDelta", "H Delta"));
				}
			}
		}
		
		return FText(LOCTEXT("RectTransformComponent_AnchorsHeight", "Height"));
	});
	
	auto AnchoredPositionHandle = DetailBuilder.GetProperty(TEXT("AnchoredPosition"));
	auto AnchoredPositionXHandle = AnchoredPositionHandle->GetChildHandle(0);
	auto AnchoredPositionYHandle = AnchoredPositionHandle->GetChildHandle(1);

	auto OffsetMinHandle = DetailBuilder.GetProperty(TEXT("OffsetMin"));
	auto OffsetMinXHandle = OffsetMinHandle->GetChildHandle(0);
	auto OffsetMinYHandle = OffsetMinHandle->GetChildHandle(1);
 
	auto OffsetMaxHandle = DetailBuilder.GetProperty(TEXT("OffsetMax"));
	auto OffsetMaxXHandle = OffsetMaxHandle->GetChildHandle(0);
	auto OffsetMaxYHandle = OffsetMaxHandle->GetChildHandle(1);

	auto SizeDeltaHandle = DetailBuilder.GetProperty(TEXT("SizeDelta"));
	auto SizeDeltaXHandle = SizeDeltaHandle->GetChildHandle(0);
	auto SizeDeltaYHandle = SizeDeltaHandle->GetChildHandle(1);

	auto AnchorMinHandle = DetailBuilder.GetProperty(TEXT("AnchorMin"));
	auto AnchorMinXHandle = AnchorMinHandle->GetChildHandle(0);
	auto AnchorMinYHandle = AnchorMinHandle->GetChildHandle(1);
	
	auto AnchorMaxHandle = DetailBuilder.GetProperty(TEXT("AnchorMax"));
	auto AnchorMaxXHandle = AnchorMaxHandle->GetChildHandle(0);
	auto AnchorMaxYHandle = AnchorMaxHandle->GetChildHandle(1);

	auto PivotHandle = DetailBuilder.GetProperty(TEXT("Pivot"));
	auto PivotXHandle = PivotHandle->GetChildHandle(0);
	auto PivotYHandle = PivotHandle->GetChildHandle(1);

	TAttribute<FVector2D> GetAnchorMin = TAttribute<FVector2D>::Create([AnchorMinHandle]
								{
									FVector2D NumericVal;
									AnchorMinHandle->GetValue(NumericVal);
									return NumericVal;
								});

	TAttribute<FVector2D> GetAnchorMax = TAttribute<FVector2D>::Create([AnchorMaxHandle]
								{
									FVector2D NumericVal;
									AnchorMaxHandle->GetValue(NumericVal);
									return NumericVal;
								});

	TAttribute<FVector2D> GetPivot = TAttribute<FVector2D>::Create([PivotHandle]
	                            {
		                            FVector2D NumericVal;
									PivotHandle->GetValue(NumericVal);
									return NumericVal;
	                            });

	TAttribute<FText> GetHorizontalText = TAttribute<FText>::Create([AnchorMaxHandle,AnchorMinHandle]
	{
	    FVector2D AnchorMinValue;
	    FVector2D AnchorMaxValue;

		AnchorMaxHandle->GetValue(AnchorMaxValue);
		AnchorMinHandle->GetValue(AnchorMinValue);

		if (AnchorMinValue.X == AnchorMaxValue.X)
		{
			if (AnchorMinValue.X == 0)
			{
				return FText(LOCTEXT("RectTransformComponent_HorizontalTextLeft", "Left"));
			}

			if (AnchorMinValue.X == 0.5)
			{
				return FText(LOCTEXT("RectTransformComponent_HorizontalTextCenter", "Center"));
			}

			if (AnchorMinValue.X == 1)
			{
				return FText(LOCTEXT("RectTransformComponent_HorizontalTextRight", "Right"));
			}
		}
		else
		{
			if (AnchorMinValue.X == 0 && AnchorMaxValue.X == 1)
			{
				return FText(LOCTEXT("RectTransformComponent_HorizontalTextStretch", "Stretch"));
			}
		}

		return FText(LOCTEXT("RectTransformComponent_HorizontalTextCustom", "Custom"));
	});

	TAttribute<FText> GetVerticalText = TAttribute<FText>::Create([AnchorMaxHandle,AnchorMinHandle]
	{
		FVector2D AnchorMinValue;
		FVector2D AnchorMaxValue;

		AnchorMaxHandle->GetValue(AnchorMaxValue);
		AnchorMinHandle->GetValue(AnchorMinValue);

		if (AnchorMinValue.Y == AnchorMaxValue.Y)
		{
			if (AnchorMinValue.Y == 0)
			{
				return FText(LOCTEXT("RectTransformComponent_VerticalTextBottom", "Bottom"));
			}

			if (AnchorMinValue.Y == 0.5)
			{
				return FText(LOCTEXT("RectTransformComponent_VerticalTextMiddle", "Middle"));
			}

			if (AnchorMinValue.Y == 1)
			{
				return FText(LOCTEXT("RectTransformComponent_VerticalTextTop", "Top"));
			}
		}
		else
		{
			if (AnchorMinValue.Y == 0 && AnchorMaxValue.Y == 1)
			{
				return FText(LOCTEXT("RectTransformComponent_VerticalTextStretch", "Stretch"));
			}
		}

		return FText(LOCTEXT("RectTransformComponent_VerticalTextCustom", "Custom"));
	});

	RectTransformCategory.AddCustomRow(LOCTEXT("RectTransformComponent_LayoutRow", "PosX PosY PosZ Width Height Left Right Top Bottom"))
	.NameContent()
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Fill)
	[
		SNew(SBox)
		.WidthOverride(60)
		.HeightOverride(60)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		.Padding(FMargin(0, 5, 0, 0))
		[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(16, 21 + 42, 0, 0))
			.AutoSize(true)
			[
				SNew(SBox)
				.WidthOverride(42)
				.HeightOverride(18)
				[
					SNew(STextBlock)
					.RenderTransform(FSlateRenderTransform(FQuat2D(-PI / 2)))
					.Justification(ETextJustify::Left)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(GetVerticalText)
				]
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(9 + 21.5, 6, 0, 0))
			.AutoSize(true)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(GetHorizontalText)
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(9, 14, 0, 0))
			.Alignment(FVector2D(0, 0))
			.AutoSize(true)
			[
				SNew(SBox)
				.WidthOverride(42)
				.HeightOverride(42)
				[
					SNew(SComboButton)
					.HasDownArrow(false)
					.ComboButtonStyle(&FComboButtonStyle::GetDefault())
					.ContentPadding(0)
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					.MenuContent()
					[
						SNew(SUIAnchorPresetPanel, AnchorMaxHandle, AnchorMinHandle, PivotHandle, AnchoredPositionHandle, TargetScriptPtr)
						.AnchorMax(GetAnchorMax)
						.AnchorMin(GetAnchorMin)
						.Pivot(GetPivot)
						.SizeDeltaHandle(SizeDeltaHandle)
					]
					.ButtonContent()
					[
						SNew(SBorder)
						.Padding(0)
						.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryMiddle"))
						.BorderBackgroundColor(FLinearColor(0.8, 0.8, 0.8, 1))
						[
							SNew(SUIAnchorPreset)
							.AnchorMax(GetAnchorMax)
							.AnchorMin(GetAnchorMin)
							.bShowSelectedPosBound(false)
						]
					]
			    ]
			]
		]
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4, 0, 0, 0)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyStretchXVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsLeft", "Left"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
							TAttribute<TOptional<float>>::Create([OffsetMinXHandle]
								{
									float NumericVal;
									if (OffsetMinXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
									{
										return TOptional<float>(NumericVal);
									}
									return TOptional<float>();
								}),
								SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, OffsetMinHandle](float NewValue)
								{
									float OffsetMinY = 0;
									if (TargetScriptPtr.IsValid())
									{
										OffsetMinY = TargetScriptPtr->GetOffsetMin().Y;
									}
									OffsetMinHandle->SetValue(FVector2D(NewValue, OffsetMinY), EPropertyValueSetFlags::InteractiveChange);
								}),
								SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
								[&, OffsetMinHandle](float NewValue, ETextCommit::Type CommitType)
								{
									float OffsetMinY = 0;
									if (TargetScriptPtr.IsValid())
									{
										OffsetMinY = TargetScriptPtr->GetOffsetMin().Y;
									}
									OffsetMinHandle->SetValue(FVector2D(NewValue, OffsetMinY), EPropertyValueSetFlags::DefaultFlags);
								}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyNonStretchXVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsPosX", "Pos X"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
								TAttribute<TOptional<float>>::Create([AnchoredPositionXHandle]
									{
										float NumericVal;
										if (AnchoredPositionXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
										{
											return TOptional<float>(NumericVal);
										}
										return TOptional<float>();
									}),
									SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchoredPositionHandle](float NewValue)
									{
										float AnchoredPositionY = 0;
										if (TargetScriptPtr.IsValid())
										{
											AnchoredPositionY = TargetScriptPtr->GetAnchoredPosition().Y;
										}
										AnchoredPositionHandle->SetValue(FVector2D(NewValue, AnchoredPositionY), EPropertyValueSetFlags::InteractiveChange);
									}),
									SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
									[&, AnchoredPositionHandle](float NewValue, ETextCommit::Type CommitType)
									{
										float AnchoredPositionY = 0;
										if (TargetScriptPtr.IsValid())
										{
											AnchoredPositionY = TargetScriptPtr->GetAnchoredPosition().Y;
										}
										AnchoredPositionHandle->SetValue(FVector2D(NewValue, AnchoredPositionY), EPropertyValueSetFlags::DefaultFlags);
									}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyStretchYVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsTop", "Top"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
						TAttribute<TOptional<float>>::Create([OffsetMaxYHandle]
							{
								float NumericVal;
								if (OffsetMaxYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
								{
									if (NumericVal == 0)
									{
										return TOptional<float>(0);
									}
									return TOptional<float>(-NumericVal);
								}
								return TOptional<float>();
							}),
							SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, OffsetMaxHandle](float NewValue)
							{
								float OffsetMaxX = 0;
								if (TargetScriptPtr.IsValid())
								{
									OffsetMaxX = TargetScriptPtr->GetOffsetMax().X;
								}
								OffsetMaxHandle->SetValue(FVector2D(OffsetMaxX, -NewValue), EPropertyValueSetFlags::InteractiveChange);
							}),
							SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
							[&, OffsetMaxHandle](float NewValue, ETextCommit::Type CommitType)
							{
								float OffsetMaxX = 0;
								if (TargetScriptPtr.IsValid())
								{
									OffsetMaxX = TargetScriptPtr->GetOffsetMax().X;
								}
								OffsetMaxHandle->SetValue(FVector2D(OffsetMaxX, -NewValue), EPropertyValueSetFlags::DefaultFlags);
							}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyNonStretchYVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsPosY", "Pos Y"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
							TAttribute<TOptional<float>>::Create([AnchoredPositionYHandle]
								{
									float NumericVal;
									if (AnchoredPositionYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
									{
										return TOptional<float>(NumericVal);
									}
									return TOptional<float>();
								}),
								SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchoredPositionHandle](float NewValue)
								{
									float AnchoredPositionX = 0;
									if (TargetScriptPtr.IsValid())
									{
										AnchoredPositionX = TargetScriptPtr->GetAnchoredPosition().X;
									}
									AnchoredPositionHandle->SetValue(FVector2D(AnchoredPositionX, NewValue), EPropertyValueSetFlags::InteractiveChange);
								}),
								SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
								[&, AnchoredPositionHandle](float NewValue, ETextCommit::Type CommitType)
								{
									float AnchoredPositionX = 0;
									if (TargetScriptPtr.IsValid())
									{
										AnchoredPositionX = TargetScriptPtr->GetAnchoredPosition().X;
									}
									AnchoredPositionHandle->SetValue(FVector2D(AnchoredPositionX, NewValue), EPropertyValueSetFlags::DefaultFlags);
								}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsPosZ", "Pos Z"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					DetailBuilder.GetProperty(TEXT("LocalPositionZ"))->CreatePropertyValueWidget()
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(4, 6, 0, 6)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyStretchXVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsRight", "Right"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
						TAttribute<TOptional<float>>::Create([OffsetMaxXHandle]
							{
								float NumericVal;
								if (OffsetMaxXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
								{
									if (NumericVal == 0)
									{
										TOptional<float>(0.0f);
									}
									return TOptional<float>(-NumericVal);
								}
								return TOptional<float>();
							}),
							SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, OffsetMaxHandle](float NewValue)
							{
								float OffsetMaxY = 0;
								if (TargetScriptPtr.IsValid())
								{
									OffsetMaxY = TargetScriptPtr->GetOffsetMax().Y;
								}
								OffsetMaxHandle->SetValue(FVector2D(-NewValue, OffsetMaxY), EPropertyValueSetFlags::InteractiveChange);
							}),
							SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
							[&, OffsetMaxHandle](float NewValue, ETextCommit::Type CommitType)
							{
								float OffsetMaxY = 0;
								if (TargetScriptPtr.IsValid())
								{
									OffsetMaxY = TargetScriptPtr->GetOffsetMax().Y;
								}
								OffsetMaxHandle->SetValue(FVector2D(-NewValue, OffsetMaxY), EPropertyValueSetFlags::DefaultFlags);
							}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyNonStretchXVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(WidthText)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
					TAttribute<TOptional<float>>::Create([SizeDeltaXHandle]
						{
							float NumericVal;
							if (SizeDeltaXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
							{
								return TOptional<float>(NumericVal);
							}
							return TOptional<float>();
						}),
						SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, SizeDeltaHandle](float NewValue)
						{
							float SizeDeltaY = 0;
							if (TargetScriptPtr.IsValid())
							{
								SizeDeltaY = TargetScriptPtr->GetSizeDelta().Y;
							}
							SizeDeltaHandle->SetValue(FVector2D(NewValue, SizeDeltaY), EPropertyValueSetFlags::InteractiveChange);
						}),
						SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
						[&, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
						{
							float SizeDeltaY = 0;
							if (TargetScriptPtr.IsValid())
							{
								SizeDeltaY = TargetScriptPtr->GetSizeDelta().Y;
							}
							SizeDeltaHandle->SetValue(FVector2D(NewValue, SizeDeltaY), EPropertyValueSetFlags::DefaultFlags);
						}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyStretchYVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RectTransformComponent_AnchorsBottom", "Bottom"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
					TAttribute<TOptional<float>>::Create([OffsetMinYHandle]
						{
							float NumericVal;
							if (OffsetMinYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
							{
								return TOptional<float>(NumericVal);
							}
							return TOptional<float>();
						}),
						SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, OffsetMinHandle](float NewValue)
						{
							float OffsetMinX = 0;
							if (TargetScriptPtr.IsValid())
							{
								OffsetMinX = TargetScriptPtr->GetOffsetMin().X;
							}
							OffsetMinHandle->SetValue(FVector2D(OffsetMinX, NewValue), EPropertyValueSetFlags::InteractiveChange);
						}),
						SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
						[&, OffsetMinHandle](float NewValue, ETextCommit::Type CommitType)
						{
							float OffsetMinX = 0;
							if (TargetScriptPtr.IsValid())
							{
								OffsetMinX = TargetScriptPtr->GetOffsetMin().X;
							}
							OffsetMinHandle->SetValue(FVector2D(OffsetMinX, NewValue), EPropertyValueSetFlags::DefaultFlags);
						}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SVerticalBox)
				.Visibility(AnyNonStretchYVisibility)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(HeightText)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Fill)
				.Padding(0, 2, 0, 0)
				[
					MakeNumericWidget(AnchoredPositionHandle,
						TAttribute<TOptional<float>>::Create([SizeDeltaYHandle]
						{
							float NumericVal;
							if (SizeDeltaYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
							{
								return TOptional<float>(NumericVal);
							}
							return TOptional<float>();
						}),
						SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, SizeDeltaHandle](float NewValue)
						{
							float SizeDeltaX = 0;
							if (TargetScriptPtr.IsValid())
							{
								SizeDeltaX = TargetScriptPtr->GetSizeDelta().X;
							}
							SizeDeltaHandle->SetValue(FVector2D(SizeDeltaX, NewValue), EPropertyValueSetFlags::InteractiveChange);
						}),
						SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
						[&, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
						{
							float SizeDeltaX = 0;
							if (TargetScriptPtr.IsValid())
							{
								SizeDeltaX = TargetScriptPtr->GetSizeDelta().X;
							}
							SizeDeltaHandle->SetValue(FVector2D(SizeDeltaX, NewValue), EPropertyValueSetFlags::DefaultFlags);
						}))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Padding(0, 0, 4, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Fill)
			[
				SNew(SSpacer)
			]
		]
	];

	auto& AnchorsGroup = RectTransformCategory.AddGroup(TEXT("Anchors"), LOCTEXT("RectTransformComponent_Anchors", "Anchors"), false, true);
	
	AnchorsGroup.AddWidgetRow()
	.NameContent()
	[
		DetailBuilder.GetProperty(TEXT("AnchorMin"))->CreatePropertyNameWidget(LOCTEXT("RectTransformComponent_AnchorMinRow", "Min"))
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 4, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
						TAttribute<TOptional<float>>::Create([AnchorMinXHandle]
						{
							float NumericVal;
							if (AnchorMinXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
							{
								return TOptional<float>(NumericVal);
							}
							return TOptional<float>();
						}),
						SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue)
						{
							SetAnchorSmart(NewValue, 0, false, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle);
						}),
						SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
						[&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
						{
							SetAnchorSmart(NewValue, 0, false, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle, true);
						}), true, LOCTEXT("RectTransformComponent_AnchorMinX", "X"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
					TAttribute<TOptional<float>>::Create([AnchorMinYHandle]
					{
						float NumericVal;
						if (AnchorMinYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
						{
							return TOptional<float>(NumericVal);
						}
						return TOptional<float>();
					}),
					SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue)
					{
						SetAnchorSmart(NewValue,1, false, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle);
					}),
					SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
			[&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
					{
						SetAnchorSmart(NewValue,1, false, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle, true);
					}), true, LOCTEXT("RectTransformComponent_AnchorMinY", "Y"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SNew(SSpacer)
		]
	];

	///////////////////////
	
	AnchorsGroup.AddWidgetRow()
	.NameContent()
	[
		DetailBuilder.GetProperty(TEXT("AnchorMax"))->CreatePropertyNameWidget(LOCTEXT("RectTransformComponent_AnchorMaxRow", "Max"))
	]
	.ValueContent()
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 4, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([AnchorMaxXHandle]
				{
					float NumericVal;
					if (AnchorMaxXHandle->GetValue(NumericVal) == FPropertyAccess::Success)
					{
						return TOptional<float>(NumericVal);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue)
				{
					SetAnchorSmart(NewValue, 0, true, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
				{
					SetAnchorSmart(NewValue, 0, true, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle, true);
				}), true, LOCTEXT("RectTransformComponent_AnchorMaxX", "X"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([AnchorMaxYHandle]
				{
					float NumericVal;
					if (AnchorMaxYHandle->GetValue(NumericVal) == FPropertyAccess::Success)
					{
						return TOptional<float>(NumericVal);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue)
				{
					SetAnchorSmart(NewValue,1, true, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle](float NewValue, ETextCommit::Type CommitType)
				{
					SetAnchorSmart(NewValue,1, true, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, AnchorMinHandle, AnchorMaxHandle, AnchoredPositionHandle, SizeDeltaHandle, true);
				}), true, LOCTEXT("RectTransformComponent_AnchorMaxY", "Y"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SNew(SSpacer)
		]
	];

	///////////////////////////////
	
	RectTransformCategory.AddCustomRow(LOCTEXT("RectTransformComponent_PivotRow", "Pivot"))
	.NameContent()
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		[
			DetailBuilder.GetProperty(TEXT("Pivot"))->CreatePropertyNameWidget()
		]
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 4, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetPivot().X);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, PivotHandle, AnchoredPositionHandle](float NewValue)
				{
					SetPivotSmart(NewValue, 0, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, PivotHandle, AnchoredPositionHandle);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&, PivotHandle, AnchoredPositionHandle](float NewValue, ETextCommit::Type CommitType)
				{
					SetPivotSmart(NewValue, 0, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, PivotHandle, AnchoredPositionHandle, true);
				}), true, LOCTEXT("RectTransformComponent_PivotX", "X"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetPivot().Y);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&, PivotHandle, AnchoredPositionHandle](float NewValue)
				{
					SetPivotSmart(NewValue, 1, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, PivotHandle, AnchoredPositionHandle);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&, PivotHandle, AnchoredPositionHandle](float NewValue, ETextCommit::Type CommitType)
				{
					SetPivotSmart(NewValue, 1, !GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode, PivotHandle, AnchoredPositionHandle, true);
				}), true, LOCTEXT("RectTransformComponent_PivotY", "Y"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 0, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			SNew(SSpacer)
		]
	];

	///////////////////////////////

	RectTransformCategory.AddCustomRow(LOCTEXT("RectTransformComponent_Rotation", "Rotation"))
	.NameContent()
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RectTransformLocalRotation", "Rotation"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 4, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalRotation().Roll);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalRotation(NewValue, 0);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalRotation(NewValue, 0, true);
				}), true, LOCTEXT("RectTransformComponent_LocalRotationX", "X"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalRotation().Pitch);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalRotation(NewValue, 1);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalRotation(NewValue, 1, true);
				}), true, LOCTEXT("RectTransformComponent_LocalRotationY", "Y"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalRotation().Yaw);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalRotation(NewValue, 2);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalRotation(NewValue, 2, true);
				}), true, LOCTEXT("RectTransformComponent_LocalRotationZ", "Z"), 0)
		]
	];
	
	///////////////////////////////

	RectTransformCategory.AddCustomRow(LOCTEXT("RectTransformComponent_Scale", "Scale"))
	.NameContent()
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RectTransformLocalScale", "Scale"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
	]
	.ValueContent()
	.VAlign(VAlign_Center)
	.HAlign(HAlign_Fill)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 4, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalScale().X);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalScale(NewValue, 0);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalScale(NewValue, 0, true);
				}), true, LOCTEXT("RectTransformComponent_LocalScaleX", "X"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalScale().Y);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalScale(NewValue, 1);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalScale(NewValue, 1, true);
				}), true, LOCTEXT("RectTransformComponent_LocalScaleY", "Y"), 0)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.Padding(0, 0, 6, 0)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Fill)
		[
			MakeNumericWidget(AnchoredPositionHandle,
				TAttribute<TOptional<float>>::Create([&]
				{
					if (TargetScriptPtr.IsValid())
					{
						return TOptional<float>(TargetScriptPtr->GetLocalScale().Z);
					}
					return TOptional<float>();
				}),
				SNumericEntryBox<float>::FOnValueChanged::CreateLambda([&](float NewValue)
				{
					SetLocalScale(NewValue, 2);
				}),
				SNumericEntryBox<float>::FOnValueCommitted::CreateLambda(
		[&](float NewValue, ETextCommit::Type CommitType)
				{
					SetLocalScale(NewValue, 2, true);
				}), true, LOCTEXT("RectTransformComponent_LocalScaleZ", "Z"), 0)
		]
	];
}

FSlateBrush* FRectTransformComponentDetails::GetIconBrush()
{
	return const_cast<FSlateBrush*>(FSlateIconFinder::FindIconBrushForClass(URectTransformComponent::StaticClass(), TEXT("SCS.Component")));
}

void FRectTransformComponentDetails::SetAnchorSmart(float Value, int32 Axis, bool bIsMax, bool bSmart, TSharedRef<IPropertyHandle> AnchorMinHandle,
		TSharedRef<IPropertyHandle> AnchorMaxHandle, TSharedRef<IPropertyHandle> AnchoredPositionHandle, TSharedRef<IPropertyHandle> SizeDeltaHandle, bool bCommitted)
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
	}

	if (!bRecordSetAnchorSmart)
	{
		bRecordSetAnchorSmart = true;
		IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("RectTransformComponentDetailsSetAnchorSmart", "ChangeSetAnchorSmart", "Change SetAnchorSmart"));
	}
	
	const URectTransformComponent* Parent = GetParentComponent();
	
	if (Parent == nullptr)
	{
		bSmart = false;
	}
	
	float OffsetSizePixels = 0;
	float OffsetPositionPixels = 0;
	if (bSmart)
	{
		const float OldValue = bIsMax ? TargetScriptPtr->GetAnchorMax()[Axis] : TargetScriptPtr->GetAnchorMin()[Axis];
		OffsetSizePixels = (Value - OldValue) * Parent->GetRect().GetSize()[Axis];
		OffsetPositionPixels = (bIsMax ? OffsetSizePixels * TargetScriptPtr->GetPivot()[Axis] : (OffsetSizePixels * (1 - TargetScriptPtr->GetPivot()[Axis])));
	}
		
	if (bIsMax)
	{
		FVector2D RectAnchorMax = TargetScriptPtr->GetAnchorMax();
		RectAnchorMax[Axis] = Value;
		//AnchorMaxHandle->SetValue(RectAnchorMax, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchorMax(RectAnchorMax);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchorMax(RectAnchorMax);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	
		const FVector2D Other = TargetScriptPtr->GetAnchorMin();
		//AnchorMinHandle->SetValue(Other, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchorMin(Other);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchorMin(Other);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	}
	else
	{
		FVector2D RectAnchorMin = TargetScriptPtr->GetAnchorMin();
		RectAnchorMin[Axis] = Value;
		//AnchorMinHandle->SetValue(RectAnchorMin, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchorMin(RectAnchorMin);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchorMin(RectAnchorMin);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
		
		const FVector2D Other = TargetScriptPtr->GetAnchorMax();
		//AnchorMaxHandle->SetValue(Other, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchorMax(Other);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchorMax(Other);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	}

	if (bSmart)
	{
		FVector2D RectPosition = TargetScriptPtr->GetAnchoredPosition();
		RectPosition[Axis] -= OffsetPositionPixels;
		//AnchoredPositionHandle->SetValue(RectPosition, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchoredPosition(RectPosition);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchoredPosition(RectPosition);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
		
		FVector2D RectSizeDelta = TargetScriptPtr->GetSizeDelta();
		RectSizeDelta[Axis] += OffsetSizePixels * (bIsMax ? -1 : 1);
		//SizeDeltaHandle->SetValue(RectSizeDelta, bCommitted ? EPropertyValueSetFlags::DefaultFlags : EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetSizeDelta(RectSizeDelta);
		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetSizeDelta(RectSizeDelta);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	}

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->Modify();
		TargetScriptPtr->MarkPackageDirty();
	}
	
	if (bCommitted)
    {
		bRecordSetPivotSmart = false;
    	IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
   	}
}

URectTransformComponent* FRectTransformComponentDetails::GetParentComponent() const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->IsTemplate())
		{
			if (const auto ComponentClass = Cast<UBlueprintGeneratedClass>(TargetScriptPtr->GetOuter()))
			{
				if (const auto SCSNode = ComponentClass->SimpleConstructionScript->FindSCSNode(NodeName))
				{
					return Cast<URectTransformComponent>(SCSNode->ComponentTemplate);
				}
			}
			
			/*TArray<UObject*> ArchetypeInstances;
			TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);

			for (const auto& Instance : ArchetypeInstances)
			{
				if (IsValid(Instance) && Instance->GetWorld() && Instance->GetWorld()->WorldType == EWorldType::EditorPreview)
				{
					const URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Instance);
					if (IsValid(RectTransform))
					{
						return Cast<URectTransformComponent>(RectTransform->GetAttachParent());
					}
				}
			}*/
		}
		else
		{
			return Cast<URectTransformComponent>(TargetScriptPtr->GetAttachParent());
		}
	}
	return nullptr;
}

FVector FRectTransformComponentDetails::GetRectReferenceCorner(const URectTransformComponent* RectTransform, bool bWorldSpace)
{
	if (bWorldSpace)
	{
		FVector WorldCorners[4];
		RectTransform->GetWorldCorners(WorldCorners);
		
		if (RectTransform->GetAttachParent())
		{
			return RectTransform->GetAttachParent()->GetComponentTransform().Inverse().TransformPosition(WorldCorners[0]);
		}
		else
		{
			return WorldCorners[0];
		}
	}
	return FVector(RectTransform->GetRect().XMin, RectTransform->GetRect().YMin, 0) + RectTransform->GetLocalLocation();
}

void FRectTransformComponentDetails::SetPivotSmart(float Value, int32 Axis, bool bSmart,
	TSharedRef<IPropertyHandle> PivotHandle, TSharedRef<IPropertyHandle> AnchoredPositionHandle, bool bCommitted)
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
	}

	if (!bRecordSetPivotSmart)
	{
		bRecordSetPivotSmart = true;
		IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("RectTransformComponentDetailsSetPivotSmart", "ChangeSetPivotSmart", "Change SetPivotSmart"));
	}

	URectTransformComponent* RectTransform = TargetScriptPtr.Get();
	if (TargetScriptPtr->IsTemplate())
	{
		RectTransform = nullptr;
		
		TArray<UObject*> ArchetypeInstances;
		TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);

		for (const auto& Instance : ArchetypeInstances)
		{
			if (IsValid(Instance) && Instance->GetWorld() && Instance->GetWorld()->WorldType == EWorldType::EditorPreview)
			{
				RectTransform = Cast<URectTransformComponent>(Instance);
				break;
			}
		}
	}

	if (!IsValid(RectTransform))
	{
		return;
	}

	const FVector CornerBefore = GetRectReferenceCorner(RectTransform, true);

	FVector2D RectPivot = TargetScriptPtr->GetPivot();
	RectPivot[Axis] = Value;

	TargetScriptPtr->SetPivot(RectPivot);
	//PivotHandle->SetValue(RectPivot, EPropertyValueSetFlags::InteractiveChange);

	TArray<UObject*> ArchetypeInstances;
	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
		for (const auto& ArchetypeInstance : ArchetypeInstances)
		{
			const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
			if (ComponentInstance)
			{
				ComponentInstance->Modify();
				ComponentInstance->SetPivot(RectPivot);
				ComponentInstance->MarkPackageDirty();
			}
		}
	}
	
	if (bSmart)
	{
		if (!IsValid(RectTransform))
		{
			return;
		}
		
		const FVector CornerAfter = GetRectReferenceCorner(RectTransform, true);
		const FVector CornerOffset = CornerAfter - CornerBefore;
		FVector2D AnchoredPositionDelta = FVector2D::ZeroVector;
		if (RectTransform)
		{
			AnchoredPositionDelta = RectTransform->GetAnchoredPosition() - static_cast<FVector2D>(CornerOffset);
		}

		//AnchoredPositionHandle->SetValue(AnchoredPositionDelta, EPropertyValueSetFlags::InteractiveChange);
		TargetScriptPtr->SetAnchoredPosition(AnchoredPositionDelta);

		if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
		{
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->Modify();
					ComponentInstance->SetAnchoredPosition(AnchoredPositionDelta);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	}

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->Modify();
		TargetScriptPtr->MarkPackageDirty();
	}
	
	if (bCommitted)
	{
		bRecordSetPivotSmart = false;
		IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
	}
}

void FRectTransformComponentDetails::SetLocalRotation(float Value, int32 Axis, bool bCommitted)
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
	}

	if (!bRecordSetLocalRotation)
	{
		bRecordSetLocalRotation = true;
		IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("RectTransformComponentDetailsSetLocalRotation", "ChangeSetLocalRotation", "Change SetLocalRotation"));
	}

	FRotator LocalRotation = TargetScriptPtr->GetLocalRotation();

	if (Axis == 0)
	{
		LocalRotation.Roll = Value;
	}
	else if (Axis == 1)
	{
		LocalRotation.Pitch = Value;
	}
	else if (Axis == 2)
	{
		LocalRotation.Yaw = Value;
	}
	
	TargetScriptPtr->SetLocalRotation(LocalRotation);

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
	{
		TArray<UObject*> ArchetypeInstances;
		TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
		for (const auto& ArchetypeInstance : ArchetypeInstances)
		{
			const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
			if (ComponentInstance)
			{
				ComponentInstance->Modify();
				ComponentInstance->SetLocalRotation(LocalRotation);
				ComponentInstance->MarkPackageDirty();
			}
		}
	}

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->Modify();
		TargetScriptPtr->MarkPackageDirty();
	}
	
	if (bCommitted)
	{
		bRecordSetLocalRotation = false;
		IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
	}
}

void FRectTransformComponentDetails::SetLocalScale(float Value, int32 Axis, bool bCommitted)
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
	}

	if (!bRecordSetLocalScale)
	{
		bRecordSetLocalScale = true;
		IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("RectTransformComponentDetailsSetLocalScale", "ChangeSetLocalScale", "Change SetLocalScale"));
	}

	FVector LocalScale = TargetScriptPtr->GetLocalScale();

	if (Axis == 0)
	{
		LocalScale.X = Value;
	}
	else if (Axis == 1)
	{
		LocalScale.Y = Value;
	}
	else if (Axis == 2)
	{
		LocalScale.Z = Value;
	}
	
	TargetScriptPtr->SetLocalScale(LocalScale);

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsTemplate())
	{
		TArray<UObject*> ArchetypeInstances;
		TargetScriptPtr->GetArchetypeInstances(ArchetypeInstances);
		for (const auto& ArchetypeInstance : ArchetypeInstances)
		{
			const auto ComponentInstance = Cast<URectTransformComponent>(ArchetypeInstance);
			if (ComponentInstance)
			{
				ComponentInstance->Modify();
				ComponentInstance->SetLocalScale(LocalScale);
				ComponentInstance->MarkPackageDirty();
			}
		}
	}

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->Modify();
		TargetScriptPtr->MarkPackageDirty();
	}
	
	if (bCommitted)
	{
		bRecordSetLocalScale = false;
		IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
	}
}

TSharedRef<SWidget> FRectTransformComponentDetails::MakeNumericWidget(const TSharedRef<IPropertyHandle>& PropertyHandle, TAttribute<TOptional<float>> Value,
                                                                      const TDelegate<TDelegate<void(float)>::RetValType(float), FDefaultDelegateUserPolicy> OnValueChanged,
                                                                      const TDelegate<TDelegate<void(float, ETextCommit::Type)>::RetValType(float, ETextCommit::Type), FDefaultDelegateUserPolicy> OnValueChangedCommitted, bool bShowLabel, FText LabelText, float SliderExponent) const
{
	const TOptional<float> MinValue, MaxValue, SliderMinValue, SliderMaxValue;
	constexpr float Delta = 0;
	constexpr int32 ShiftMouseMovePixelPerDelta = 1;
	constexpr bool SupportDynamicSliderMaxValue = false;
	constexpr bool SupportDynamicSliderMinValue = false;

	if (bShowLabel)
	{
		return SNew(SNumericEntryBox<float>)
			.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.Value(Value)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.OnValueChanged(OnValueChanged)
			.OnValueCommitted(OnValueChangedCommitted)
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
			.Label()
			[
				SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LabelText)
			];		
	}
	else
	{
		return SNew(SNumericEntryBox<float>)
			.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
			.Value(Value)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
			.OnValueChanged(OnValueChanged)
			.OnValueCommitted(OnValueChangedCommitted)
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
			.Delta(Delta);
	}
}

#undef LOCTEXT_NAMESPACE
