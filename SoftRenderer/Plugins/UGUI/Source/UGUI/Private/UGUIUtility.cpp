#include "UGUIUtility.h"
#include "EventSystem/UGUIGameViewportClient.h"

URectTransformComponent* UUGUIUtility::AddUIComponent(TSubclassOf<URectTransformComponent> ComponentClass,
                                                  URectTransformComponent* ParentComponent)
{
	if (ComponentClass == nullptr || !IsValid(ParentComponent))
	{
		return nullptr;
	}

	const auto Owner = ParentComponent->GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}
	
	if (ComponentClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}
	
	URectTransformComponent* NewUIComponent = NewObject<URectTransformComponent>(Owner, ComponentClass, NAME_None, RF_Transient);
	if (IsValid(NewUIComponent))
	{
		NewUIComponent->RegisterComponent();
		NewUIComponent->AttachToComponent(ParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
		NewUIComponent->SetLocalTransform(FTransform::Identity);
		NewUIComponent->AwakeFromLoad();
		NewUIComponent->CreationMethod = EComponentCreationMethod::Instance;
	}
	
	return NewUIComponent;
}
