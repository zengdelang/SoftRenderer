#include "DetailCustomizations/HorizontalLayoutGroupComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/HorizontalLayoutGroupComponent.h"
#include "Core/Layout/HorizontalOrVerticalLayoutGroupComponent.h"
#include "Core/Layout/LayoutGroupComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FHorizontalLayoutGroupComponentDetails::MakeInstance()
{
	return MakeShareable(new FHorizontalLayoutGroupComponentDetails);
}

void FHorizontalLayoutGroupComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Layout"));

	IDetailCategoryBuilder& HorizontalLayoutGroupCategory = DetailBuilder.EditCategory(TEXT("Horizontal Layout Group"));

	AddRowHeaderContent(HorizontalLayoutGroupCategory, DetailBuilder);
	
	HorizontalLayoutGroupCategory.AddProperty(TEXT("Padding"), ULayoutGroupComponent::StaticClass());
	HorizontalLayoutGroupCategory.AddProperty(TEXT("Spacing"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
	HorizontalLayoutGroupCategory.AddProperty(TEXT("ChildAlignment"), ULayoutGroupComponent::StaticClass());

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UHorizontalLayoutGroupComponent>(TargetObjects[0].Get());

	if (TargetScriptPtr.IsValid())
	{
		/// Control Child Size
		const auto ChildControlWidthProperty = DetailBuilder.GetProperty(TEXT("bChildControlWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildControlHeightProperty = DetailBuilder.GetProperty(TEXT("bChildControlHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		HorizontalLayoutGroupCategory.AddCustomRow(LOCTEXT("HorizontalLayoutGroupControlRow", "ControlChildSize"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HorizontalLayoutGroupControlTitle", "Control Child Size"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildControlWidthProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildControlWidth", "Width"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildControlHeightProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildControlHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];

		/// Use Child Scale
		const auto ChildScaleWidthProperty = DetailBuilder.GetProperty(TEXT("bChildScaleWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildScaleHeightProperty = DetailBuilder.GetProperty(TEXT("bChildScaleHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		HorizontalLayoutGroupCategory.AddCustomRow(LOCTEXT("HorizontalLayoutGroupScaleRow", "UseChildScaleRow"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HorizontalLayoutScaleTitle", "Use Child Scale"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildScaleWidthProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildScaleWidth", "Width"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildScaleHeightProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildScaleHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];

		/// Child Force Expand
		const auto ChildForceExpandWidthProperty = DetailBuilder.GetProperty(TEXT("bChildForceExpandWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildForceExpandHeightProperty = DetailBuilder.GetProperty(TEXT("bChildForceExpandHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		HorizontalLayoutGroupCategory.AddCustomRow(LOCTEXT("HorizontalLayoutGroupExpandRow", "ChildForceExpand"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("HorizontalLayoutExpandTitle", "Child Force Expand"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildForceExpandWidthProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildExpandWidth", "Width"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ChildForceExpandHeightProperty->CreatePropertyValueWidget()
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(2, 1, 0, 0)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("HorizontalChildExpandHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];
	}
}

#undef LOCTEXT_NAMESPACE
