#include "DetailCustomizations/GridLayoutGroupComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Core/Layout/GridLayoutGroupComponent.h"
#include "Core/Layout/LayoutGroupComponent.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FGridLayoutGroupComponentDetails::MakeInstance()
{
	return MakeShareable(new FGridLayoutGroupComponentDetails);
}

void FGridLayoutGroupComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Layout"));

	IDetailCategoryBuilder& GridLayoutGroupCategory = DetailBuilder.EditCategory(TEXT("Grid Layout Group"));

	AddRowHeaderContent(GridLayoutGroupCategory, DetailBuilder);
	
	GridLayoutGroupCategory.AddProperty(TEXT("Padding"), ULayoutGroupComponent::StaticClass());
	GridLayoutGroupCategory.AddProperty(TEXT("CellSize"));
	GridLayoutGroupCategory.AddProperty(TEXT("Spacing"));
	
	GridLayoutGroupCategory.AddProperty(TEXT("StartCorner"));
	GridLayoutGroupCategory.AddProperty(TEXT("StartAxis"));
	GridLayoutGroupCategory.AddProperty(TEXT("ChildAlignment"), ULayoutGroupComponent::StaticClass());
	GridLayoutGroupCategory.AddProperty(TEXT("Constraint"));

	DetailBuilder.GetProperty(TEXT("Constraint"))
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UGridLayoutGroupComponent>(TargetObjects[0].Get());

	if (TargetScriptPtr.IsValid())
	{
		if (TargetScriptPtr->GetConstraint() != EGridLayoutConstraint::GridLayoutConstraint_Flexible)
		{
			//GridLayoutGroupCategory.AddProperty(TEXT("ConstraintCount"));
			
			const auto ConstraintCountProperty = DetailBuilder.GetProperty(TEXT("ConstraintCount"));
			GridLayoutGroupCategory.AddCustomRow(LOCTEXT("GridLayoutGroupRow", "ConstraintCount"))
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
						ConstraintCountProperty->CreatePropertyValueWidget()
					]
				];
		}
	}
}

#undef LOCTEXT_NAMESPACE
