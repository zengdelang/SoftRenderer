#include "DetailCustomizations/SubComponentsDetailRow/IComponentDetailCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "K2Node_ComponentBoundEvent.h"
#include "Core/BehaviourComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/SComponentDetailCategory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Styling/SlateIconFinder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "BlueprintDetailsCustomization"

void IComponentDetailCustomization::AddRowHeaderContent(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder)
{
	if (!BehaviourComponent.IsValid())
	{
		TArray<TWeakObjectPtr<UObject>> TargetObjects;
		DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
		BehaviourComponent = Cast<UBehaviourComponent>(TargetObjects[0].Get());
	}
	
	const TSharedRef<SComponentDetailCategory> DetailCategory = SNew(SComponentDetailCategory, BehaviourComponent.Get());
	DetailCategory->ComponentIcon = GetIconBrush();
	CategoryBuilder.HeaderContent(DetailCategory);
}

void IComponentDetailCustomization::AddEventProperty(IDetailCategoryBuilder& CategoryBuilder, IDetailLayoutBuilder& DetailBuilder, FName PropertyName)
{
	if (!BehaviourComponent.IsValid())
	{
		TArray<TWeakObjectPtr<UObject>> TargetObjects;
		DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
		BehaviourComponent = Cast<UBehaviourComponent>(TargetObjects[0].Get());
	}

	if (!BehaviourComponent->IsTemplate())
		return;

	UClass* PropertyClass = BehaviourComponent->GetClass();
	const UPackage* Package = Cast<UPackage>(BehaviourComponent->GetOutermost());
	if (IsValid(Package))
	{
		for (TObjectIterator<UBlueprint> It; It; ++It)
		{
			UBlueprint*	Blueprint = *It;
			if(Blueprint->GetOuter() == Package )
			{
				BlueprintObj = Blueprint;
				break;
			}
		}
	}

	// Check for Ed Graph vars that can generate events
	if (BlueprintObj.IsValid() && BlueprintObj->AllowsDynamicBinding())
	{
#if WITH_EDITORONLY_DATA
		// If the object property can't be resolved for the property, than we can't use it's events.
		FObjectProperty* VariableProperty = FindFProperty<FObjectProperty>(BlueprintObj->SkeletonGeneratedClass, BehaviourComponent->EditorVariableName);

		if ( FBlueprintEditorUtils::CanClassGenerateEvents(PropertyClass) && VariableProperty )
		{
			FMulticastDelegateProperty* Property = CastField<FMulticastDelegateProperty>(PropertyClass->FindPropertyByName(PropertyName));
			
			static const FName HideInDetailPanelName("HideInDetailPanel");
			// Check for multicast delegates that we can safely assign
			if ( Property && !Property->HasAnyPropertyFlags(CPF_Parm) && Property->HasAllPropertyFlags(CPF_BlueprintAssignable) &&
				!Property->HasMetaData(HideInDetailPanelName))
			{
				FName EventName = Property->GetFName();
				FText EventText = Property->GetDisplayNameText();
				
				CategoryBuilder.AddCustomRow(EventText)
				.NameContent()
				[
					SNew(SHorizontalBox)
					.ToolTipText(Property->GetToolTipText())

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0.0f, 0.0f, 5.0f, 0.0f)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("GraphEditor.Event_16x"))
					]

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(EventText)
					]
				]
				.ValueContent()
				.MinDesiredWidth(150.0f)
				.HAlign(HAlign_Fill)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
					.HAlign(HAlign_Center)
					.OnClicked(this, &IComponentDetailCustomization::HandleAddOrViewEventForVariable, EventName, BehaviourComponent->EditorVariableName, MakeWeakObjectPtr(PropertyClass))
					.ForegroundColor(FSlateColor::UseForeground())
					[
						SNew(SWidgetSwitcher)
						.WidgetIndex(this, &IComponentDetailCustomization::HandleAddOrViewIndexForButton, EventName,  BehaviourComponent->EditorVariableName)

						+ SWidgetSwitcher::Slot()
						[
							SNew(STextBlock)
							.Font(FEditorStyle::GetFontStyle(TEXT("BoldFont")))
							.Text(LOCTEXT("ViewEvent", "View"))
						]

						+ SWidgetSwitcher::Slot()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("Plus"))
						]
					]
				];
			}
		}
#endif
	}
}

FSlateBrush* IComponentDetailCustomization::GetIconBrush()
{
	FSlateBrush* ComponentIcon = const_cast<FSlateBrush*>(FEditorStyle::GetBrush("SCS.NativeComponent"));
	if (const UBehaviourComponent* ComponentTemplate = BehaviourComponent.Get())
	{
		ComponentIcon = const_cast<FSlateBrush*>(FSlateIconFinder::FindIconBrushForClass(ComponentTemplate->GetClass(), TEXT("SCS.Component")));
	}
	return ComponentIcon;
}

FReply IComponentDetailCustomization::HandleAddOrViewEventForVariable(const FName EventName, FName VariableName,
	TWeakObjectPtr<UClass> PropertyClass)
{
	// Find the corresponding variable property in the Blueprint
	FObjectProperty* VariableProperty = FindFProperty<FObjectProperty>(BlueprintObj->SkeletonGeneratedClass, VariableName);

	if ( VariableProperty )
	{
		if ( !FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj.Get(), EventName, VariableProperty->GetFName()) )
		{
			FKismetEditorUtilities::CreateNewBoundEventForClass(PropertyClass.Get(), EventName, BlueprintObj.Get(), VariableProperty);
		}
		else
		{
			const UK2Node_ComponentBoundEvent* ExistingNode = FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj.Get(), EventName, VariableProperty->GetFName());
			if ( ExistingNode )
			{
				FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(ExistingNode);
			}
		}
	}

	return FReply::Handled();
}

int32 IComponentDetailCustomization::HandleAddOrViewIndexForButton(const FName EventName, FName VariableName) const
{
	if ( FKismetEditorUtilities::FindBoundEventForComponent(BlueprintObj.Get(), EventName, VariableName) )
	{
		return 0; // View
	}
	return 1; // Add
}

#undef LOCTEXT_NAMESPACE
