#include "DetailCustomizations/VerticalLayoutGroupComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/VerticalLayoutGroupComponent.h"
#include "Core/Layout/LayoutGroupComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FVerticalLayoutGroupComponentDetails::MakeInstance()
{
	return MakeShareable(new FVerticalLayoutGroupComponentDetails);
}

void FVerticalLayoutGroupComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UVerticalLayoutGroupComponent>(TargetObjects[0].Get());

	if (TargetObjects[0].Get()->GetClass() != UVerticalLayoutGroupComponent::StaticClass())
	{
		return;
	}
	
	DetailBuilder.HideCategory(TEXT("Layout"));

	IDetailCategoryBuilder& VerticalLayoutGroupCategory = DetailBuilder.EditCategory(TEXT("Vertical Layout Group"));

	AddRowHeaderContent(VerticalLayoutGroupCategory, DetailBuilder);
	
	VerticalLayoutGroupCategory.AddProperty(TEXT("Padding"), ULayoutGroupComponent::StaticClass());
	VerticalLayoutGroupCategory.AddProperty(TEXT("Spacing"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
	VerticalLayoutGroupCategory.AddProperty(TEXT("ChildAlignment"), ULayoutGroupComponent::StaticClass());

	if (TargetScriptPtr.IsValid())
	{
		/// Control Child Size
		const auto ChildControlWidthProperty = DetailBuilder.GetProperty(TEXT("bChildControlWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildControlHeightProperty = DetailBuilder.GetProperty(TEXT("bChildControlHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		VerticalLayoutGroupCategory.AddCustomRow(LOCTEXT("VerticalLayoutGroupControlRow", "ControlChildSizeRow"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VerticalLayoutGroupControlTitle", "Control Child Size"))
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
						.Text(LOCTEXT("VerticalChildControlWidth", "Width"))
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
						.Text(LOCTEXT("VerticalChildControlHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];

		/// Use Child Scale
		const auto ChildScaleWidthProperty = DetailBuilder.GetProperty(TEXT("bChildScaleWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildScaleHeightProperty = DetailBuilder.GetProperty(TEXT("bChildScaleHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		VerticalLayoutGroupCategory.AddCustomRow(LOCTEXT("VerticalLayoutGroupScaleRow", "UseChildScaleRow"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VerticalLayoutScaleTitle", "Use Child Scale"))
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
						.Text(LOCTEXT("VerticalChildScaleWidth", "Width"))
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
						.Text(LOCTEXT("VerticalChildScaleHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];

		/// Child Force Expand
		const auto ChildForceExpandWidthProperty = DetailBuilder.GetProperty(TEXT("bChildForceExpandWidth"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		const auto ChildForceExpandHeightProperty = DetailBuilder.GetProperty(TEXT("bChildForceExpandHeight"), UHorizontalOrVerticalLayoutGroupComponent::StaticClass());
		VerticalLayoutGroupCategory.AddCustomRow(LOCTEXT("VerticalLayoutGroupExpandRow", "ChildForceExpandRow"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("VerticalLayoutExpandTitle", "Child Force Expand"))
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
						.Text(LOCTEXT("VerticalChildExpandWidth", "Width"))
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
						.Text(LOCTEXT("VerticalChildExpandHeight", "Height"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			];
	}
}

#undef LOCTEXT_NAMESPACE
