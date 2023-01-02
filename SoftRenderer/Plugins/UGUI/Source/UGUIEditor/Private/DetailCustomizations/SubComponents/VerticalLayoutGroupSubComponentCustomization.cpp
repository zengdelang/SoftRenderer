#include "DetailCustomizations/SubComponents/VerticalLayoutGroupSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/VerticalLayoutGroupSubComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FVerticalLayoutGroupSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FVerticalLayoutGroupSubComponentCustomization);
}

void FVerticalLayoutGroupSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	TargetScriptPtr = Cast<UVerticalLayoutGroupSubComponent>(SubComponent.Get());
	if (TargetScriptPtr.Get()->GetClass() != UVerticalLayoutGroupSubComponent::StaticClass())
	{
		return;
	}
	
	AddProperty(TEXT("Padding"), CategoryBuilder);
	AddProperty(TEXT("Spacing"), CategoryBuilder);
	AddProperty(TEXT("ChildAlignment"), CategoryBuilder);

	if (TargetScriptPtr.IsValid())
	{
		/// Control Child Size
		const auto ChildControlWidthProperty = AddProperty(TEXT("bChildControlWidth"), CategoryBuilder);
		ChildControlWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildControlHeightProperty = AddProperty(TEXT("bChildControlHeight"), CategoryBuilder);
		ChildControlHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("VerticalLayoutGroupControlRow", "ControlChildSizeRow"))
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
						ChildControlWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildControlHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
		const auto ChildScaleWidthProperty = AddProperty(TEXT("bChildScaleWidth"), CategoryBuilder);
		ChildScaleWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildScaleHeightProperty = AddProperty(TEXT("bChildScaleHeight"), CategoryBuilder);
		ChildScaleHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("VerticalLayoutGroupScaleRow", "UseChildScaleRow"))
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
						ChildScaleWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildScaleHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
		const auto ChildForceExpandWidthProperty = AddProperty(TEXT("bChildForceExpandWidth"), CategoryBuilder);
		ChildForceExpandWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildForceExpandHeightProperty = AddProperty(TEXT("bChildForceExpandHeight"), CategoryBuilder);
		ChildForceExpandHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("VerticalLayoutGroupExpandRow", "ChildForceExpandRow"))
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
						ChildForceExpandWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildForceExpandHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
