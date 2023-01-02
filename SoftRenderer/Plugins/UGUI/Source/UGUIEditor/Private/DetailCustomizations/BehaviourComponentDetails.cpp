#include "DetailCustomizations/BehaviourComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UGUIEditor.h"
#include "UGUISettings.h"
#include "Core/BehaviourComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"
#include "DetailCustomizations/SubComponentsDetailRow/SAddSubComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

class FAddSubComponentNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FAddSubComponentNodeBuilder>
{
public:
	FAddSubComponentNodeBuilder() {}
	virtual ~FAddSubComponentNodeBuilder() override {};

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override {  }
	virtual bool RequiresTick() const override { return false; }
	virtual void Tick(float DeltaTime) override {}
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override {}
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override {}
	virtual bool InitiallyCollapsed() const override { return true; };
	virtual FName GetName() const override { return FName(TEXT("AddSubComponent")); }
};

TSharedRef<IDetailCustomization> FBehaviourComponentDetails::MakeInstance()
{
	return MakeShareable(new FBehaviourComponentDetails);
}

void FBehaviourComponentDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	TAttribute<bool> IsParentEnabled(this, &FBehaviourComponentDetails::IsParentEnabled);
	
	InDetailBuilder.HideCategory(TEXT("Sockets"));
	InDetailBuilder.HideCategory(TEXT("SubComponents"));
	InDetailBuilder.HideCategory(TEXT("Variable"));
	InDetailBuilder.HideCategory(TEXT("Behaviour"));
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	InDetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UBehaviourComponent>(TargetObjects[0].Get());

	if (TargetScriptPtr.IsValid() && !TargetScriptPtr->IsTemplate())
	{
		IDetailCategoryBuilder& UIComponentCategory = InDetailBuilder.EditCategory(TEXT("UIComponent"), FText(LOCTEXT("UIComponentCategory", "UI Component")),
	ECategoryPriority::Variable);

		UIComponentCategory.AddProperty(TEXT("bEnabled"));
		UIComponentCategory.AddProperty(TEXT("bInteractable"));
		UIComponentCategory.AddProperty(TEXT("bBlockRaycasts"));
		UIComponentCategory.AddProperty(TEXT("bIgnoreReversedGraphics"));
		UIComponentCategory.AddProperty(TEXT("bGraying"));
		UIComponentCategory.AddProperty(TEXT("bInvertColor"));
		UIComponentCategory.AddProperty(TEXT("bIgnoreParentRenderOpacity"));
		UIComponentCategory.AddProperty(TEXT("bIgnoreParentInteractable"));
		UIComponentCategory.AddProperty(TEXT("bIgnoreParentBlockRaycasts"));
		UIComponentCategory.AddProperty(TEXT("bIgnoreParentReversedGraphics"));
		UIComponentCategory.AddProperty(TEXT("RenderOpacity"));
		UIComponentCategory.AddProperty(TEXT("ZOrder"));
	}

	SubComponentDetailCustomizationArray.Reset();

	if (TargetScriptPtr.IsValid())   
	{
		FUGUIEditorModule& UGUIEditorModule = FModuleManager::LoadModuleChecked<FUGUIEditorModule>("UGUIEditor");
		
		const auto SubComponents = TargetScriptPtr->GetAllSubComponents();
		for (int32 Index = 0, Count = SubComponents.Num(); Index < Count; ++Index)
		{
			UBehaviourSubComponent* SubComponent = SubComponents[Index];
			if (SubComponent)
			{
				const auto CategoryName = GetSubComponentCategoryName(InDetailBuilder, FName(SubComponent->GetClass()->GetDisplayNameText().ToString()));
				
				auto& SubComponentProperty = InDetailBuilder.EditCategory(CategoryName, FText::Format(LOCTEXT("{0}", "{1}"),
					FText::FromString(CategoryName.ToString() + FString(TEXT("SubComponentCategory"))), FText::FromString(CategoryName.ToString())), ECategoryPriority::Uncommon);

				FOnGetSubComponentDetailCustomizationInstance OnGetDetailCustomizationInstance;
				if (UGUIEditorModule.GetSubComponentClassLayout(SubComponent->GetClass()->GetFName(), OnGetDetailCustomizationInstance))
				{
					auto SubComponentDetailCustomization = OnGetDetailCustomizationInstance.Execute();
					SubComponentDetailCustomization.Get().SetSubComponent(SubComponent, TargetScriptPtr.Get(), CategoryName, Index, IsParentEnabled);
					SubComponentDetailCustomization.Get().CustomizeDetails(InDetailBuilder, SubComponentProperty);
					SubComponentDetailCustomizationArray.Add(SubComponentDetailCustomization);
				}
				else
				{
					auto SubComponentDetailCustomization = ISubComponentDetailCustomization::MakeInstance();
					SubComponentDetailCustomization.Get().SetSubComponent(SubComponent, TargetScriptPtr.Get(), CategoryName, Index, IsParentEnabled);
					SubComponentDetailCustomization.Get().CustomizeDetails(InDetailBuilder, SubComponentProperty);
					SubComponentDetailCustomization.Get().AddRowHeaderContent(SubComponentProperty, InDetailBuilder);

					for (FProperty* Property = SubComponent->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext)
					{
						if (!Property->HasAnyPropertyFlags(CPF_Edit))
						{
							continue;
						}

						if (Property->GetFName() == TEXT("bIsEnabled"))
						{
							continue;
						}

						EPropertyLocation::Type Location = EPropertyLocation::Default;
						if (Property->HasAnyPropertyFlags(CPF_AdvancedDisplay))
						{
							Location = EPropertyLocation::Advanced;
						}
						SubComponentDetailCustomization.Get().AddProperty(Property->GetFName(), SubComponentProperty, Location);
					}

					SubComponentDetailCustomizationArray.Add(SubComponentDetailCustomization);
				}
			}
		}
	}
	
	/// Add Sub Component
	const auto SubComponentsPropertyHandle = InDetailBuilder.GetProperty(TEXT("SubComponents"));
	const auto RefreshPropertyHandle = InDetailBuilder.GetProperty(TEXT("bRefreshDetailForEditor"));
	RefreshPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { InDetailBuilder.ForceRefreshDetails(); }));
	IDetailCategoryBuilder& AddSubComponentCategory = InDetailBuilder.EditCategory(TEXT("Add Sub Component"), FText::GetEmpty(), ECategoryPriority::Uncommon);
	AddSubComponentCategory.HeaderContent(SNew(SAddSubComponent, RefreshPropertyHandle, SubComponentsPropertyHandle, TargetScriptPtr.Get(), IsParentEnabled));
	AddSubComponentCategory.AddCustomBuilder(MakeShareable(new FAddSubComponentNodeBuilder()));
}

void FBehaviourComponentDetails::CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder)
{
	DetailBuilder = InDetailBuilder;
	CustomizeDetails(*InDetailBuilder);
}

bool FBehaviourComponentDetails::IsParentEnabled() const
{
	return !DetailBuilder.IsValid() || !DetailBuilder.Pin()->GetDetailsView() || DetailBuilder.Pin()->GetDetailsView()->IsPropertyEditingEnabled();
}

FName FBehaviourComponentDetails::GetSubComponentCategoryName(IDetailLayoutBuilder& InDetailBuilder, FName CategoryName)
{
	TArray<FName> CategoryNames;
	InDetailBuilder.GetCategoryNames(CategoryNames);

	while(CategoryNames.Contains(CategoryName))
	{
		CategoryName = FName(CategoryName.ToString() + FString(" "));
	}

	return CategoryName;
}

#undef LOCTEXT_NAMESPACE
