#include "DetailCustomizations/ChildWidgetActorComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "IDetailGroup.h"
#include "Components/SlateWrapperTypes.h"
#include "Core/Layout/ChildWidgetActorComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FChildWidgetActorComponentDetails::MakeInstance()
{
	return MakeShareable(new FChildWidgetActorComponentDetails);
}

void FChildWidgetActorComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("ChildWidget"));
	
	IDetailCategoryBuilder& ChildWidgetCategory = DetailBuilder.EditCategory(TEXT("Child Widget"), FText(LOCTEXT("ChildWidgetActorComponentCategory", "Child Widget")));
	AddRowHeaderContent(ChildWidgetCategory, DetailBuilder);

	ChildWidgetCategory.AddProperty(TEXT("ChildWidgetActorClass"));
	ChildWidgetCategory.AddProperty(TEXT("bStripCanvasComponent"));
	
	const auto ChildWidgetActorClassProperty = DetailBuilder.GetProperty(TEXT("ChildWidgetActorClass"));
	ChildWidgetActorClassProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UChildWidgetActorComponent>(TargetObjects[0].Get());
	
	if (TargetScriptPtr.IsValid() && IsValid(TargetScriptPtr->GetParametersObject()))   
	{
		IDetailGroup& TemplateParametersGroup = ChildWidgetCategory.AddGroup(TEXT("Template Parameters"), FText(LOCTEXT("ChildWidgetActorComponentCategory", "Template Parameters")));

		TMap<FString, IDetailGroup*> PropertyGroupMap;
		for (FProperty* Property = TargetScriptPtr->GetParametersObject()->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
		{
			if (!Property->HasAnyPropertyFlags(CPF_Edit))
			{
				continue;
			}
			
			EPropertyLocation::Type Location = EPropertyLocation::Default;
			if (Property->HasAnyPropertyFlags(CPF_AdvancedDisplay))
			{
				Location = EPropertyLocation::Advanced;
			}

			const FString PropertyGroupName = Property->GetMetaData(TEXT("Category"));
			const auto PropertyGroupPtr = PropertyGroupMap.Find(PropertyGroupName);
			IDetailGroup* PropertyDetailGroup = nullptr; 
			if (!PropertyGroupPtr)
			{
				PropertyDetailGroup = &TemplateParametersGroup.AddGroup(FName(PropertyGroupName), FText::FromString(PropertyGroupName));
				PropertyGroupMap.Add(PropertyGroupName, PropertyDetailGroup);
			}
			else
			{
				PropertyDetailGroup = *PropertyGroupPtr;
			}
			
			{
				TArray<UObject*> Objects;
				Objects.Add(TargetScriptPtr->GetParametersObject());
				IDetailPropertyRow* PropertyRow = ChildWidgetCategory.AddExternalObjectProperty(Objects, Property->GetFName(), Location);
				PropertyRow->Visibility(EVisibility::Collapsed);
				
				PropertyDetailGroup->AddPropertyRow(PropertyRow->GetPropertyHandle().ToSharedRef());
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
