#include "DetailCustomizations/SubComponents/GridLayoutGroupSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/GridLayoutGroupSubComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FGridLayoutGroupSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FGridLayoutGroupSubComponentCustomization);
}

void FGridLayoutGroupSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("Padding"), CategoryBuilder);
	AddProperty(TEXT("CellSize"), CategoryBuilder);
	AddProperty(TEXT("Spacing"), CategoryBuilder);
	
	AddProperty(TEXT("StartCorner"), CategoryBuilder);
	AddProperty(TEXT("StartAxis"), CategoryBuilder);
	AddProperty(TEXT("ChildAlignment"), CategoryBuilder);
	AddProperty(TEXT("Constraint"), CategoryBuilder)
		->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TargetScriptPtr = Cast<UGridLayoutGroupSubComponent>(SubComponent.Get());
	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->GetConstraint() != EGridLayoutConstraint::GridLayoutConstraint_Flexible)
		{
			//GridLayoutGroupCategory.AddProperty(TEXT("ConstraintCount"));
			
			const auto ConstraintCountProperty = AddProperty(TEXT("ConstraintCount"), CategoryBuilder);
			ConstraintCountProperty->Visibility(EVisibility::Collapsed);
			CategoryBuilder.AddCustomRow(LOCTEXT("GridLayoutGroupRow", "ConstraintCount"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("GridLayoutGroupTitle", "\tConstraint Count"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						ConstraintCountProperty->GetPropertyHandle()->CreatePropertyValueWidget()
					]
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
