#include "Core/BehaviourSubComponent.h"
#include "Core/BehaviourComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Layout/RectTransformComponent.h"

/////////////////////////////////////////////////////
// UBehaviourSubComponent

UBehaviourSubComponent::UBehaviourSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), AttachTransform(nullptr)
{
	bIsEnabled = true;

	bHasBeenAwaken = false;
	bHasBeenEnabled = false;
	bHasStarted = false;

	bInteractable = true;
	bCurrentInteractable = true;

	bBlockRaycasts = true;
	bCurrentBlockRaycasts = true;
}

#if WITH_EDITORONLY_DATA

void UBehaviourSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBehaviourSubComponent, bIsEnabled))
	{
		SetEnabled(bIsEnabled);
	}
}

#endif

void UBehaviourSubComponent::InternalAdd()
{
	OnDynamicAdd();
}

void UBehaviourSubComponent::InternalAwake()
{
	if (bHasBeenAwaken)
		return;

	bHasBeenAwaken = true;
	Awake();
}

void UBehaviourSubComponent::InternalRemove()
{
	OnDynamicRemove();
}

void UBehaviourSubComponent::InternalDestroy()
{
	if (!bHasBeenAwaken)
		return;

	bHasBeenAwaken = false;
	OnDestroy();
}

void UBehaviourSubComponent::UpdateEnableState()
{
	if (!bHasBeenAwaken)
		return;
	
	const bool bIsEnabledInHierarchy = IsEnabledInHierarchy();
	if (bIsEnabledInHierarchy != bHasBeenEnabled)
	{
		if (bHasBeenEnabled)
		{
			bHasBeenEnabled = false;
			OnDisable();
		}
		else
		{
			if (!bHasBeenEnabled)
			{
				bHasBeenEnabled = true;
				OnEnable();
			}

			if (!bHasStarted)
			{
				bHasStarted = true;
				Start();
			}
		}
	}
}

void UBehaviourSubComponent::InternalUpdateInteractableState()
{
	if (!bHasBeenAwaken)
		return;

	const bool bInteractableInHierarchy = IsInteractableInHierarchy();
	if (bInteractableInHierarchy != bCurrentInteractable)
	{
		bCurrentInteractable = bInteractableInHierarchy;
		OnInteractableStateChanged();
	}
}

void UBehaviourSubComponent::InternalUpdateBlockRaycastsState()
{
	if (!bHasBeenAwaken)
		return;
	
	const bool bBlocksRaycastsInHierarchy = IsBlockRaycastsInHierarchy();
	if (bBlocksRaycastsInHierarchy != bCurrentBlockRaycasts)
	{
		bCurrentBlockRaycasts = bBlocksRaycastsInHierarchy;
		OnBlocksRaycastsStateChanged();
	}
}

UCanvasSubComponent* UBehaviourSubComponent::GetOwnerCanvas() const
{
	if (IsValid(AttachTransform))
	{
		return AttachTransform->GetOwnerCanvas();
	}
	return nullptr;
}

void UBehaviourSubComponent::SetEnabled(bool bNewEnabled)
{	
	bIsEnabled = bNewEnabled;
	UpdateEnableState();
}

bool UBehaviourSubComponent::IsEnabledInHierarchy() const
{
	if (bIsEnabled)
	{
		const auto Owner = Cast<UBehaviourComponent>(GetOuter());
		if (IsValid(Owner))
		{
			return bIsEnabled && Owner->IsActiveAndEnabled();
		}
	}
	
	return bIsEnabled;
}

bool UBehaviourSubComponent::IsInteractableInHierarchy() const
{
	const auto Owner = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(Owner))
	{
		return bInteractable && Owner->IsInteractableInHierarchy();
	}
	return bInteractable;
}

void UBehaviourSubComponent::SetInteractable(bool bInInteractable)
{
	if (bInteractable != bInInteractable)
	{
		bInteractable = bInInteractable;
		InternalUpdateInteractableState();
	}
}

bool UBehaviourSubComponent::IsBlockRaycastsInHierarchy() const
{
	const auto Owner = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(Owner))
	{
		return bBlockRaycasts && Owner->IsBlockRaycastsInHierarchy();
	}
	return bBlockRaycasts;
}

void UBehaviourSubComponent::SetBlockRaycasts(bool bInBlockRaycasts)
{
	if (bBlockRaycasts != bInBlockRaycasts)
	{
		bBlockRaycasts = bInBlockRaycasts;
		InternalUpdateBlockRaycastsState();
	}
}

bool UBehaviourSubComponent::IsIgnoreReversedGraphicsInHierarchy() const
{
	const auto Owner = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(Owner))
	{
		return Owner->IsIgnoreReversedGraphicsInHierarchy();
	}
	return false;
}

void UBehaviourSubComponent::Awake()
{
	AttachTransform = Cast<URectTransformComponent>(GetOuter());
}

void UBehaviourSubComponent::InternalOnZOrderChanged()
{
	if (!HasBeenAwaken())
		return;

	OnZOrderChanged();
}

UObject* UBehaviourSubComponent::GetComponent(UClass* InClass, bool bIncludeInactive) const
{
	const auto SceneComp = Cast<USceneComponent>(GetOuter());
	if (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					return BehaviourComp;
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							return SubComponent;
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			return SceneComp;
		}
	}

	return nullptr;
}

UObject* UBehaviourSubComponent::GetComponentInChildren(UClass* InClass, bool bIncludeInactive) const
{
	return InternalGetComponentInChildren(Cast<USceneComponent>(GetOuter()), InClass, bIncludeInactive);
}

UObject* UBehaviourSubComponent::InternalGetComponentInChildren(USceneComponent* SceneComp, UClass* InClass, bool bIncludeInactive) const
{
	if (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					return BehaviourComp;
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							return SubComponent;
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			return SceneComp;
		}

		for (const auto& ChildComp : SceneComp->GetAttachChildren())
		{
			const auto ChildObj = InternalGetComponentInChildren(ChildComp, InClass, bIncludeInactive);
			if (IsValid(ChildObj))
			{
				return ChildObj;
			}
		}
	}

	return nullptr;
}

UObject* UBehaviourSubComponent::GetComponentInParent(UClass* InClass, bool bIncludeInactive) const
{
	auto SceneComp = Cast<USceneComponent>(GetOuter());
	while (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					return BehaviourComp;
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							return SubComponent;
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			return SceneComp;
		}

		SceneComp = SceneComp->GetAttachParent();
	}

	return nullptr;
}

void UBehaviourSubComponent::GetComponents(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive) const
{
	Components.Reset();

	const auto SceneComp = Cast<USceneComponent>(GetOuter());
	if (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					Components.Add(BehaviourComp);
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							Components.Add(SubComponent);
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			Components.Add(SceneComp);
		}
	}
}

void UBehaviourSubComponent::GetComponentsInChildren(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive) const
{
	Components.Reset();

	InternalGetComponentsInChildren(Cast<USceneComponent>(GetOuter()), Components, InClass, bIncludeInactive);
}

void UBehaviourSubComponent::InternalGetComponentsInChildren(USceneComponent* SceneComp, TArray<UObject*>& Components,
	UClass* InClass, bool bIncludeInactive) const
{
	if (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					Components.Add(BehaviourComp);
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							Components.Add(SubComponent);
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			Components.Add(SceneComp);
		}

		for (const auto& ChildComp : SceneComp->GetAttachChildren())
		{
			const auto ChildObj = InternalGetComponentInChildren(ChildComp, InClass, bIncludeInactive);
			if (IsValid(ChildObj))
			{
				Components.Add(ChildObj);
			}
		}
	}
}

void UBehaviourSubComponent::GetComponentsInParent(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive) const
{
	Components.Reset();

	auto SceneComp = Cast<USceneComponent>(GetOuter());
	while (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->IsA(InClass))
				{
					Components.Add(BehaviourComp);
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->IsA(InClass))
						{
							Components.Add(SubComponent);
						}
					}
				}
			}
		}
		else if (SceneComp->IsA(InClass))
		{
			Components.Add(SceneComp);
		}

		SceneComp = SceneComp->GetAttachParent();
	}
}

/////////////////////////////////////////////////////
