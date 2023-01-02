#include "DetailCustomizations/SubComponentsDetailRow/SAddSubComponent.h"
#include "ClassViewerModule.h"
#include "DetailLayoutBuilder.h"
#include "ClassViewer/Private/UnloadedBlueprintData.h"
#include "Core/BehaviourComponent.h"
#include "Core/BehaviourSubComponent.h"
#include "Misc/MessageDialog.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

class FAddSubComponentClassFilter : public IClassViewerFilter
{
public:
	UClass* PropertyClass;
	UClass* OwnerClass;

	/** Whether or not abstract classes are allowed. */
	bool bAllowAbstract;

	/** Hierarchy of objects that own this property. Used to check against ClassWithin. */
	TSet< const UObject* > OwningObjects;

	TSet<UClass*> DisallowedSubClasses;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		for (const auto& DisallowedClass : DisallowedSubClasses)
		{
			const bool bIsInterface = DisallowedClass && DisallowedClass->HasAnyClassFlags(CLASS_Interface);

			if ((DisallowedClass && InClass->IsChildOf(DisallowedClass)) || (bIsInterface && InClass->ImplementsInterface(DisallowedClass)))
			{
				return false;
			}
		}

		{
			const FString& OnlyForClassesString = InClass->GetMetaData("OnlyForClasses");
			TArray<FString> OnlyForClassNames;
			OnlyForClassesString.ParseIntoArrayWS(OnlyForClassNames, TEXT(","), true);

			if (OnlyForClassNames.Num() > 0)
			{
				bool bIsOnlyForSomeClass = false;
				for (const FString& OnlyForClassName : OnlyForClassNames)
				{
					UClass* OnlyForClass = FindObject<UClass>(ANY_PACKAGE, *OnlyForClassName);
					if (OnlyForClass && OnlyForClass->IsChildOf(UBehaviourComponent::StaticClass()))
					{
						if (OwnerClass->IsChildOf(OnlyForClass))
						{
							bIsOnlyForSomeClass = true;
							break;
						}
					}
				}

				if (!bIsOnlyForSomeClass)
				{
					return false;
				}
			}
		}
		
		const bool bChildOfObjectClass = PropertyClass && InClass->IsChildOf(PropertyClass);

		const bool bMatchesFlags = InClass->HasAnyClassFlags(CLASS_EditInlineNew) &&
			!InClass->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if ((bChildOfObjectClass) && bMatchesFlags)
		{
			// Verify that the Owners of the property satisfy the ClassWithin constraint of the given class.
			// When ClassWithin is null, assume it can be owned by anything.
			return InClass->ClassWithin == nullptr || InFilterFuncs->IfMatchesAll_ObjectsSetIsAClass(OwningObjects, InClass->ClassWithin) != EFilterReturn::Failed;
		}

		return false;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		const bool bChildOfObjectClass = InUnloadedClassData->IsChildOf(PropertyClass);

		const bool bMatchesFlags = InUnloadedClassData->HasAnyClassFlags(CLASS_EditInlineNew) &&
			!InUnloadedClassData->HasAnyClassFlags(CLASS_Hidden | CLASS_HideDropDown | CLASS_Deprecated) &&
			(bAllowAbstract || !InUnloadedClassData->HasAnyClassFlags((CLASS_Abstract)));

		if (bChildOfObjectClass && bMatchesFlags)
		{
			const UClass* ClassWithin = InUnloadedClassData->GetClassWithin();

			// Verify that the Owners of the property satisfy the ClassWithin constraint of the given class.
			// When ClassWithin is null, assume it can be owned by anything.
			return ClassWithin == nullptr || InFilterFuncs->IfMatchesAll_ObjectsSetIsAClass(OwningObjects, ClassWithin) != EFilterReturn::Failed;
		}
		return false;
	}
};

void SAddSubComponent::Construct( const FArguments& InArgs, const TSharedRef< class IPropertyHandle >& InRefreshPropertyHandle,
	const TSharedRef< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, const TAttribute<bool>& InIsParentEnabled)
{
	OwnerObject = InOwnerObject;
	RefreshPropertyHandle = InRefreshPropertyHandle;
	SubComponentsPropertyHandle = InSubComponentsPropertyHandle;
	
	IsParentEnabled = InIsParentEnabled;
	
	SetCanTick(false);
	SetVisibility(EVisibility::SelfHitTestInvisible);

	ChildSlot
	[
		SNew(SHorizontalBox)
		.IsEnabled(IsParentEnabled)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.Padding(0, 8)
		[
			SNew(SBox)
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			.WidthOverride(210)
			.HeightOverride(21)
			[
				SAssignNew(ComboButton, SComboButton)
				.OnGetMenuContent(this, &SAddSubComponent::GenerateClassPicker)
				.ContentPadding(0)
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Fill)
				.HasDownArrow(false)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Dark")
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AddSubComponent", "Add Sub Component"))
					.Font(FEditorStyle::GetFontStyle(TEXT("FlatButton.DefaultTextStyle")))
				]
				.VAlign(EVerticalAlignment::VAlign_Center)			
			]
		]
	];
}

FVector2D SAddSubComponent::ComputeDesiredSize(float InValue) const
{
	auto ParentWidget = GetParentWidget();
	if (ParentWidget.IsValid())
	{
		ParentWidget = ParentWidget->GetParentWidget();
		if (ParentWidget.IsValid())
		{
			SSplitter* ParentSplitter = static_cast<SSplitter*>(ParentWidget.Get());
			if (ParentSplitter)
			{
				ParentSplitter->SetVisibility(EVisibility::SelfHitTestInvisible);

				FChildren* Children = ParentSplitter->GetChildren();
				if (Children && Children->Num() > 0)
				{
					const TSharedPtr<SWidget> Child = Children->GetChildAt(0);
					if (Child.IsValid())
					{
						Child->SetVisibility(EVisibility::Collapsed);
					}
				}
			}

			ParentWidget = ParentWidget->GetParentWidget();
			if (ParentWidget.IsValid())
			{
				ParentWidget->SetVisibility(EVisibility::SelfHitTestInvisible);

				SBorder* ParentBorder = static_cast<SBorder*>(ParentWidget.Get());
				if (ParentBorder)
				{
					ParentBorder->SetPadding(FMargin(0));
					ParentBorder->SetBorderImage(FCoreStyle::Get().GetBrush("NoBorder"));
				}

				ParentWidget = ParentWidget->GetParentWidget();
				if (ParentWidget.IsValid())
				{
					ParentWidget->SetVisibility(EVisibility::SelfHitTestInvisible);
				}
			}
		}
	}
	return SCompoundWidget::ComputeDesiredSize(InValue);
}

TSharedRef<SWidget> SAddSubComponent::GenerateClassPicker()
{			
	FClassViewerInitializationOptions Options;
	Options.bShowBackgroundBorder = false;
	Options.bShowUnloadedBlueprints = true;
	Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::DisplayName;

	const TSharedPtr<FAddSubComponentClassFilter> ClassFilter = MakeShareable(new FAddSubComponentClassFilter());
	Options.ClassFilter = ClassFilter;
	ClassFilter->bAllowAbstract = false;
	
	ClassFilter->PropertyClass = UBehaviourSubComponent::StaticClass();
	Options.bShowNoneOption = false;

	if (OwnerObject.IsValid())
	{
		ClassFilter->OwningObjects.Add(OwnerObject.Get());
	}

	UObject* Object = OwnerObject.Get();
	if (Object)
	{
		TSet<UClass*> DisallowedSubClasses;
		GetDisallowedSubClasses(Object->GetClass(), DisallowedSubClasses);
		ClassFilter->DisallowedSubClasses = DisallowedSubClasses;
		ClassFilter->OwnerClass = Object->GetClass();
	}

	const FOnClassPicked OnPicked(FOnClassPicked::CreateRaw(this, &SAddSubComponent::OnClassPicked));
	return FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked);
}

void SAddSubComponent::OnClassPicked(UClass* InClass) const
{
	if (OwnerObject.IsValid())
	{
		TArray<FString> NewValues;
		bool bIsArchetypeObject = false;
		ComboButton->SetIsOpen(false);
		
		GEditor->BeginTransaction(TEXT("AddSubComponent"), NSLOCTEXT("AddSubComponent", "OnClassPicked", "Set Class"), nullptr);
 
		if (InClass)
		{
			UObject* Object = OwnerObject.Get();

			const auto BehaviourComp = Cast<UBehaviourComponent>(Object);
			if (BehaviourComp)
			{
				UClass* RootDisallowClass = nullptr;
				const auto bDisallowMultipleComponent = IsDisallowMultipleComponent(InClass, RootDisallowClass);
			
				const auto& SubComponents = BehaviourComp->GetAllSubComponents();

				if (bDisallowMultipleComponent)
				{
					for (const auto& SubComponent : SubComponents)
					{
						// InClass == RootDisallowClass
						if (SubComponent && SubComponent->IsA(RootDisallowClass))
						{
							// End the transaction if we called PreChange
							GEditor->EndTransaction();

							if (InClass == RootDisallowClass)
							{
								const FText DialogText = FText::Format(
									LOCTEXT("AddSubDisallowMultipleComponent", "Can't add '{0}' because a '{1}' already added to the BehaviourComponent!"),
									InClass->GetDisplayNameText(),
									InClass->GetDisplayNameText()
								);

								FMessageDialog::Open(EAppMsgType::Ok, DialogText);
							}
							else
							{
								const FText DialogText = FText::Format(
									LOCTEXT("AddSubDisallowMultipleComponent", "Can't add '{0}' because a '{1}' already added to the BehaviourComponent!\nA BehaviourComponent can only contain one '{2}' component."),
									InClass->GetDisplayNameText(),
									SubComponent->GetClass()->GetDisplayNameText(),
									RootDisallowClass->GetDisplayNameText()
								);

								FMessageDialog::Open(EAppMsgType::Ok, DialogText);
							}
							return;
						}
					}
				}

				TArray<UClass*> RequireSubClasses;
				GetRequireSubClasses(InClass, RequireSubClasses);

				RequireSubClasses.Remove(InClass);
				StripInvalidRequireSubClasses(RequireSubClasses, BehaviourComp);

				bIsArchetypeObject = BehaviourComp->HasAnyFlags(RF_ArchetypeObject);
				
				for (int32 Index = RequireSubClasses.Num() - 1; Index >= 0; --Index)
				{
					const auto& SubClass = RequireSubClasses[Index];
					if (SubClass)
					{
						const auto SubObject = CreateObjectByClass(SubClass, Object);

						if (!bIsArchetypeObject)
						{
							BehaviourComp->AddSubComponentForEditor(Cast<UBehaviourSubComponent>(SubObject));
						}
						else if (SubObject)
						{			
							NewValues.Add(SubObject->GetPathName());
						}
					}
				}

				const auto NewUObject = CreateObjectByClass(InClass, Object);
				if (!bIsArchetypeObject)
				{
					BehaviourComp->AddSubComponentForEditor(Cast<UBehaviourSubComponent>(NewUObject));
				}
				else if (NewUObject)
				{
					NewValues.Add(NewUObject->GetPathName());
				}
			}	
		}

		if (bIsArchetypeObject)
		{
			const auto ArrayHandle = SubComponentsPropertyHandle->AsArray();
			if (ArrayHandle.IsValid())
			{
				for (const auto NewValue : NewValues)
				{
					ArrayHandle->AddItem();
					
					uint32 NumElements = 0;
					ArrayHandle->GetNumElements(NumElements);

					TArray<FString> ItemValues;
					ItemValues.Add(NewValue);
					const auto ItemHandle = ArrayHandle->GetElement(NumElements - 1);
					ItemHandle->SetPerObjectValues(ItemValues);
				}
			}
		}
		
		bool bValue = false;
		RefreshPropertyHandle->GetValue(bValue);
		RefreshPropertyHandle->SetValue(!bValue);

		// End the transaction if we called PreChange
		GEditor->EndTransaction();
	}
}

bool SAddSubComponent::IsDisallowMultipleComponent(UClass* InClass, UClass*& RootDisallowClass)
{
	bool bIsDisallow = false;
	while (InClass)
	{
		const auto bDisallowMultipleComponent = InClass->HasMetaData(TEXT("DisallowMultipleComponent"));
		if (bDisallowMultipleComponent)
		{
			RootDisallowClass = InClass;
			bIsDisallow = bDisallowMultipleComponent;
		}
		InClass = InClass->GetSuperClass();
	}	
	return bIsDisallow;
}

UObject* SAddSubComponent::CreateObjectByClass(const UClass* InClass, UObject* Object) const
{
	UObject* UseOuter = (InClass->IsChildOf(UClass::StaticClass()) ? Cast<UClass>(Object)->GetDefaultObject() : Object);
	EObjectFlags	MaskedOuterFlags = UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : RF_NoFlags;
	if (UseOuter && UseOuter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		MaskedOuterFlags |= RF_ArchetypeObject;
	}
	
	UObject* NewUObject = NewObject<UObject>(UseOuter, InClass, NAME_None, MaskedOuterFlags, nullptr);
	return NewUObject;
}

void SAddSubComponent::GetDisallowedSubClasses(UClass* InClass, TSet<UClass*>& DisallowedSubClasses) const
{
	while (InClass)
	{
		const FString& DisallowedClassesString = InClass->GetMetaData("DisallowedSubClasses");
		TArray<FString> DisallowedClassNames;
		DisallowedClassesString.ParseIntoArrayWS(DisallowedClassNames, TEXT(","), true);

		for (const FString& DisallowedClassName : DisallowedClassNames)
		{
			UClass* DisallowedClass = FindObject<UClass>(ANY_PACKAGE, *DisallowedClassName);
			if (DisallowedClass)
			{
				DisallowedSubClasses.Add(DisallowedClass);
			}
		}

		InClass = InClass->GetSuperClass();
	}
}

void SAddSubComponent::GetRequireSubClasses(UClass* InClass, TArray<UClass*>& RequireSubClasses) const
{
	while (InClass)
	{
		const FString& RequireClassesString = InClass->GetMetaData("RequireSubClasses");
		TArray<FString> RequireClassNames;
		RequireClassesString.ParseIntoArrayWS(RequireClassNames, TEXT(","), true);

		for (const FString& RequireClassName : RequireClassNames)
		{
			UClass* RequireClass = FindObject<UClass>(ANY_PACKAGE, *RequireClassName);
			if (RequireClass && RequireClass->IsChildOf(UBehaviourSubComponent::StaticClass()))
			{
				if (!RequireSubClasses.Contains(RequireClass))
				{
					RequireSubClasses.AddUnique(RequireClass);
					GetRequireSubClasses(RequireClass, RequireSubClasses);
				}
			}
		}

		InClass = InClass->GetSuperClass();
	}
}

void SAddSubComponent::StripInvalidRequireSubClasses(TArray<UClass*>& RequireSubClasses, UBehaviourComponent* Component)
{
	if (Component)
	{
		for (const auto& SubComp : Component->GetAllSubComponents())
		{
			if (SubComp)
			{
				RequireSubClasses.Remove(SubComp->GetClass());
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
