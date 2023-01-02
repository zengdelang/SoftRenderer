#include "UIBlueprintFactory.h"
#include "UGUISettings.h"
#include "Core/UIBlueprint.h"
#include "Core/WidgetActor.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "BlueprintFactory"

UUIBlueprintFactory::UUIBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UUIBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UUIBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	// Make sure we are trying to factory a UI Blueprint, then create and init one
	check(Class->IsChildOf(UUIBlueprint::StaticClass()));

	UClass* CurrentParentClass = AWidgetActor::StaticClass();

	if ((CurrentParentClass == nullptr) || !FKismetEditorUtilities::CanCreateBlueprintOfClass(CurrentParentClass) || !CurrentParentClass->IsChildOf(AWidgetActor::StaticClass()))
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("ClassName"), CurrentParentClass ? FText::FromString(CurrentParentClass->GetName()) : LOCTEXT("Null", "(null)"));
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("CannotCreateUIBlueprint", "Cannot create a UI Blueprint based on the class '{ClassName}'."), Args));
		return nullptr;
	}
	else
	{
		UUIBlueprint* NewBP = CastChecked<UUIBlueprint>(FKismetEditorUtilities::CreateBlueprint(CurrentParentClass, InParent, Name, EBlueprintType::BPTYPE_Normal, 
			UUIBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), CallingContext));
		
		if (NewBP)
		{
			// Remove the current scene root node from the SCS context
			NewBP->SimpleConstructionScript->RemoveNode(NewBP->SimpleConstructionScript->GetDefaultSceneRootNode(), /*bValidateSceneRootNodes=*/false);

			const auto UIRootNode = NewBP->SimpleConstructionScript->CreateNode(URectTransformComponent::StaticClass(), TEXT("UIRoot"));

			if (URectTransformComponent* UIRootComp = Cast<URectTransformComponent>(UIRootNode->ComponentTemplate))
			{
				UIRootComp->Modify();

				UIRootComp->SetSizeDelta(UUGUISettings::Get()->UIRootSizeDelta);

				{
					UObject* UseOuter = UIRootComp;
					EObjectFlags	MaskedOuterFlags = UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : RF_NoFlags;
					if (UseOuter && UseOuter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
					{
						MaskedOuterFlags |= RF_ArchetypeObject;
					}
	
					UBehaviourSubComponent* NewUObject = NewObject<UBehaviourSubComponent>(UseOuter, UCanvasSubComponent::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);

					UIRootComp->AddSubComponentForEditor(NewUObject);
				}
				
				UIRootComp->MarkPackageDirty();
			}
			
			// Add dropped node to the SCS context
			NewBP->SimpleConstructionScript->AddNode(UIRootNode);			
		}
	
		return NewBP;
	}
}

UObject* UUIBlueprintFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
