#include "Slate/SUIAnchorPresetPanel.h"
#include "Slate/SUIAnchorPreset.h"
#include "DetailLayoutBuilder.h"
#include "UIBlueprintEditorModule.h"
#include "Brushes/SlateBorderBrush.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Widgets/Layout/SConstraintCanvas.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

SUIAnchorPresetPanel::SUIAnchorPresetPanel()
	: SelectHorizontalAnchorType()
	, SelectVerticalAnchorType()
{

}

void SUIAnchorPresetPanel::Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InAnchorMax, TSharedPtr<IPropertyHandle> InAnchorMin,TSharedPtr<IPropertyHandle> InPivot,
	TSharedPtr<IPropertyHandle> InAnchoredPositionHandle, TWeakObjectPtr<class URectTransformComponent> InTargetScriptPtr)
{
	AnchorMinHandle = InAnchorMin;
	AnchorMaxHandle = InAnchorMax;
	PivotHandle = InPivot;
	AnchoredPositionHandle = InAnchoredPositionHandle;
	SizeDeltaHandle = InArgs._SizeDeltaHandle.Get();
	TargetScriptPtr = InTargetScriptPtr;

	AnchorMin = InArgs._AnchorMin;
	AnchorMax = InArgs._AnchorMax;

	DefaultAnchorMin = FVector2D::ZeroVector;
	AnchorMinHandle->GetValue(DefaultAnchorMin);
	
	DefaultanchorMax = FVector2D::ZeroVector;
	AnchorMaxHandle->GetValue(DefaultanchorMax);
	
	DefaultanchoredPosition = FVector2D::ZeroVector;
	AnchoredPositionHandle->GetValue(DefaultanchoredPosition);
	
	DefaultSizeDelta = FVector2D::ZeroVector;
	SizeDeltaHandle->GetValue(DefaultSizeDelta);
	
	SelectHorizontalAnchorType = GetHorizontalAnchorType(AnchorMin.Get(), AnchorMax.Get());
	SelectVerticalAnchorType = GetVerticalAnchorType(AnchorMin.Get(), AnchorMax.Get());
	
	bool bShowPivot = false;
	bool bShowPosition = true;
	
	const TAttribute<bool> bShowPivotLambda = TAttribute<bool>::Create([bShowPivot]
	{
		if (FSlateApplication::Get().GetModifierKeys().IsShiftDown())
			return true;

		return false;
	});

	const TAttribute<bool> bShowPositionLambda = TAttribute<bool>::Create([bShowPosition]
	{
		if (FSlateApplication::Get().GetModifierKeys().IsAltDown())
			return false;

		return true;
	});

	const TAttribute<EVisibility> bShowHorizonCustom = TAttribute<EVisibility>::Create([this]
	{
		const auto HorizontalAnchorType = GetHorizontalAnchorType(AnchorMin.Get(), AnchorMax.Get());

		if (HorizontalAnchorType == EHorizontalAnchorType::Custom)
		{
			return EVisibility::All;
		}

		return EVisibility::Hidden;
	});

	const TAttribute<EVisibility> bShowVerticalCustom = TAttribute<EVisibility>::Create([this]
	{
		const auto VerticalAnchorType = GetVerticalAnchorType(AnchorMin.Get(), AnchorMax.Get());

		if (VerticalAnchorType == EVerticalAnchorType::Custom)
		{
			return EVisibility::All;
		}

		return EVisibility::Hidden;
	});

	const TAttribute<EVisibility> bShowCustom = TAttribute<EVisibility>::Create([bShowHorizonCustom, bShowVerticalCustom]
	{
		if (bShowHorizonCustom.Get() == EVisibility::All || bShowVerticalCustom.Get() == EVisibility::All)
			return EVisibility::All;

		return EVisibility::Hidden;
	});

	const TAttribute<EHorizontalAnchorType> SelectHorizontalLambda = TAttribute<EHorizontalAnchorType>::Create([this]
	{
		return this->SelectHorizontalAnchorType;}
	);

	const TAttribute<EVerticalAnchorType> SelectVerticalLambda = TAttribute<EVerticalAnchorType>::Create([this]
	{
		return this->SelectVerticalAnchorType;
	});

	const TAttribute<bool> bShowSelectedBound = TAttribute<bool>::Create([bShowCustom]
	{
		return bShowCustom.Get() == EVisibility::Hidden;
	});

	const TAttribute<bool> bShowVerticalSelectedBound = TAttribute<bool>::Create([bShowVerticalCustom]
	{
		return bShowVerticalCustom.Get() == EVisibility::Hidden;
	});

	const TAttribute<bool> bShowHorizontalSelectedBound = TAttribute<bool>::Create([bShowHorizonCustom]
	{
		return bShowHorizonCustom.Get() == EVisibility::Hidden;
	});
	
	static FSlateBrush BgBorderBrush;
	BgBorderBrush.ImageType = ESlateBrushImageType::FullColor;
	BgBorderBrush.TintColor = FLinearColor(32 / 255.0f, 32 / 255.0f, 32 / 255.0f);
	
	static FSlateBrush BorderBrush;
	BorderBrush.ImageType = ESlateBrushImageType::FullColor;
	BorderBrush.TintColor = FLinearColor(14 / 255.0f, 14 / 255.0f, 14 / 255.0f);

	static FSlateBrush LineBorderBrush;
	LineBorderBrush.ImageType = ESlateBrushImageType::FullColor;
	LineBorderBrush.TintColor = FLinearColor(50 / 255.0f, 50 / 255.0f, 50 / 255.0f);

	static FSlateBrush DarkBgBorderBrush;
	DarkBgBorderBrush.ImageType = ESlateBrushImageType::FullColor;
	DarkBgBorderBrush.TintColor = FLinearColor(10 / 255.0f, 10 / 255.0f, 10 / 255.0f);
	
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(&BgBorderBrush)
		.Padding(FMargin(1.0f))
		[
			SNew(SBorder)
			.BorderImage(&BorderBrush)
			.Padding(FMargin(0))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6, 4, 0, 0)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UIAnchorPresetPanel_Title1", "Anchor Presets"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(6, 2, 0, 0)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("UIAnchorPresetPanel_Title2", "Shift: Also set pivot          Alt: Also set position"))
				]
				+ SVerticalBox::Slot()
				.Padding(0, 4, 0, 0)
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(1)
					.Padding(0)
					[
						SNew(SBorder)
						.Padding(0)
						.BorderImage(&LineBorderBrush)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBorder)
					.Padding(FMargin(8, 4, 2, 0))
					.BorderImage(&DarkBgBorderBrush)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SBox)
							.Visibility(bShowCustom)
							.WidthOverride(60)
							.HeightOverride(60)
							.VAlign(VAlign_Fill)
							.HAlign(HAlign_Fill)
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
										.Visibility(bShowVerticalCustom)
										.RenderTransform(FSlateRenderTransform(FQuat2D(-PI / 2)))
										.Justification(ETextJustify::Left)
										.Font(IDetailLayoutBuilder::GetDetailFont())
										.Text(LOCTEXT("RectTransformComponent_VerticalTextCustom", "Custom"))
									]
								]
								+ SConstraintCanvas::Slot()
								.Offset(FMargin(9 + 21.5, 6, 0, 0))
								.AutoSize(true)
								[
									SNew(STextBlock)
									.Visibility(bShowHorizonCustom)
									.Justification(ETextJustify::Center)
									.Font(IDetailLayoutBuilder::GetDetailFont())
									.Text(LOCTEXT("RectTransformComponent_HorizontalTextCustom", "Custom"))
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
										SNew(SBorder)
										.Padding(0)
										.BorderImage(FEditorStyle::GetBrush("DetailsView.CategoryMiddle"))
										.BorderBackgroundColor(FLinearColor(0.8, 0.8, 0.8, 1))
										[
											SNew(SUIAnchorPreset)
											.AnchorMax(FVector2D(0.75, 0.75))
											.AnchorMin(FVector2D(0.75, 0.75))
										]
									]
								]
							]
						 ]
						 + SHorizontalBox::Slot()
						 .AutoWidth()
						 .Padding(0, -4, 13, 0)
						 [
							 SNew(SBox)
							 .WidthOverride(1)
							 .Padding(0)
							 [
								 SNew(SBorder)
								 .Padding(0)
								 .BorderImage(&LineBorderBrush)
							 ]
						 ]
						 + SHorizontalBox::Slot()
						 .AutoWidth()
					     [
							MakeHorizontalEdgeAnchorItem(FVector2D(0.0, 0.75), FVector2D(0.0, 0.75), EHorizontalAnchorType::Left, EVerticalAnchorType::Custom, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_HorizontalTextLeft", "Left")), bShowHorizontalSelectedBound)
					     ]
					     + SHorizontalBox::Slot()
						 .AutoWidth()
					     [
							MakeHorizontalEdgeAnchorItem(FVector2D(0.5, 0.75), FVector2D(0.5, 0.75), EHorizontalAnchorType::Center, EVerticalAnchorType::Custom, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_HorizontalTextCenter", "Center")), bShowHorizontalSelectedBound)
					     ]
					     + SHorizontalBox::Slot()
						 .AutoWidth()
					     [
							MakeHorizontalEdgeAnchorItem(FVector2D(1.0, 0.75), FVector2D(1.0, 0.75), EHorizontalAnchorType::Right, EVerticalAnchorType::Custom, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_HorizontalTextRight", "Right")), bShowHorizontalSelectedBound)
					     ]
					     + SHorizontalBox::Slot()
						 .AutoWidth()
						 .Padding(0, 0, 20, 0)
					     [
							SNew(SSpacer)
					     ]
					     + SHorizontalBox::Slot()
						 .AutoWidth()
					     [
							MakeHorizontalEdgeAnchorItem(FVector2D(0.0, 0.75), FVector2D(1.0, 0.75), EHorizontalAnchorType::Stretch, EVerticalAnchorType::Custom, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_HorizontalTextStretch", "Stretch")), bShowHorizontalSelectedBound)
					     ]
					]
				]
				+ SVerticalBox::Slot()
				.Padding(0, 0, 0, 0)
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(1)
					.Padding(0)
					[
						SNew(SBorder)
						.Padding(0)
						.BorderImage(&LineBorderBrush)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBorder)
						.Padding(FMargin(8, 13, 0, 2))
						.BorderImage(&DarkBgBorderBrush)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeVerticalEdgeAnchorItem(FVector2D(0.75, 1.0), FVector2D(0.75, 1.0), EHorizontalAnchorType::Custom, EVerticalAnchorType::Top, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_VerticalTextTop", "Top")), bShowVerticalSelectedBound)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeVerticalEdgeAnchorItem(FVector2D(0.75, 0.5), FVector2D(0.75, 0.5), EHorizontalAnchorType::Custom, EVerticalAnchorType::Middle, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_VerticalTextMiddle", "Middle")), bShowVerticalSelectedBound)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeVerticalEdgeAnchorItem(FVector2D(0.75, 0.0), FVector2D(0.75, 0.0), EHorizontalAnchorType::Custom, EVerticalAnchorType::Bottom, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_VerticalTextBottom", "Bottom")), bShowVerticalSelectedBound)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 20, 0, 0)
							[
								SNew(SSpacer)
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								MakeVerticalEdgeAnchorItem(FVector2D(0.75, 0.0), FVector2D(0.75, 1.0), EHorizontalAnchorType::Custom, EVerticalAnchorType::Stretch, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, FText(LOCTEXT("RectTransformComponent_VerticalTextStretch", "Stretch")), bShowVerticalSelectedBound)
							]
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(1)
						.Padding(0)
						[
							SNew(SBorder)
							.Padding(0)
							.BorderImage(&LineBorderBrush)
						]
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(13, 13, 0, 0)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 1.0), FVector2D(0.0, 1.0), FVector2D(0, 1.0), EHorizontalAnchorType::Left, EVerticalAnchorType::Top, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.5, 1.0), FVector2D(0.5, 1.0), FVector2D(0.5, 1.0), EHorizontalAnchorType::Center, EVerticalAnchorType::Top, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(1.0, 1.0), FVector2D(1.0, 1.0), FVector2D(1.0, 1.0), EHorizontalAnchorType::Right, EVerticalAnchorType::Top, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 .Padding(0, 0, 20, 0)
							 [
								SNew(SSpacer)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 1.0), FVector2D(1.0, 1.0), FVector2D(0.5, 1.0), EHorizontalAnchorType::Stretch, EVerticalAnchorType::Top, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.5), FVector2D(0.0, 0.5), FVector2D(0.0, 0.5), EHorizontalAnchorType::Left, EVerticalAnchorType::Middle, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.5, 0.5), FVector2D(0.5, 0.5), FVector2D(0.5,0.5), EHorizontalAnchorType::Center, EVerticalAnchorType::Middle, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(1.0, 0.5), FVector2D(1.0, 0.5), FVector2D(1.0, 0.5), EHorizontalAnchorType::Right, EVerticalAnchorType::Middle, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 .Padding(0, 0, 20, 0)
							 [
								SNew(SSpacer)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.5), FVector2D(1.0, 0.5), FVector2D(0.5, 0.5), EHorizontalAnchorType::Stretch, EVerticalAnchorType::Middle, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.0), FVector2D(0.0, 0.0), FVector2D(0.0, 0.0), EHorizontalAnchorType::Left, EVerticalAnchorType::Bottom, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.5, 0.0), FVector2D(0.5, 0.0), FVector2D(0.5, 0.0), EHorizontalAnchorType::Center, EVerticalAnchorType::Bottom, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(1.0, 0.0), FVector2D(1.0, 0.0), FVector2D(1.0, 0.0), EHorizontalAnchorType::Right, EVerticalAnchorType::Bottom, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 .Padding(0, 0, 20, 0)
							 [
								SNew(SSpacer)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.0), FVector2D(1.0, 0.0), FVector2D(0.5, 0.0), EHorizontalAnchorType::Stretch, EVerticalAnchorType::Bottom, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 20, 0, 0)
						[
							SNew(SHorizontalBox)
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.0), FVector2D(0.0, 1.0), FVector2D(0.0, 0.5), EHorizontalAnchorType::Left, EVerticalAnchorType::Stretch, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.5, 0.0), FVector2D(0.5, 1.0), FVector2D(0.5, 0.5), EHorizontalAnchorType::Center, EVerticalAnchorType::Stretch, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(1.0, 0.0), FVector2D(1.0, 1.0), FVector2D(1.0, 0.5), EHorizontalAnchorType::Right, EVerticalAnchorType::Stretch, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 .Padding(0, 0, 20, 0)
							 [
								SNew(SSpacer)
							 ]
							 + SHorizontalBox::Slot()
							 .AutoWidth()
							 [
								MakeAnchorItem(FVector2D(0.0, 0.0), FVector2D(1.0, 1.0), FVector2D(0.5, 0.5), EHorizontalAnchorType::Stretch, EVerticalAnchorType::Stretch, bShowPivotLambda, bShowPositionLambda, SelectHorizontalLambda, SelectVerticalLambda, bShowSelectedBound)
							 ]
						]
					]
				]
			]
		]
	];
}

FReply SUIAnchorPresetPanel::OnClick(const FGeometry&, const FPointerEvent&, FVector2D InAnchorMin, FVector2D InAnchorMax,FVector2D InPivot, FAnchorType InAnchorType)
{
	IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(TargetScriptPtr.Get(), NSLOCTEXT("SUIAnchorPresetPanel", "ChangeRectTransformPreset", "Change RectTransform Preset"));
	
	SelectHorizontalAnchorType = InAnchorType.HorizontalAnchorType;
	SelectVerticalAnchorType = InAnchorType.VerticalAnchorType;

	FVector2D RefPosition = FVector2D::ZeroVector;
	const bool bDoPosition = FSlateApplication::Get().GetModifierKeys().IsAltDown();

	if (bDoPosition)
	{
		//AnchorMinHandle->SetValue(DefaultAnchorMin, EPropertyValueSetFlags::NotTransactable);
		TargetScriptPtr->SetAnchorMin(DefaultAnchorMin);
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
					ComponentInstance->SetAnchorMin(DefaultAnchorMin);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
		
		//AnchorMaxHandle->SetValue(DefaultanchorMax, EPropertyValueSetFlags::NotTransactable);
		TargetScriptPtr->SetAnchorMax(DefaultanchorMax);
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
					ComponentInstance->SetAnchorMax(DefaultanchorMax);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
		
		//AnchoredPositionHandle->SetValue(DefaultanchoredPosition, EPropertyValueSetFlags::NotTransactable);
		TargetScriptPtr->SetAnchoredPosition(DefaultanchoredPosition);
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
					ComponentInstance->SetAnchoredPosition(DefaultanchoredPosition);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
		
		//SizeDeltaHandle->SetValue(DefaultSizeDelta, EPropertyValueSetFlags::NotTransactable);
		TargetScriptPtr->SetSizeDelta(DefaultSizeDelta);
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
					ComponentInstance->SetSizeDelta(DefaultSizeDelta);
					ComponentInstance->MarkPackageDirty();
				}
			}
		}
	}
	
	if (FSlateApplication::Get().GetModifierKeys().IsShiftDown())
	{
		SetPivotSmart(InPivot.X, 0, true);
		SetPivotSmart(InPivot.Y, 1, true);
	}
	
	SetAnchorSmart(InAnchorMin.X, 0, false, true);
	SetAnchorSmart(InAnchorMin.Y, 1, false, true);

	SetAnchorSmart(InAnchorMax.X, 0, true, true);
	SetAnchorSmart(InAnchorMax.Y, 1, true, true);
	
	if (bDoPosition && TargetScriptPtr.IsValid())
	{
		if (SelectHorizontalAnchorType == EHorizontalAnchorType::Left)
		{
			RefPosition.X = TargetScriptPtr->GetOffsetMin().X;
		}
		else if (SelectHorizontalAnchorType == EHorizontalAnchorType::Right)
		{
			RefPosition.X = TargetScriptPtr->GetOffsetMax().X;
		}
		else if (SelectHorizontalAnchorType == EHorizontalAnchorType::Center ||
			SelectHorizontalAnchorType == EHorizontalAnchorType::Stretch)
		{
			RefPosition.X = (TargetScriptPtr->GetOffsetMin().X + TargetScriptPtr->GetOffsetMax().X) * 0.5f;
		}
		
		if (SelectVerticalAnchorType == EVerticalAnchorType::Bottom)
		{
			RefPosition.Y = TargetScriptPtr->GetOffsetMin().Y;
		}
		else if (SelectVerticalAnchorType == EVerticalAnchorType::Top)
		{
			RefPosition.Y = TargetScriptPtr->GetOffsetMax().Y;
		}
		else if (SelectVerticalAnchorType == EVerticalAnchorType::Middle ||
			SelectVerticalAnchorType == EVerticalAnchorType::Stretch)
		{
			RefPosition.Y = (TargetScriptPtr->GetOffsetMin().Y + TargetScriptPtr->GetOffsetMax().Y) * 0.5f;
		}
		
		// Handle position
		FVector2D RectPosition = FVector2D::ZeroVector;
		AnchoredPositionHandle->GetValue(RectPosition);
		
		RectPosition -= RefPosition;
		//AnchoredPositionHandle->SetValue(RectPosition, EPropertyValueSetFlags::NotTransactable);
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
		
		// Handle sizeDelta
		FVector2D RectSizeDelta = FVector2D::ZeroVector;
		SizeDeltaHandle->GetValue(RectSizeDelta);
		
		if (SelectHorizontalAnchorType == EHorizontalAnchorType::Stretch)
		{
			RectSizeDelta[0] = 0;
		}

		if (SelectVerticalAnchorType == EVerticalAnchorType::Stretch)
		{
			RectSizeDelta[1] = 0;
		}

		if (SelectHorizontalAnchorType == EHorizontalAnchorType::Stretch ||
			SelectVerticalAnchorType == EVerticalAnchorType::Stretch)
		{
			//SizeDeltaHandle->SetValue(RectSizeDelta, EPropertyValueSetFlags::NotTransactable);
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
	}

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		TargetScriptPtr->Modify();
		TargetScriptPtr->MarkPackageDirty();
	}

	IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(TargetScriptPtr.Get());
	return FReply::Handled();
}

FReply SUIAnchorPresetPanel::OnEdgeClick(const FGeometry& Geometry, const FPointerEvent& Event, FVector2D InAnchorMin, FVector2D InAnchorMax, FAnchorType InAnchorType)
{
	FVector2D CorrectAnchorMin;
	FVector2D CorrectAnchorMax;
	FVector2D CorrectPivot = FVector2D::ZeroVector;

	if(InAnchorType.HorizontalAnchorType == EHorizontalAnchorType::Custom)
	{
		SelectVerticalAnchorType = InAnchorType.VerticalAnchorType;

	    CorrectAnchorMin = FVector2D(AnchorMin.Get().X, InAnchorMin.Y);
	    CorrectAnchorMax = FVector2D(AnchorMax.Get().X, InAnchorMax.Y);
	}

	if(InAnchorType.VerticalAnchorType == EVerticalAnchorType::Custom)
	{
		SelectHorizontalAnchorType = InAnchorType.HorizontalAnchorType;
	
	    CorrectAnchorMin = FVector2D(InAnchorMin.X,AnchorMin.Get().Y);
	    CorrectAnchorMax = FVector2D(InAnchorMax.X,AnchorMax.Get().Y);
	}

	const FAnchorType Anchor = FAnchorType(SelectHorizontalAnchorType, SelectVerticalAnchorType);

	if(FSlateApplication::Get().GetModifierKeys().IsShiftDown())
	{
		CorrectPivot = GetPivot(Anchor);
	}

	OnClick(Geometry, Event, CorrectAnchorMin, CorrectAnchorMax, CorrectPivot, Anchor);

	return FReply::Handled();
}

void SUIAnchorPresetPanel::SetAnchorSmart(float Value, int32 Axis, bool bIsMax, bool bSmart) const
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
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
		//AnchorMaxHandle->SetValue(RectAnchorMax, EPropertyValueSetFlags::NotTransactable);
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
		//AnchorMinHandle->SetValue(Other, EPropertyValueSetFlags::NotTransactable);
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
		//AnchorMinHandle->SetValue(RectAnchorMin, EPropertyValueSetFlags::NotTransactable);
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
		//AnchorMaxHandle->SetValue(Other, EPropertyValueSetFlags::NotTransactable);
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
		//AnchoredPositionHandle->SetValue(RectPosition, EPropertyValueSetFlags::NotTransactable);
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

		//SizeDeltaHandle->SetValue(RectSizeDelta, EPropertyValueSetFlags::NotTransactable);
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
}

void SUIAnchorPresetPanel::SetPivotSmart(float Value, int32 Axis, bool bSmart) const
{
	if (!TargetScriptPtr.IsValid())
	{
		return;
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
	
	//PivotHandle->SetValue(RectPivot, EPropertyValueSetFlags::NotTransactable);
	TargetScriptPtr->SetPivot(RectPivot);
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
		
		//AnchoredPositionHandle->SetValue(AnchoredPositionDelta, EPropertyValueSetFlags::NotTransactable);
		TargetScriptPtr->SetAnchoredPosition(AnchoredPositionDelta);
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
}

URectTransformComponent* SUIAnchorPresetPanel::GetParentComponent() const
{
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> ArchetypeInstances;
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
			}
		}
		else
		{
			return Cast<URectTransformComponent>(TargetScriptPtr->GetAttachParent());
		}
	}
	return nullptr;
}

FVector SUIAnchorPresetPanel::GetRectReferenceCorner(const URectTransformComponent* RectTransform, bool bWorldSpace)
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

EVerticalAnchorType SUIAnchorPresetPanel::GetVerticalAnchorType( FVector2D InAnchorMin,  FVector2D InAnchorMax)
{
	if (InAnchorMin.Y == InAnchorMax.Y)
	{
		if (InAnchorMin.Y == 0)
		{
			return EVerticalAnchorType::Bottom;
		}

		if (InAnchorMin.Y == 0.5)
		{
			return EVerticalAnchorType::Middle;
		}

		if (InAnchorMin.Y == 1)
		{
			return EVerticalAnchorType::Top;
		}
	}
	else
	{
		if (InAnchorMin.Y == 0 && InAnchorMax.Y == 1)
		{
			return EVerticalAnchorType::Stretch;
		}
	}

	return EVerticalAnchorType::Custom;
}

EHorizontalAnchorType SUIAnchorPresetPanel::GetHorizontalAnchorType( FVector2D InAnchorMin,  FVector2D InAnchorMax)
{
	if (InAnchorMin.X == InAnchorMax.X)
	{
		if (InAnchorMin.X == 0)
		{
			return EHorizontalAnchorType::Left;
		}

		if (InAnchorMin.X == 0.5)
		{
			return EHorizontalAnchorType::Center;
		}

		if (InAnchorMin.X == 1)
		{
			return EHorizontalAnchorType::Right;
		}
	}
	else
	{
		if (InAnchorMin.X == 0 && InAnchorMax.X == 1)
		{
			return EHorizontalAnchorType::Stretch;
		}
	}

	return EHorizontalAnchorType::Custom;
}

FVector2D SUIAnchorPresetPanel::GetPivot(FAnchorType AnchorType)
{
	FVector2D Result = FVector2D::ZeroVector;

	if(AnchorType.VerticalAnchorType == EVerticalAnchorType::Middle)
	{
		Result.Y = 0.5f;
	}
	else if(AnchorType.VerticalAnchorType == EVerticalAnchorType::Top)
	{
		Result.Y = 1.0f;
	}

	if(AnchorType.HorizontalAnchorType == EHorizontalAnchorType::Center)
	{
		Result.X = 0.5f;
	}
	else if(AnchorType.HorizontalAnchorType == EHorizontalAnchorType::Right)
	{
		Result.X = 1.0f;
	}

	return  Result;
}

TSharedRef<SWidget> SUIAnchorPresetPanel::MakeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InPivot, EHorizontalAnchorType InHorizontalAnchorType, EVerticalAnchorType InVerticalAnchorType, TAttribute<bool> InShowPivotLambda, TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda, TAttribute<EVerticalAnchorType> SelectVerticalLambda, TAttribute<bool> bShowSelectedBound)
{
	return SNew(SBox)
	.WidthOverride(42)
	.HeightOverride(42)
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Alignment(FVector2D(0, 0))
		.AutoSize(true)
		[
			SNew(SBox)
			.WidthOverride(42)
			.HeightOverride(42)
			[
				SNew(SBorder)
				.Padding(0)
				.BorderImage(FEditorStyle::GetNoBrush())
				.OnMouseButtonDown(this, &SUIAnchorPresetPanel::OnClick, InAnchorMin, InAnchorMax, InPivot,FAnchorType( InHorizontalAnchorType, InVerticalAnchorType))
				[
					SNew(SUIAnchorPreset)
					.CurrentHorizontalAnchorType(SelectHorizontalLambda)
					.CurrentVerticalAnchorType(SelectVerticalLambda)
					.bShowPivot(InShowPivotLambda)
					.bShowCenterPosBound(InShowPositionLambda)
					.PivotPosition(InPivot)
					.AnchorMin(InAnchorMin)
					.AnchorMax(InAnchorMax)
					.bShowSelectedPosBound(bShowSelectedBound)
				]
			]
		]
	];
}

TSharedRef<SWidget> SUIAnchorPresetPanel::MakeVerticalEdgeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax,EHorizontalAnchorType InHorizontalAnchorType,
	EVerticalAnchorType InVerticalAnchorType,TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda,TAttribute<EVerticalAnchorType> SelectVerticalLambda, FText TextLabel, TAttribute<bool> bShowSelectedBound)
{
	return SNew(SBox)
	.WidthOverride(60)
	.HeightOverride(42)
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SConstraintCanvas)
		+ SConstraintCanvas::Slot()
		.Offset(FMargin(16, 7 + 42, 0, 0))
		.AutoSize(true)
		[
			SNew(SBox)
			.WidthOverride(42)
			.HeightOverride(18)
			[
				SNew(STextBlock)
				.RenderTransform(FSlateRenderTransform(FQuat2D(-PI / 2)))
				.Justification(ETextJustify::Center)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(TextLabel)
			]
		]
		+ SConstraintCanvas::Slot()
		.Offset(FMargin(9, 0, 0, 0))
		.Alignment(FVector2D(0, 0))
		.AutoSize(true)
		[
			SNew(SBox)
			.WidthOverride(42)
			.HeightOverride(42)
			[
				SNew(SBorder)
				.Padding(0)
				.BorderImage(FEditorStyle::GetNoBrush())
				.OnMouseButtonDown(this, &SUIAnchorPresetPanel::OnEdgeClick, InAnchorMin, InAnchorMax, FAnchorType(InHorizontalAnchorType, InVerticalAnchorType))
				[
					SNew(SUIAnchorPreset)
					.CurrentHorizontalAnchorType(SelectHorizontalLambda)
					.CurrentVerticalAnchorType(SelectVerticalLambda)
					.bShowCenterPosBound(InShowPositionLambda)
					.AnchorMin(InAnchorMin)
					.AnchorMax(InAnchorMax)
					.bShowSelectedPosBound(bShowSelectedBound)
				]
			]
		]
	];
}

TSharedRef<SWidget> SUIAnchorPresetPanel::MakeHorizontalEdgeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax, EHorizontalAnchorType InHorizontalAnchorType,
	EVerticalAnchorType InVerticalAnchorType, TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda, TAttribute<EVerticalAnchorType> SelectVerticalLambda, FText TextLabel, TAttribute<bool> bShowSelectedBound)
{
	return SNew(SBox)
		.WidthOverride(42)
		.HeightOverride(60)
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Fill)
		[
			SNew(SConstraintCanvas)
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(21.5, 6, 0, 0))
			.AutoSize(true)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(TextLabel)
			]
			+ SConstraintCanvas::Slot()
			.Offset(FMargin(0, 14, 0, 0))
			.Alignment(FVector2D(0, 0))
			.AutoSize(true)
			[
				SNew(SBox)
				.WidthOverride(42)
				.HeightOverride(42)
				[
					SNew(SBorder)
					.Padding(0)
					.BorderImage(FEditorStyle::GetNoBrush())
					.OnMouseButtonDown(this, &SUIAnchorPresetPanel::OnEdgeClick, InAnchorMin, InAnchorMax, FAnchorType(InHorizontalAnchorType, InVerticalAnchorType))
					[
						SNew(SUIAnchorPreset)
						.CurrentHorizontalAnchorType(SelectHorizontalLambda)
						.CurrentVerticalAnchorType(SelectVerticalLambda)
						.bShowCenterPosBound(InShowPositionLambda)
						.AnchorMin(InAnchorMin)
						.AnchorMax(InAnchorMax)
						.bShowSelectedPosBound(bShowSelectedBound)
					]
				]
			]
		];
}

#undef LOCTEXT_NAMESPACE
