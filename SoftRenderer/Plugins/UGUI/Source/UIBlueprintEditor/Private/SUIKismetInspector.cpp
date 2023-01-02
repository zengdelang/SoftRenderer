#include "SUIKismetInspector.h"
#include "DetailLayoutBuilder.h"
#include "EditorClassUtils.h"
#include "SSCSComponentEditor.h"
#include "SSCSEditor.h"
#include "UIBlueprintEditor.h"
#include "UIBlueprintEditorStyle.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "Kismet2/Kismet2NameValidators.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "KismetInspector"

//////////////////////////////////////////////////////////////////////////
// SUIKismetInspector

void SUIKismetInspector::Construct(const FArguments& InArgs, TSharedPtr<class FUIBlueprintEditor> InUIBlueprintEditorPtr)
{
	UIBlueprintEditorPtr = InUIBlueprintEditorPtr;
	
	SKismetInspector::Construct(InArgs);

	check( UIBlueprintEditorPtr.IsValid() );
	const TSharedPtr<SSCSComponentEditor> Editor = UIBlueprintEditorPtr.Pin()->GetSCSComponentEditor();
	check( Editor.IsValid() );
	Blueprint = UIBlueprintEditorPtr.Pin()->GetBlueprintObj();
	const UBlueprint* BlueprintObj = GetBlueprintObj();
	check(BlueprintObj != nullptr);

	TArray<FSCSComponentEditorTreeNodePtrType> Nodes = Editor->GetSelectedNodes();
	if (!Nodes.Num())
	{
		CachedNodePtr = nullptr;
	}
	else if (Nodes.Num() == 1)
	{
		CachedNodePtr = Nodes[0];
	}
	
	VariableNameEditableTextBox = SNew(SEditableTextBox)
		.SelectAllTextWhenFocused(true)
		.HintText(LOCTEXT("Name", "Name"))
		.Text(this, &SUIKismetInspector::OnGetVariableText)
		.OnTextChanged(this, &SUIKismetInspector::OnVariableTextChanged)
		.OnTextCommitted(this, &SUIKismetInspector::OnVariableTextCommitted)
		.IsReadOnly(this, &SUIKismetInspector::OnVariableCanRename)
		.Font(IDetailLayoutBuilder::GetDetailFont());

	const FText CategoryTooltip = NSLOCTEXT("BlueprintDetailsCustomization", "EditCategoryName_Tooltip", "The category of the variable; editing this will place the variable into another category or create a new one.");

	const TOptional<float> MinValue, MaxValue, SliderMinValue, SliderMaxValue;
	constexpr float SliderExponent = 1;
	constexpr float Delta = 0;
	constexpr int32 ShiftMouseMovePixelPerDelta = 1;
	constexpr bool SupportDynamicSliderMaxValue = false;
	constexpr bool SupportDynamicSliderMinValue = false;

	const TAttribute<bool> IsEnabled = TAttribute<bool>::Create([&]
	{
		return UIBlueprintEditorPtr.IsValid() && UIBlueprintEditorPtr.Pin()->InEditingMode();
	});

	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			RenderOpacityValue = Component->GetRenderOpacity();
			ZOrderValue = Component->GetZOrder();
		}
	}

	const TAttribute<FSlateColor> GameObjectOpacity = TAttribute<FSlateColor>::Create([&]
	{
		if (CachedNodePtr.IsValid())
		{
			if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
			{
				if(Component->IsEnabled())
				{
					return FLinearColor(1.0f,1.0f,1.0f,1.0f);
				}
				else
				{
					return FLinearColor(1.0f,1.0f,1.0f,0.1f);	
				}
			}
		}

		return FLinearColor(1.0f,1.0f,1.0f,1.0f);;
	});
	
	ChildSlot
	[
		SNew(SVerticalBox)
		.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("BlueprintInspector")))
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 2)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
			.Visibility(this, &SUIKismetInspector::GetBorderAreaVisibility)
			.IsEnabled(IsEnabled)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(6, 6, 6, 0)
				.VAlign(VAlign_Top)
				[
					SNew(SBox)
					.WidthOverride(32)
					.HeightOverride(32)
					[
						SNew(SImage)
						.ColorAndOpacity(GameObjectOpacity)
						.Image(this, &SUIKismetInspector::GetComponentIcon)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				.VAlign(VAlign_Top)
				.HAlign(HAlign_Fill)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.Padding(4)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.IsChecked(this, &SUIKismetInspector::GetIsEnabled)
							.OnCheckStateChanged(this, &SUIKismetInspector::HandleEnabledChanged)
							.Padding(FMargin(3,1,3,1))
						]
						
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.Padding(0, 0, 6, 0)
						.HAlign(HAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride(200.0f)
							.VAlign(VAlign_Center)
							[
								VariableNameEditableTextBox.ToSharedRef()
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SCheckBox)
							.IsChecked(this, &SUIKismetInspector::GetIsVariable)
							.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsVariableChanged)
							.Padding(FMargin(3,1,3,1))
							[
								SNew(STextBlock)
								.Text(LOCTEXT("IsVariable", "Is Variable"))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(15,0,0,0)
						[
							SAssignNew(ClassLinkArea, SBox)
						]
					]
					
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.5)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						.Padding(0, 0, 12, 0)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0, 0, 6, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("BlueprintDetailsCustomization", "BlueprintComponentDetails_VariableTooltipLabel", "Tooltip"))
								.MinDesiredWidth(45)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Fill)
							[
								SNew(SEditableTextBox)
									.Text(this, &SUIKismetInspector::OnGetTooltipText)
									.OnTextCommitted(this, &SUIKismetInspector::OnTooltipTextCommitted)
									.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.5)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0, 0, 6, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("BlueprintDetailsCustomization", "BlueprintComponentDetails_VariableCategoryLabel", "Category"))
								.ToolTipText(CategoryTooltip)
								.MinDesiredWidth(75)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							+ SHorizontalBox::Slot()
							.FillWidth(0.5)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Fill)
							[
								SAssignNew(VariableCategoryComboButton, SComboButton)
								.ContentPadding(FMargin(0,0,5,0))
								.IsEnabled(this, &SUIKismetInspector::OnVariableCategoryChangeEnabled)
								.ButtonContent()
								[
									SNew(SBorder)
									.BorderImage(FEditorStyle::GetBrush("NoBorder"))
									.Padding(FMargin(0, 0, 5, 0))
									[
										SNew(SEditableTextBox)
										.Text(this, &SUIKismetInspector::OnGetVariableCategoryText)
										.OnTextCommitted(this, &SUIKismetInspector::OnVariableCategoryTextCommitted)
										.ToolTipText(CategoryTooltip)
										.SelectAllTextWhenFocused(true)
										.RevertTextOnEscape(true)
										.Font(IDetailLayoutBuilder::GetDetailFont())
									]
								]
								.MenuContent()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									.MaxHeight(400.0f)
									[
										SAssignNew(VariableCategoryListView, SListView<TSharedPtr<FText>>)
										.ListItemsSource(&VariableCategorySource)
										.OnGenerateRow(this, &SUIKismetInspector::MakeVariableCategoryViewWidget)
										.OnSelectionChanged(this, &SUIKismetInspector::OnVariableCategorySelectionChanged)
									]
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(0.5)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						.Padding(0, 0, 12, 0)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0, 0, 6, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("BlueprintDetailsCustomization", "BlueprintComponentDetails_ZOrder", "Z Order"))
								.ToolTipText(LOCTEXT("BlueprintComponentDetails_ZOrder", "Z Order"))
								.MinDesiredWidth(45)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Fill)
							[
								SNew(SNumericEntryBox<float>)
								.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
								.Value(this, &SUIKismetInspector::GetZOrderValue)
								.OnValueChanged(this, &SUIKismetInspector::OnZOrderValueChanged )
								.OnValueCommitted(this, &SUIKismetInspector::OnZOrderValueCommitted )
								.LabelVAlign(VAlign_Center)
								.AllowSpin(true)
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
						]
						+ SHorizontalBox::Slot()
						.FillWidth(0.5)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Fill)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0, 0, 6, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("BlueprintDetailsCustomization", "BlueprintComponentDetails_RenderOpacity", "Render Opacity"))
								.ToolTipText(LOCTEXT("BlueprintComponentDetails_RenderOpacity", "Render Opacity"))
								.MinDesiredWidth(75)
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							+ SHorizontalBox::Slot()
							.FillWidth(1)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Fill)
							[
								SNew(SNumericEntryBox<float>)
								.EditableTextBoxStyle(&FCoreStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
								.UndeterminedString(NSLOCTEXT("PropertyEditor", "MultipleValues", "Multiple Values"))
								.Value(this, &SUIKismetInspector::GetRenderOpacityValue)
								.OnValueChanged(this, &SUIKismetInspector::OnRenderOpacityValueChanged )
								.OnValueCommitted(this, &SUIKismetInspector::OnRenderOpacityValueCommitted )
								.LabelVAlign(VAlign_Center)
								.AllowSpin(true)
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
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_EditableWhenInherited", "Editable When Inherited"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsEditableWhenInherited)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsEditableWhenInherited)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_Graying", "Graying"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsGraying)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsGraying)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_InvertColor", "Invert Color"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsInvertColor)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsInvertColor)
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_Interactable", "Interactable"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsInteractable)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsInteractable)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_BlockRaycasts", "Block Raycasts"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsBlockRaycasts)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsBlockRaycasts)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_IgnoreReversedGraphics", "Ignore Reversed Graphics"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsIgnoreReversedGraphics)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsIgnoreReversedGraphics)
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_IgnoreParentRenderOpacity", "Ignore Parent Render Opacity"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsIgnoreParentRenderOpacity)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsIgnoreParentRenderOpacity)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_IgnoreParentInteractable", "Ignore Parent Interactable"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsIgnoreParentInteractable)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsIgnoreParentInteractable)
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(4)
					.HAlign(HAlign_Fill)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_IgnoreParentBlockRaycasts", "Ignore Parent Block Raycasts"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsIgnoreParentBlockRaycasts)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsIgnoreParentBlockRaycasts)
								]
							]
						]
						+ SHorizontalBox::Slot()
						.FillWidth(1)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Right)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 6, 0)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(LOCTEXT("BlueprintComponentDetails_IgnoreParentReversedGraphics", "Ignore Parent Reversed Graphics"))
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
								+ SHorizontalBox::Slot()
								.AutoWidth()
								.VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SUIKismetInspector::GetIsIgnoreParentReversedGraphics)
									.OnCheckStateChanged(this, &SUIKismetInspector::HandleIsIgnoreParentReversedGraphics)
								]
							]
						]
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew( ContextualEditingBorderWidget, SBorder )
			.Padding(0)
			.BorderImage( FEditorStyle::GetBrush("NoBorder") )
		]
	];
}

void SUIKismetInspector::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if(bRefreshOnTick)
	{
		check( UIBlueprintEditorPtr.IsValid() );
		const TSharedPtr<SSCSComponentEditor> Editor = UIBlueprintEditorPtr.Pin()->GetSCSComponentEditor();
		check( Editor.IsValid() );

		TArray<FSCSComponentEditorTreeNodePtrType> Nodes = Editor->GetSelectedNodes();
		if (!Nodes.Num())
		{
			CachedNodePtr = nullptr;
		}
		else if (Nodes.Num() == 1)
		{
			CachedNodePtr = Nodes[0];
		}
		
		if (CachedNodePtr.IsValid())
		{
			if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
			{
				RenderOpacityValue = Component->GetRenderOpacity();
				ZOrderValue = Component->GetZOrder();
			}
		}

		// If only 1 valid selected object exists, update the class link to point to the right class.
		if (CachedNodePtr.IsValid() && CachedNodePtr->GetComponentTemplate())
		{
#if WITH_EDITORONLY_DATA
			UBehaviourComponent* BehaviourComponent = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate());
			if (BehaviourComponent)
			{
				BehaviourComponent->EditorVariableName = CachedNodePtr->GetVariableName();
			}
#endif
			ClassLinkArea->SetContent(FEditorClassUtils::GetSourceLink(CachedNodePtr->GetComponentTemplate()->GetClass(), TWeakObjectPtr<UObject>()));
		}
		else
		{
			ClassLinkArea->SetContent(SNullWidget::NullWidget);
		}

		PopulateVariableCategories();
	}
	
	SKismetInspector::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

const FSlateBrush* SUIKismetInspector::GetComponentIcon() const
{
	return FUIBlueprintEditorStyle::Get().GetBrush("UGUI.GameObject");
}

EVisibility SUIKismetInspector::GetBorderAreaVisibility() const
{
	if (UIBlueprintEditorPtr.IsValid())
	{
		const TSharedPtr<SSCSComponentEditor> Editor = UIBlueprintEditorPtr.Pin()->GetSCSComponentEditor();
		if (Editor.IsValid())
		{
			const TArray<FSCSComponentEditorTreeNodePtrType> Nodes = Editor->GetSelectedNodes();
			const EVisibility AreaVisibility =  (Nodes.Num() == 1) ? EVisibility::Visible : EVisibility::Collapsed;
			if (AreaVisibility == EVisibility::Visible)
			{
				if (CachedNodePtr == nullptr || CachedNodePtr->GetNodeType() != FSCSComponentEditorTreeNode::ComponentNode)
				{
					return EVisibility::Collapsed;
				}

				if (CachedNodePtr.IsValid())
				{
					UBehaviourComponent* BehaviourComponent = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate());
					if (!BehaviourComponent)
					{
						return EVisibility::Collapsed;
					}
				}
			}
			return AreaVisibility;
		}
	}
	return EVisibility::Collapsed;
}

FText SUIKismetInspector::OnGetVariableText() const
{
	if (CachedNodePtr.IsValid())
	{
		return FText::FromName(CachedNodePtr->GetVariableName());
	}
	return FText();
}

void SUIKismetInspector::OnVariableTextChanged(const FText& InNewText)
{
	if (CachedNodePtr.IsValid())
	{
		bIsVariableNameInvalid = true;

		const FString& NewTextStr = InNewText.ToString();

		if (const USCS_Node* SCS_Node = CachedNodePtr->GetSCSNode())
		{
			if (!NewTextStr.IsEmpty() && !FComponentEditorUtils::IsValidVariableNameString(SCS_Node->ComponentTemplate, NewTextStr))
			{
				VariableNameEditableTextBox->SetError(LOCTEXT("ComponentVariableRenameFailed_NotValid", "This name is reserved for engine use."));
				return;
			}

			if (!FComponentEditorUtils::IsComponentNameAvailable(NewTextStr, SCS_Node->ComponentTemplate->GetOuter(), SCS_Node->ComponentTemplate))
			{
				VariableNameEditableTextBox->SetError(FText::Format(LOCTEXT("ComponentVariableRenameFailed_InUse", "{0} is in use by another variable or function!"), InNewText));
				return;
			}
		}

		const TSharedPtr<INameValidatorInterface> VariableNameValidator = MakeShareable(new FKismetNameValidator(GetBlueprintObj(), CachedNodePtr->GetVariableName()));

		const EValidatorResult ValidatorResult = VariableNameValidator->IsValid(NewTextStr);
		if(ValidatorResult == EValidatorResult::AlreadyInUse)
		{
			VariableNameEditableTextBox->SetError(FText::Format(LOCTEXT("ComponentVariableRenameFailed_InUse", "{0} is in use by another variable or function!"), InNewText));
		}
		else if(ValidatorResult == EValidatorResult::EmptyName)
		{
			VariableNameEditableTextBox->SetError(LOCTEXT("RenameFailed_LeftBlank", "Names cannot be left blank!"));
		}
		else if(ValidatorResult == EValidatorResult::TooLong)
		{
			VariableNameEditableTextBox->SetError(FText::Format( LOCTEXT("RenameFailed_NameTooLong", "Names must have fewer than {0} characters!"), FText::AsNumber( FKismetNameValidator::GetMaximumNameLength())));
		}
		else
		{
			bIsVariableNameInvalid = false;
			VariableNameEditableTextBox->SetError(FText::GetEmpty());
		}
	}
}

void SUIKismetInspector::OnVariableTextCommitted(const FText& InNewName, ETextCommit::Type InTextCommit)
{
	if ( !bIsVariableNameInvalid )
	{
		check(CachedNodePtr.IsValid());

		USCS_Node* SCS_Node = CachedNodePtr->GetSCSNode();
		if(SCS_Node != nullptr)
		{
			const FScopedTransaction Transaction( LOCTEXT("RenameComponentVariable", "Rename Component Variable") );
			FBlueprintEditorUtils::RenameComponentMemberVariable(GetBlueprintObj(), CachedNodePtr->GetSCSNode(), FName( *InNewName.ToString() ));
		}
	}

	bIsVariableNameInvalid = false;
	VariableNameEditableTextBox->SetError(FText::GetEmpty());
}

bool SUIKismetInspector::OnVariableCanRename() const
{
	if (CachedNodePtr.IsValid())
	{
		return !CachedNodePtr->CanRename();
	}
	return false;
}

ECheckBoxState SUIKismetInspector::GetIsEnabled() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleEnabledChanged(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("EnabledChangedTransaction","Change Enabled") );
			
			Component->Modify();
				
			Component->SetEnabled(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetEnabled(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsVariable() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->bIsVariable ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsVariableChanged(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsVariableChangedTransaction","Change IsVariable") );
			
			Component->Modify();
				
			Component->bIsVariable = CheckState == ECheckBoxState::Checked;
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
		}
	}
}

FText SUIKismetInspector::OnGetTooltipText() const
{
	if (CachedNodePtr.IsValid())
	{
		const FName VarName = CachedNodePtr->GetVariableName();
		if (VarName != NAME_None)
		{
			FString Result;
			FBlueprintEditorUtils::GetBlueprintVariableMetaData(GetBlueprintObj(), VarName, nullptr, TEXT("tooltip"), Result);
			return FText::FromString(Result);
		}
	}
	return FText();
}

void SUIKismetInspector::OnTooltipTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit) const
{
	if (CachedNodePtr.IsValid())
	{
		FScopedTransaction Transaction( LOCTEXT("TooltipTextChangedTransaction","Change TooltipText") );
		FBlueprintEditorUtils::SetBlueprintVariableMetaData(GetBlueprintObj(), CachedNodePtr->GetVariableName(), nullptr, TEXT("tooltip"), NewText.ToString() );
	}
}

bool SUIKismetInspector::OnVariableCategoryChangeEnabled() const
{
	if (CachedNodePtr.IsValid())
	{
		return !CachedNodePtr->IsInheritedComponent();
	}
	return false;
}

FText SUIKismetInspector::OnGetVariableCategoryText() const
{
	if (CachedNodePtr.IsValid())
	{
		const FName VarName = CachedNodePtr->GetVariableName();
		if (VarName != NAME_None)
		{
			FText Category = FBlueprintEditorUtils::GetBlueprintVariableCategory(GetBlueprintObj(), VarName, nullptr);

			// Older blueprints will have their name as the default category
			if( Category.EqualTo(FText::FromString(GetBlueprintObj()->GetName())) )
			{
				return UEdGraphSchema_K2::VR_DefaultCategory;
			}
			else
			{
				return Category;
			}
		}
	}
	return FText();
}

void SUIKismetInspector::OnVariableCategoryTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
{
	if (CachedNodePtr.IsValid())
	{
		if (InTextCommit == ETextCommit::OnEnter || InTextCommit == ETextCommit::OnUserMovedFocus)
		{
			FBlueprintEditorUtils::SetBlueprintVariableCategory(GetBlueprintObj(), CachedNodePtr->GetVariableName(), nullptr, NewText);
			PopulateVariableCategories();
		}
	}
}

void SUIKismetInspector::OnVariableCategorySelectionChanged( TSharedPtr<FText> ProposedSelection, ESelectInfo::Type /*SelectInfo*/ ) const
{
	if (CachedNodePtr.IsValid())
	{
		const FName VarName = CachedNodePtr->GetVariableName();
		if (ProposedSelection.IsValid() && VarName != NAME_None)
		{
			const FText NewCategory = *ProposedSelection.Get();
			FBlueprintEditorUtils::SetBlueprintVariableCategory(GetBlueprintObj(), VarName, nullptr, NewCategory);

			check(VariableCategoryListView.IsValid());
			check(VariableCategoryComboButton.IsValid());

			VariableCategoryListView->ClearSelection();
			VariableCategoryComboButton->SetIsOpen(false);
		}
	}
}

TSharedRef< ITableRow > SUIKismetInspector::MakeVariableCategoryViewWidget( TSharedPtr<FText> Item, const TSharedRef< STableViewBase >& OwnerTable ) const
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
	[
		SNew(STextBlock)
			.Text(*Item.Get())
	];
}

void SUIKismetInspector::PopulateVariableCategories()
{
	UBlueprint* BlueprintObj = GetBlueprintObj();

	check(BlueprintObj);
	check(BlueprintObj->SkeletonGeneratedClass);

	TSet<FName> VisibleVariables;
	for (TFieldIterator<FProperty> PropertyIt(BlueprintObj->SkeletonGeneratedClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		FProperty* Property = *PropertyIt;

		if ((!Property->HasAnyPropertyFlags(CPF_Parm) && Property->HasAllPropertyFlags(CPF_BlueprintVisible)))
		{
			VisibleVariables.Add(Property->GetFName());
		}
	}

	FBlueprintEditorUtils::GetSCSVariableNameList(BlueprintObj, VisibleVariables);

	VariableCategorySource.Empty();
	VariableCategorySource.Add(MakeShareable(new FText(LOCTEXT("Default", "Default"))));
	for (const FName& VariableName : VisibleVariables)
	{
		FText Category = FBlueprintEditorUtils::GetBlueprintVariableCategory(BlueprintObj, VariableName, nullptr);
		if (!Category.IsEmpty() && !Category.EqualTo(FText::FromString(BlueprintObj->GetName())))
		{
			bool bNewCategory = true;
			for (int32 j = 0; j < VariableCategorySource.Num() && bNewCategory; ++j)
			{
				bNewCategory &= !VariableCategorySource[j].Get()->EqualTo(Category);
			}
			if (bNewCategory)
			{
				VariableCategorySource.Add(MakeShareable(new FText(Category)));
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsEditableWhenInherited() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->bEditableWhenInherited ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsEditableWhenInherited(ECheckBoxState CheckState)
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsEditableWhenInheritedChangedTransaction","Change IsEditableWhenInherited") );
			
			Component->Modify();
				
			Component->bEditableWhenInherited = CheckState == ECheckBoxState::Checked;
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsGraying() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsGraying() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsGraying(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsGrayingChangedTransaction","Change IsGraying") );
			
			Component->Modify();
				
			Component->SetGraying(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetGraying(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsInvertColor() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsInvertColor() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsInvertColor(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsInvertColorChangedTransaction","Change IsInvertColor") );
			
			Component->Modify();
				
			Component->SetInvertColor(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetInvertColor(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsInteractable() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsInteractable() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsInteractable(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsInteractableChangedTransaction","Change IsInteractable") );
			
			Component->Modify();
				
			Component->SetInteractable(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetInteractable(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsBlockRaycasts() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsBlockRaycasts() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsBlockRaycasts(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsBlockRaycastsChangedTransaction","Change IsBlockRaycasts") );
			
			Component->Modify();
				
			Component->SetBlockRaycasts(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetBlockRaycasts(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsIgnoreReversedGraphics() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsIgnoreReversedGraphics() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsIgnoreReversedGraphics(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsIgnoreReversedGraphicsChangedTransaction","Change IsIgnoreReversedGraphics") );
			
			Component->Modify();
				
			Component->SetIgnoreReversedGraphics(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetIgnoreReversedGraphics(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsIgnoreParentRenderOpacity() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsIgnoreParentRenderOpacity() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsIgnoreParentRenderOpacity(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsIgnoreParentRenderOpacityChangedTransaction","Change IsIgnoreParentRenderOpacity") );
			
			Component->Modify();
				
			Component->SetIgnoreParentRenderOpacity(CheckState == ECheckBoxState::Checked);

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetIgnoreParentRenderOpacity(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsIgnoreParentInteractable() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsIgnoreParentInteractable() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsIgnoreParentInteractable(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsIgnoreParentInteractableChangedTransaction","Change IsIgnoreParentInteractable") );
			
			Component->Modify();
				
			Component->SetIgnoreParentInteractable(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetIgnoreParentInteractable(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsIgnoreParentBlockRaycasts() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsIgnoreParentBlockRaycasts() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsIgnoreParentBlockRaycasts(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsIgnoreParentBlockRaycastsChangedTransaction","Change IsIgnoreParentBlockRaycasts") );
			
			Component->Modify();
				
			Component->SetIgnoreParentBlockRaycasts(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetIgnoreParentBlockRaycasts(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

ECheckBoxState SUIKismetInspector::GetIsIgnoreParentReversedGraphics() const
{
	TOptional<ECheckBoxState> Result;
	if (CachedNodePtr.IsValid())
	{
		if (const UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			const ECheckBoxState ComponentState = Component->IsIgnoreParentReversedGraphics() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			if (Result.IsSet() && ComponentState != Result.GetValue())
			{
				return ECheckBoxState::Undetermined;
			}
			Result = ComponentState;
		}
	}
	return Result.IsSet() ? Result.GetValue() : ECheckBoxState::Unchecked;
}

void SUIKismetInspector::HandleIsIgnoreParentReversedGraphics(ECheckBoxState CheckState) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("IsIgnoreParentReversedGraphicsChangedTransaction","Change IsIgnoreParentReversedGraphics") );
			
			Component->Modify();
				
			Component->SetIgnoreParentReversedGraphics(CheckState == ECheckBoxState::Checked);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetIgnoreParentReversedGraphics(CheckState == ECheckBoxState::Checked);
				}
			}
		}
	}
}

TOptional<float> SUIKismetInspector::GetZOrderValue() const
{
	return ZOrderValue;
}

void SUIKismetInspector::OnZOrderValueChanged(float NewZOrder)
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			ZOrderValue = NewZOrder;
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetZOrder(NewZOrder);
				}
			}
		}
	}
}

void SUIKismetInspector::OnZOrderValueCommitted(float NewZOrder, ETextCommit::Type CommitType) const
{
	if (CachedNodePtr.IsValid())
	{
		FScopedTransaction Transaction( LOCTEXT("ZOrderChangedTransaction","Change the Z Order") );
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			Component->Modify();
				
			Component->SetZOrder(NewZOrder);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetZOrder(NewZOrder);
				}
			}
		}
	}
}

TOptional<float> SUIKismetInspector::GetRenderOpacityValue() const
{
	return RenderOpacityValue;
}

void SUIKismetInspector::OnRenderOpacityValueChanged(float NewRenderOpacity)
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			RenderOpacityValue = NewRenderOpacity;

			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetRenderOpacity(NewRenderOpacity);
				}
			}
		}
	}
}

void SUIKismetInspector::OnRenderOpacityValueCommitted(float NewRenderOpacity, ETextCommit::Type CommitType) const
{
	if (CachedNodePtr.IsValid())
	{
		if (UBehaviourComponent* Component = Cast<UBehaviourComponent>(CachedNodePtr->GetComponentTemplate()))
		{
			FScopedTransaction Transaction( LOCTEXT("RenderOpacityChangedTransaction","Change the Render Opacity") );
			
			Component->Modify();
				
			Component->SetRenderOpacity(NewRenderOpacity);
			
			// Update the actor before leaving.
			Component->MarkPackageDirty();

			FBlueprintEditorUtils::MarkBlueprintAsModified(CachedNodePtr->GetBlueprint());
			
			TArray<UObject*> ArchetypeInstances;
			Component->GetArchetypeInstances(ArchetypeInstances);
			for (const auto& ArchetypeInstance : ArchetypeInstances)
			{
				const auto ComponentInstance = Cast<UBehaviourComponent>(ArchetypeInstance);
				if (ComponentInstance)
				{
					ComponentInstance->SetRenderOpacity(NewRenderOpacity);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
