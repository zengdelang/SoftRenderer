#include "DetailCustomizations/SubComponents/HorizontalLayoutGroupSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/HorizontalLayoutGroupSubComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FHorizontalLayoutGroupSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FHorizontalLayoutGroupSubComponentCustomization);
}

void FHorizontalLayoutGroupSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("Padding"), CategoryBuilder);
	AddProperty(TEXT("Spacing"), CategoryBuilder);
	AddProperty(TEXT("ChildAlignment"), CategoryBuilder);
	
	TargetScriptPtr = Cast<UHorizontalLayoutGroupSubComponent>(SubComponent.Get());
	if (TargetScriptPtr.IsValid())
	{
		/// Control Child Size
		const auto ChildControlWidthProperty = AddProperty(TEXT("bChildControlWidth"), CategoryBuilder);
		ChildControlWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildControlHeightProperty = AddProperty(TEXT("bChildControlHeight"), CategoryBuilder);
		ChildControlHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("HorizontalLayoutGroupControlRow", "ControlChildSize"))
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
						ChildControlWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildControlHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
		const auto ChildScaleWidthProperty = AddProperty(TEXT("bChildScaleWidth"), CategoryBuilder);
		ChildScaleWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildScaleHeightProperty = AddProperty(TEXT("bChildScaleHeight"), CategoryBuilder);
		ChildScaleHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("HorizontalLayoutGroupScaleRow", "UseChildScaleRow"))
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
						ChildScaleWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildScaleHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
		const auto ChildForceExpandWidthProperty = AddProperty(TEXT("bChildForceExpandWidth"), CategoryBuilder);
		ChildForceExpandWidthProperty->Visibility(EVisibility::Collapsed);
		const auto ChildForceExpandHeightProperty = AddProperty(TEXT("bChildForceExpandHeight"), CategoryBuilder);
		ChildForceExpandHeightProperty->Visibility(EVisibility::Collapsed);
		CategoryBuilder.AddCustomRow(LOCTEXT("HorizontalLayoutGroupExpandRow", "ChildForceExpand"))
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
						ChildForceExpandWidthProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
						ChildForceExpandHeightProperty->GetPropertyHandle()->CreatePropertyValueWidget()
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
