#include "Core/BehaviourComponent.h"
#include "Core/Widgets/GraphicSubComponent.h"
#include "UGUISettings.h"
#include "Core/WidgetActor.h"

/////////////////////////////////////////////////////
// UBehaviourComponent

UBehaviourComponent::UBehaviourComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	bUseAttachParentBound = true;

	RenderOpacity = 1;
	ZOrder = 0;
	ParentRenderOpacityInHierarchy = 1;
	CurrentRenderOpacity = 1;
	
	OwnerCanvas = nullptr;
	
	bEnabled = true;

	bParentEnabledInHierarchy = false;
	bHasBeenEnabled = false;
	bHasBeenAwaken = false;
	
	bInteractable = true;
	bParentInteractableInHierarchy = true;
	bCurrentInteractable = true;
	
	bBlockRaycasts = true;
	bParentBlockRaycastsInHierarchy = true;
	bCurrentBlockRaycasts = true;
	
	bIgnoreReversedGraphics = false;
	bParentIgnoreReversedGraphicsInHierarchy = false;
	bCurrentIgnoreReversedGraphics = false;
	
	bDisableTransformParentChanged = false;
	bIsRootComponent = false;

#if WITH_EDITORONLY_DATA
	bRefreshDetailForEditor = false;
	bIsVariable = false;
	bIsLockForEditor = false;
	bIsVisibleForEditor = true;
#endif
	
#if WITH_EDITOR
	FProperty* Prop = StaticClass()->FindPropertyByName(TEXT("OnComponentActivated"));
	if (Prop)
	{
		Prop->SetMetaData(TEXT("HideInDetailPanel"), TEXT("true"));
	}

	Prop = StaticClass()->FindPropertyByName(TEXT("OnComponentDeactivated"));
	if (Prop)
	{
		Prop->SetMetaData(TEXT("HideInDetailPanel"), TEXT("true"));
	}

	Prop = StaticClass()->FindPropertyByName(TEXT("PhysicsVolumeChangedDelegate"));
	if (Prop)
	{
		Prop->SetMetaData(TEXT("HideInDetailPanel"), TEXT("true"));
	}
#endif

	bGraying = false;
	bParentGrayingInHierarchy = false;
	bCurrentGraying = false;

	bInvertColor = false;
	bAttachChildChanged = false;

	bIgnoreParentRenderOpacity = false;
	bIgnoreParentInteractable = false;
	bIgnoreParentBlockRaycasts = false;
	bIgnoreParentReversedGraphics = false;
	
	bParentInvertColorInHierarchy = false;
	bCurrentInvertColor = false;
}

void UBehaviourComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (bHasBeenEnabled)
	{
		bHasBeenEnabled = false;
		
		TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
		GetSubComponents(Components);

		for (UBehaviourSubComponent* SubComponent : Components)
		{
			SubComponent->UpdateEnableState();
		}
	
		OnDisable();

		if (bIsRootComponent)
		{
			if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
			{
				WidgetActor->OnActorDisable();
			}
		}
	}
	
	InternalDestroy();
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UBehaviourComponent::AwakeFromLoad()
{
	if (!bHasBeenAwaken)
	{
		bHasBeenAwaken = true;
		InternalAwake();
	}

	const UBehaviourComponent* ParentBehaviourComp = Cast<UBehaviourComponent>(GetAttachParent());
	
	UpdateParentRenderOpacityInHierarchy(ParentBehaviourComp);
	const float ParentRenderOpacity = GetRenderOpacityInHierarchy();
	const bool bUpdateRenderOpacity = !FMath::IsNearlyEqual(ParentRenderOpacity, CurrentRenderOpacity);
	if (bUpdateRenderOpacity)
	{
		CurrentRenderOpacity = ParentRenderOpacity;
	}

	UpdateParentInvertColorInHierarchy(ParentBehaviourComp);
	const bool bIsInvertColorInHierarchy = IsInvertColorInHierarchy();
	const bool bUpdateInvertColor = bIsInvertColorInHierarchy != bCurrentInvertColor;
	if (bUpdateInvertColor)
	{
		bCurrentInvertColor = bIsInvertColorInHierarchy;
	}
	
	UpdateParentGrayingInHierarchy(ParentBehaviourComp);
	const bool bIsGrayingInHierarchy = IsGrayingInHierarchy();
	const bool bUpdateGraying = bIsGrayingInHierarchy != bCurrentGraying;
	if (bUpdateGraying)
	{
		bCurrentGraying = bIsGrayingInHierarchy;
	}

	UpdateParentInteractableInHierarchy(ParentBehaviourComp);
	const bool bIsInteractableInHierarchy = IsInteractableInHierarchy();
	const bool bUpdateInteractable = bIsInteractableInHierarchy != bCurrentInteractable;
	if (bUpdateInteractable)
	{
		bCurrentInteractable = bIsInteractableInHierarchy;
	}

	UpdateParentBlockRaycastsInHierarchy(ParentBehaviourComp);
	const bool bIsBlockRaycastsInHierarchy = IsBlockRaycastsInHierarchy();
	const bool bUpdateBlockRaycasts = bIsBlockRaycastsInHierarchy != bCurrentBlockRaycasts;
	if (bUpdateBlockRaycasts)
	{
		bCurrentBlockRaycasts = bIsBlockRaycastsInHierarchy;
	}
	
	UpdateParentIgnoreReversedGraphicsInHierarchy(ParentBehaviourComp);
	bCurrentIgnoreReversedGraphics = IsIgnoreReversedGraphicsInHierarchy();
	
	bool bInvokeOnDisable = false;
	bool bInvokeOnEnable = false;
	UpdateParentEnabledInHierarchy(ParentBehaviourComp);
	const bool bIsEnabledInHierarchy = IsEnabledInHierarchy();
	const bool bUpdateEnableState = bIsEnabledInHierarchy != bHasBeenEnabled;
	if (bUpdateEnableState)
	{
		if (bHasBeenEnabled)
		{
			bHasBeenEnabled = false;
			bInvokeOnDisable = true;
		}
		else
		{
			if (!bHasBeenEnabled)
			{
				bHasBeenEnabled = true;
				bInvokeOnEnable = true;
			}
		}
	}

	if (bUpdateEnableState || bUpdateRenderOpacity || bUpdateInvertColor || bUpdateGraying)
	{
		TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
		GetSubComponents(Components);

		for (UBehaviourSubComponent* SubComponent : Components)
		{
			if (UGraphicSubComponent* GraphicSubComp = Cast<UGraphicSubComponent>(SubComponent))
			{
				if (bUpdateRenderOpacity)
				{
					GraphicSubComp->InternalUpdateRenderOpacity();
				}

				if (bUpdateInvertColor)
				{
					GraphicSubComp->InternalUpdateInvertColorState();
				}

				if (bUpdateGraying)
				{
					GraphicSubComp->InternalUpdateGrayingState();
				}
			}

			if (bUpdateInteractable)
			{
				SubComponent->InternalUpdateInteractableState();
			}

			if (bUpdateBlockRaycasts)
			{
				SubComponent->InternalUpdateBlockRaycastsState();
			}

			if (bUpdateEnableState)
			{
				SubComponent->UpdateEnableState();
			}
		}
		
		if (bUpdateRenderOpacity)
		{
			OnRenderOpacityChanged();
		}
		
		if (bUpdateInvertColor)
		{
			OnInvertColorStateChanged();
		}

		if (bUpdateGraying)
		{
			OnGrayingStateChanged();
		}

		if (bUpdateInteractable)
		{
			OnInteractableStateChanged();
		}

		if (bUpdateBlockRaycasts)
		{
			OnBlockRaycastsStateChanged();
		}

		if (bUpdateEnableState)
		{
			if (bInvokeOnEnable)
			{
				OnEnable();

				if (bIsRootComponent)
				{
					if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
					{
						WidgetActor->OnActorEnable();
					}
				}
			}
			else if (bInvokeOnDisable)
			{
				OnDisable();

				if (bIsRootComponent)
				{
					if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(GetOwner()))
					{
						WidgetActor->OnActorDisable();
					}
				}
			}
		}
	}
}

#if WITH_EDITORONLY_DATA

void UBehaviourComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBehaviourComponent, bEnabled))
	{
		SetEnabled(bEnabled);
	}
}

void UBehaviourComponent::AddSubComponentForEditor(UBehaviourSubComponent* SubComponent)
{
	if (!IsValid(SubComponent) || SubComponent->GetOuter() != this)
		return;

	if (!SubComponents.Contains(SubComponent))
	{
		SubComponents.Add(SubComponent);

		if (bHasBeenAwaken)
		{
			SubComponent->InternalAwake();
		}

		if (bHasBeenEnabled)
		{
			SubComponent->SetEnabled(true);
		}
	}
}

void UBehaviourComponent::MoveSubComponentForEditor(int32 OldIndex, int32 NewIndex)
{
	if (SubComponents.IsValidIndex(OldIndex) && 
		SubComponents.IsValidIndex(NewIndex) && OldIndex != NewIndex)
	{
		const auto Component = SubComponents[OldIndex];
		SubComponents.RemoveAt(OldIndex, 1, false);
		SubComponents.Insert(Component, NewIndex);
	}
}

void UBehaviourComponent::RemoveSubComponentForEditor(int32 Index)
{
	if (SubComponents.IsValidIndex(Index))
	{
		const auto SubComponent = SubComponents[Index];
		if (IsValid(SubComponent))
		{
			SubComponent->SetEnabled(false);
			SubComponent->InternalDestroy();
		}
		SubComponents.RemoveAt(Index, 1, false);
	}
}

#endif

void UBehaviourComponent::InternalAwake()
{
	if (!IsValid(OwnerCanvas))
	{
		if (const UBehaviourComponent* ParentBehaviourComp = Cast<UBehaviourComponent>(GetAttachParent()))
		{
			OwnerCanvas = ParentBehaviourComp->OwnerCanvas;
		}
	}
	
	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->InternalAwake();
	}
	
	Awake();
}

void UBehaviourComponent::InternalDestroy()
{
	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->InternalDestroy();
	}

	if (bHasBeenAwaken)
	{
		bHasBeenAwaken = false;
		OnDestroy();
	}
}

UCanvasSubComponent* UBehaviourComponent::GetOwnerCanvas() const
{
	return OwnerCanvas;
}

void UBehaviourComponent::UpdateParentEnabledInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (IsValid(ParentBehaviourComp))
	{
		bParentEnabledInHierarchy = ParentBehaviourComp->IsEnabledInHierarchy();
		return;
	}

	bParentEnabledInHierarchy = true;
}

void UBehaviourComponent::UpdateEnableState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	bool bInvokeOnDisable = false;
	bool bInvokeOnEnable = false;
	
	BehaviourComp->UpdateParentEnabledInHierarchy(ParentBehaviourComp);
	const bool bIsEnabledInHierarchy = BehaviourComp->IsEnabledInHierarchy();
 
	if (bIsEnabledInHierarchy != BehaviourComp->bHasBeenEnabled)
	{
		if (BehaviourComp->bHasBeenEnabled)
		{
			BehaviourComp->bHasBeenEnabled = false;
			bInvokeOnDisable = true;
		}
		else
		{
			if (!BehaviourComp->bHasBeenEnabled)
			{
				BehaviourComp->bHasBeenEnabled = true;
				bInvokeOnEnable = true;
			}
		}
	}

	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				UpdateEnableState(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->UpdateEnableState();
	}

	if (bInvokeOnEnable)
	{
		BehaviourComp->OnEnable();

		if (BehaviourComp->bIsRootComponent)
		{
			if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(BehaviourComp->GetOwner()))
			{
				WidgetActor->OnActorEnable();
			}
		}
	}
	else if (bInvokeOnDisable)
	{
		BehaviourComp->OnDisable();

		if (BehaviourComp->bIsRootComponent)
		{
			if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(BehaviourComp->GetOwner()))
			{
				WidgetActor->OnActorDisable();
			}
		}
	}
}

void UBehaviourComponent::UpdateParentInteractableInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (!bIgnoreParentRenderOpacity && IsValid(ParentBehaviourComp))
	{
		bParentInteractableInHierarchy = ParentBehaviourComp->IsInteractableInHierarchy();
		return;
	}

	bParentInteractableInHierarchy = true;
}

void UBehaviourComponent::InternalUpdateInteractableState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentInteractableInHierarchy(ParentBehaviourComp);
	const bool bIsInteractableInHierarchy = BehaviourComp->IsInteractableInHierarchy();
	if (bIsInteractableInHierarchy != BehaviourComp->bCurrentInteractable)
	{
		BehaviourComp->bCurrentInteractable = bIsInteractableInHierarchy;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				InternalUpdateInteractableState(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->InternalUpdateInteractableState();
	}
		
	BehaviourComp->OnInteractableStateChanged();
}

void UBehaviourComponent::UpdateParentBlockRaycastsInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (!bIgnoreParentBlockRaycasts && IsValid(ParentBehaviourComp))
	{
		bParentBlockRaycastsInHierarchy = ParentBehaviourComp->IsBlockRaycastsInHierarchy();
		return;
	}

	bParentBlockRaycastsInHierarchy = true;
}

void UBehaviourComponent::InternalUpdateBlockRaycastsState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentBlockRaycastsInHierarchy(ParentBehaviourComp);
	const bool bIsBlockRaycastsInHierarchy = BehaviourComp->IsBlockRaycastsInHierarchy();
	if (bIsBlockRaycastsInHierarchy != BehaviourComp->bCurrentBlockRaycasts)
	{
		BehaviourComp->bCurrentBlockRaycasts = bIsBlockRaycastsInHierarchy;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				InternalUpdateBlockRaycastsState(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->InternalUpdateBlockRaycastsState();
	}
		
	BehaviourComp->OnBlockRaycastsStateChanged();
}

void UBehaviourComponent::UpdateParentIgnoreReversedGraphicsInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (!bIgnoreParentReversedGraphics && IsValid(ParentBehaviourComp))
	{
		bParentIgnoreReversedGraphicsInHierarchy = ParentBehaviourComp->IsIgnoreReversedGraphicsInHierarchy();
		return;
	}

	bParentIgnoreReversedGraphicsInHierarchy = true;
}

void UBehaviourComponent::InternalUpdateIgnoreReversedGraphicsState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentIgnoreReversedGraphicsInHierarchy(ParentBehaviourComp);
	const bool bIsIgnoreReversedGraphicsInHierarchy = BehaviourComp->IsIgnoreReversedGraphicsInHierarchy();
	if (bIsIgnoreReversedGraphicsInHierarchy != BehaviourComp->bCurrentIgnoreReversedGraphics)
	{
		BehaviourComp->bCurrentIgnoreReversedGraphics = bIsIgnoreReversedGraphicsInHierarchy;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				InternalUpdateIgnoreReversedGraphicsState(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}
}

void UBehaviourComponent::UpdateParentGrayingInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (ParentBehaviourComp)
	{
		bParentGrayingInHierarchy = ParentBehaviourComp->IsGrayingInHierarchy();
		return;
	}

	bParentGrayingInHierarchy = false;
}

void UBehaviourComponent::UpdateGrayingStateChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentGrayingInHierarchy(ParentBehaviourComp);
	const bool bIsGrayingInHierarchy = BehaviourComp->IsGrayingInHierarchy();
	if (bIsGrayingInHierarchy != BehaviourComp->bCurrentGraying)
	{
		BehaviourComp->bCurrentGraying = bIsGrayingInHierarchy;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				UpdateGrayingStateChanged(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		if (UGraphicSubComponent* GraphicSubComp = Cast<UGraphicSubComponent>(SubComponent))
		{
			GraphicSubComp->InternalUpdateGrayingState();
		}
	}
		
	BehaviourComp->OnGrayingStateChanged();
}

void UBehaviourComponent::UpdateParentInvertColorInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (ParentBehaviourComp)
	{
		bParentInvertColorInHierarchy = ParentBehaviourComp->IsInvertColorInHierarchy();
		return;
	}

	bParentInvertColorInHierarchy = false;
}

void UBehaviourComponent::UpdateInvertColorStateChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentInvertColorInHierarchy(ParentBehaviourComp);
	const bool bIsInvertColorInHierarchy = BehaviourComp->IsInvertColorInHierarchy();
	if (bIsInvertColorInHierarchy != BehaviourComp->bCurrentInvertColor)
	{
		BehaviourComp->bCurrentInvertColor = bIsInvertColorInHierarchy;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				UpdateInvertColorStateChanged(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		if (UGraphicSubComponent* GraphicSubComp = Cast<UGraphicSubComponent>(SubComponent))
		{
			GraphicSubComp->InternalUpdateInvertColorState();
		}
	}
		
	BehaviourComp->OnInvertColorStateChanged();
}

void UBehaviourComponent::UpdateParentRenderOpacityInHierarchy(const UBehaviourComponent* ParentBehaviourComp)
{
	if (!bIgnoreParentRenderOpacity && ParentBehaviourComp)
	{
		ParentRenderOpacityInHierarchy = ParentBehaviourComp->GetRenderOpacityInHierarchy();
		return;
	}

	ParentRenderOpacityInHierarchy = 1;
}

void UBehaviourComponent::UpdateRenderOpacityChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;

	BehaviourComp->UpdateParentRenderOpacityInHierarchy(ParentBehaviourComp);
	const float ParentRenderOpacity = BehaviourComp->GetRenderOpacityInHierarchy();
	if (!FMath::IsNearlyEqual(ParentRenderOpacity, BehaviourComp->CurrentRenderOpacity))
	{
		BehaviourComp->CurrentRenderOpacity = ParentRenderOpacity;
	}
	else
	{
		return;
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				UpdateRenderOpacityChanged(ChildBehaviourComp, BehaviourComp);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		if (UGraphicSubComponent* GraphicSubComp = Cast<UGraphicSubComponent>(SubComponent))
		{
			GraphicSubComp->InternalUpdateRenderOpacity();
		}
	}
		
	BehaviourComp->OnRenderOpacityChanged();
}

void UBehaviourComponent::InternalOnZOrderChanged(const UBehaviourComponent* BehaviourComp)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->IsRegistered() || !BehaviourComp->HasBeenAwaken())
		return;
		
	TArray<UBehaviourSubComponent*, TInlineAllocator<8>> Components;
	BehaviourComp->GetSubComponents(Components);
	
	UCanvasRendererSubComponent* Renderer = nullptr;

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		if (Cast<UCanvasSubComponent>(SubComponent))
		{
			SubComponent->InternalOnZOrderChanged();
			return;
		}

		if (const auto CanvasRenderer = Cast<UCanvasRendererSubComponent>(SubComponent))
		{
			Renderer = CanvasRenderer;
		}
	}

	if (Renderer)
	{
		Renderer->InternalOnZOrderChanged();
	}

	for (const auto& ChildComp : BehaviourComp->GetAttachChildren())
	{
		InternalOnZOrderChanged(Cast<UBehaviourComponent>(ChildComp));
	}
}

void UBehaviourComponent::SetOwnerCanvas(UCanvasSubComponent* InCanvas, bool bUpdateOwnerCanvas, bool bThroughChildren)
{
	InternalSetOwnerCanvas(this, InCanvas, bUpdateOwnerCanvas, bThroughChildren);
}

void UBehaviourComponent::InternalSetOwnerCanvas(UBehaviourComponent* BehaviourComp, UCanvasSubComponent* InCanvas,
	bool bUpdateOwnerCanvas, bool bThroughChildren)
{
	if (!IsValid(BehaviourComp))
		return;
	
	if (bUpdateOwnerCanvas)
	{
		if(BehaviourComp->OwnerCanvas != InCanvas)
		{
			BehaviourComp->OwnerCanvas = InCanvas;
		}
		else
		{
			return;
		}
	}
	
	BehaviourComp->OnCanvasHierarchyChanged();
		
	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	BehaviourComp->GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->OnCanvasHierarchyChanged();
	}
	
	if (bThroughChildren)
	{
		bool bNeedLoop = true;
		while(bNeedLoop)
		{
			bNeedLoop = false;
			BehaviourComp->ResetAttachChildChanged();

			const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
			for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
			{
				const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
				if (IsValid(ChildBehaviourComp))
				{
					InternalSetOwnerCanvas(ChildBehaviourComp, InCanvas, bUpdateOwnerCanvas, bThroughChildren);
				}
				
				if (BehaviourComp->IsAttachChildChanged())
				{
					bNeedLoop = true;
					break;
				}
			}
		}
	}
}

void UBehaviourComponent::SetEnabled(bool bNewEnabled)
{
	if (bEnabled != bNewEnabled)
	{
		bEnabled = bNewEnabled;
		UpdateEnableState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

bool UBehaviourComponent::IsEnabledInHierarchy() const
{
#if WITH_EDITORONLY_DATA
	if (GetDefault<UUGUISettings>()->bVisibilityForEditor && GetWorld() && GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		return bIsVisibleForEditor && bParentEnabledInHierarchy;
	}
	return bEnabled && bParentEnabledInHierarchy;
#else
	return bEnabled && bParentEnabledInHierarchy;
#endif
}

void UBehaviourComponent::SetInteractable(bool bInInteractable)
{
	if (bInteractable != bInInteractable)
	{
		bInteractable = bInInteractable;
		InternalUpdateInteractableState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetBlockRaycasts(bool bInBlockRaycasts)
{
	if (bBlockRaycasts != bInBlockRaycasts)
	{
		bBlockRaycasts = bInBlockRaycasts;
		InternalUpdateBlockRaycastsState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetIgnoreReversedGraphics(bool bInIgnoreReversedGraphics)
{
	if (bIgnoreReversedGraphics != bInIgnoreReversedGraphics)
	{
		bIgnoreReversedGraphics = bInIgnoreReversedGraphics;
		InternalUpdateIgnoreReversedGraphicsState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetGraying(bool bInGraying)
{
	if (bGraying != bInGraying)
	{
		bGraying = bInGraying;
		UpdateGrayingStateChanged(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetInvertColor(bool bInInvertColor)
{
	if (bInvertColor != bInInvertColor)
	{
		bInvertColor = bInInvertColor;
		UpdateInvertColorStateChanged(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetRenderOpacity(float InRenderOpacity)
{
	if (RenderOpacity != InRenderOpacity)
	{
		RenderOpacity = InRenderOpacity;
		UpdateRenderOpacityChanged(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetIgnoreParentRenderOpacity(bool bInIgnoreParentRenderOpacity)
{
	if (bIgnoreParentRenderOpacity != bInIgnoreParentRenderOpacity)
	{
		bIgnoreParentRenderOpacity = bInIgnoreParentRenderOpacity;
		UpdateRenderOpacityChanged(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetIgnoreParentInteractable(bool bInIgnoreParentInteractable)
{
	if (bIgnoreParentInteractable != bInIgnoreParentInteractable)
	{
		bIgnoreParentInteractable = bInIgnoreParentInteractable;
		InternalUpdateInteractableState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetIgnoreParentBlockRaycasts(bool bInIgnoreParentBlockRaycasts)
{
	if (bIgnoreParentBlockRaycasts != bInIgnoreParentBlockRaycasts)
	{
		bIgnoreParentBlockRaycasts = bInIgnoreParentBlockRaycasts;
		InternalUpdateBlockRaycastsState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetIgnoreParentReversedGraphics(bool bInIgnoreParentReversedGraphics)
{
	if (bIgnoreParentReversedGraphics != bInIgnoreParentReversedGraphics)
	{
		bIgnoreParentReversedGraphics = bInIgnoreParentReversedGraphics;
		InternalUpdateIgnoreReversedGraphicsState(this, Cast<UBehaviourComponent>(GetAttachParent()));
	}
}

void UBehaviourComponent::SetZOrder(int32 InZOrder)
{
	if (ZOrder != InZOrder)
	{
		ZOrder = InZOrder;
		InternalOnZOrderChanged(this);
	}
}

void UBehaviourComponent::OnChildAttached(USceneComponent* ChildComponent)
{
	bAttachChildChanged = true;
	Super::OnChildAttached(ChildComponent);
	OnChildAttachmentChanged();
}

void UBehaviourComponent::OnChildDetached(USceneComponent* ChildComponent)
{
	bAttachChildChanged = true;
	Super::OnChildDetached(ChildComponent);
	OnChildAttachmentChanged();
}

void UBehaviourComponent::InternalOnRectTransformDimensionsChange()
{
	if (!HasBeenAwaken())
		return;
	
	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->OnRectTransformDimensionsChange();
	}
	
	OnRectTransformDimensionsChange();
}

void UBehaviourComponent::InternalOnChildAttachmentChanged()
{
	TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
	GetSubComponents(Components);

	for (UBehaviourSubComponent* SubComponent : Components)
	{
		SubComponent->OnChildAttachmentChanged();
	}
	
	OnChildAttachmentChanged();
}

void UBehaviourComponent::InternalTransformParentChanged()
{
	if (!IsRegistered() || bDisableTransformParentChanged)
		return;

	bool bUpdateOwnerCanvas = true;
	if (GetComponent(UCanvasSubComponent::StaticClass(), true))
	{
		bUpdateOwnerCanvas = false;
	}

	const UBehaviourComponent* ParentBehaviourComp = Cast<UBehaviourComponent>(GetAttachParent());
	if (IsValid(ParentBehaviourComp) && OwnerCanvas == ParentBehaviourComp->OwnerCanvas)
	{
		bUpdateOwnerCanvas = false;
	}
	
	UpdateTransformParentChanged(this, Cast<UBehaviourComponent>(GetAttachParent()), bUpdateOwnerCanvas);
}

void UBehaviourComponent::UpdateTransformParentChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp,
	bool bUpdateOwnerCanvas, bool bUpdateEnableState, bool bUpdateRenderOpacity, bool bUpdateInvertColor, bool bUpdateGraying, bool bUpdateInteractable, bool bUpdateBlockRaycasts, bool bUpdateIgnoreReversedGraphics)
{
	if (!IsValid(BehaviourComp) || !BehaviourComp->HasBeenAwaken())
		return;

	if (bUpdateOwnerCanvas)
	{
		if (!BehaviourComp->GetComponent(UCanvasSubComponent::StaticClass(), true))
		{
			if (IsValid(ParentBehaviourComp))
			{
				BehaviourComp->OwnerCanvas = ParentBehaviourComp->OwnerCanvas;
			}
		}
		else
		{
			bUpdateOwnerCanvas = false;
		}
	}

	const bool bUpdateValue = BehaviourComp->IsRegistered() && BehaviourComp->HasBeenAwaken();
	
	bUpdateRenderOpacity = bUpdateValue && bUpdateRenderOpacity;
	if (bUpdateRenderOpacity)
	{
		BehaviourComp->UpdateParentRenderOpacityInHierarchy(ParentBehaviourComp);
		const float ParentRenderOpacity = BehaviourComp->GetRenderOpacityInHierarchy();
		bUpdateRenderOpacity = !FMath::IsNearlyEqual(ParentRenderOpacity, BehaviourComp->CurrentRenderOpacity);
		if (bUpdateRenderOpacity)
		{
			BehaviourComp->CurrentRenderOpacity = ParentRenderOpacity;
		}
	}
	
	bUpdateInvertColor = bUpdateValue && bUpdateInvertColor;
	if (bUpdateInvertColor)
	{
		BehaviourComp->UpdateParentInvertColorInHierarchy(ParentBehaviourComp);
		const bool bIsInvertColorInHierarchy = BehaviourComp->IsInvertColorInHierarchy();
		bUpdateInvertColor = bIsInvertColorInHierarchy != BehaviourComp->bCurrentInvertColor;
		if (bUpdateInvertColor)
		{
			BehaviourComp->bCurrentInvertColor = bIsInvertColorInHierarchy;
		}
	}

	bUpdateGraying = bUpdateValue && bUpdateGraying;
	if (bUpdateGraying)
	{
		BehaviourComp->UpdateParentGrayingInHierarchy(ParentBehaviourComp);
		const bool bIsGrayingInHierarchy = BehaviourComp->IsGrayingInHierarchy();
		bUpdateGraying = bIsGrayingInHierarchy != BehaviourComp->bCurrentGraying;
		if (bUpdateGraying)
		{
			BehaviourComp->bCurrentGraying = bIsGrayingInHierarchy;
		}
	}

	bUpdateInteractable = bUpdateValue && bUpdateInteractable;
	if (bUpdateInteractable)
	{
		BehaviourComp->UpdateParentInteractableInHierarchy(ParentBehaviourComp);
		const bool bIsInteractableInHierarchy = BehaviourComp->IsInteractableInHierarchy();
		bUpdateInteractable = bIsInteractableInHierarchy != BehaviourComp->bCurrentInteractable;
		if (bUpdateInteractable)
		{
			BehaviourComp->bCurrentInteractable = bIsInteractableInHierarchy;
		}
	}
	
	bUpdateBlockRaycasts = bUpdateValue && bUpdateBlockRaycasts;
	if (bUpdateBlockRaycasts)
	{
		BehaviourComp->UpdateParentBlockRaycastsInHierarchy(ParentBehaviourComp);
		const bool bIsBlockRaycastsInHierarchy = BehaviourComp->IsBlockRaycastsInHierarchy();
		bUpdateBlockRaycasts = bIsBlockRaycastsInHierarchy != BehaviourComp->bCurrentBlockRaycasts;
		if (bUpdateBlockRaycasts)
		{
			BehaviourComp->bCurrentBlockRaycasts = bIsBlockRaycastsInHierarchy;
		}
	}

	bUpdateIgnoreReversedGraphics = bUpdateValue && bUpdateIgnoreReversedGraphics;
	if (bUpdateIgnoreReversedGraphics)
	{
		BehaviourComp->UpdateParentIgnoreReversedGraphicsInHierarchy(ParentBehaviourComp);
		const bool bIsIgnoreReversedGraphicsInHierarchy = BehaviourComp->IsIgnoreReversedGraphicsInHierarchy();
		bUpdateIgnoreReversedGraphics = bIsIgnoreReversedGraphicsInHierarchy != BehaviourComp->bCurrentIgnoreReversedGraphics;
		if (bUpdateIgnoreReversedGraphics)
		{
			BehaviourComp->bCurrentIgnoreReversedGraphics = bIsIgnoreReversedGraphicsInHierarchy;
		}
	}

	bUpdateEnableState = bUpdateValue && bUpdateEnableState;
	bool bInvokeOnDisable = false;
	bool bInvokeOnEnable = false;
	if (bUpdateEnableState)
	{
		BehaviourComp->UpdateParentEnabledInHierarchy(ParentBehaviourComp);
		const bool bIsEnabledInHierarchy = BehaviourComp->IsEnabledInHierarchy();
		bUpdateEnableState = bIsEnabledInHierarchy != BehaviourComp->bHasBeenEnabled;
		if (bUpdateEnableState)
		{
			if (BehaviourComp->bHasBeenEnabled)
			{
				BehaviourComp->bHasBeenEnabled = false;
				bInvokeOnDisable = true;
			}
			else
			{
				if (!BehaviourComp->bHasBeenEnabled)
				{
					BehaviourComp->bHasBeenEnabled = true;
					bInvokeOnEnable = true;
				}
			}
		}
	}

	{
		TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
		BehaviourComp->GetSubComponents(Components);

		for (UBehaviourSubComponent* SubComponent : Components)
		{
			SubComponent->OnTransformParentChanged();
		}

		BehaviourComp->OnTransformParentChanged();
	}
	
	bool bNeedLoop = true;
	while(bNeedLoop)
	{
		bNeedLoop = false;
		BehaviourComp->ResetAttachChildChanged();

		const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
		for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
		{
			const auto ChildBehaviourComp = Cast<UBehaviourComponent>(BehaviourChildren[Index]);
			if (IsValid(ChildBehaviourComp))
			{
				UpdateTransformParentChanged(ChildBehaviourComp, BehaviourComp, bUpdateOwnerCanvas, bUpdateRenderOpacity,
					bUpdateInvertColor, bUpdateGraying, bUpdateInteractable, bUpdateBlockRaycasts);
			}
				
			if (BehaviourComp->IsAttachChildChanged())
			{
				bNeedLoop = true;
				break;
			}
		}
	}

	if (bUpdateEnableState || bUpdateRenderOpacity || bUpdateInvertColor || bUpdateGraying || bUpdateInteractable || bUpdateBlockRaycasts)
	{
		TArray<UBehaviourSubComponent*, TInlineAllocator<16>> Components;
		BehaviourComp->GetSubComponents(Components);

		for (UBehaviourSubComponent* SubComponent : Components)
		{
			if (UGraphicSubComponent* GraphicSubComp = Cast<UGraphicSubComponent>(SubComponent))
			{
				if (bUpdateRenderOpacity)
				{
					GraphicSubComp->InternalUpdateRenderOpacity();
				}

				if (bUpdateInvertColor)
				{
					GraphicSubComp->InternalUpdateInvertColorState();
				}

				if (bUpdateGraying)
				{
					GraphicSubComp->InternalUpdateGrayingState();
				}
			}

			if (bUpdateInteractable)
			{
				SubComponent->InternalUpdateInteractableState();
			}

			if (bUpdateBlockRaycasts)
			{
				SubComponent->InternalUpdateBlockRaycastsState();
			}

			if (bUpdateEnableState)
			{
				SubComponent->UpdateEnableState();
			}
		}
		
		if (bUpdateRenderOpacity)
		{
			BehaviourComp->OnRenderOpacityChanged();
		}
		
		if (bUpdateInvertColor)
		{
			BehaviourComp->OnInvertColorStateChanged();
		}

		if (bUpdateGraying)
		{
			BehaviourComp->OnGrayingStateChanged();
		}

		if (bUpdateInteractable)
		{
			BehaviourComp->OnInteractableStateChanged();
		}

		if (bUpdateBlockRaycasts)
		{
			BehaviourComp->OnBlockRaycastsStateChanged();
		}

		if (bUpdateEnableState)
		{
			if (bInvokeOnEnable)
			{
				BehaviourComp->OnEnable();

				if (BehaviourComp->bIsRootComponent)
				{
					if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(BehaviourComp->GetOwner()))
					{
						WidgetActor->OnActorEnable();
					}
				}
			}
			else if (bInvokeOnDisable)
			{
				BehaviourComp->OnDisable();

				if (BehaviourComp->bIsRootComponent)
				{
					if (AWidgetActor* WidgetActor = Cast<AWidgetActor>(BehaviourComp->GetOwner()))
					{
						WidgetActor->OnActorDisable();
					}
				}
			}
		}
	}
}

UBehaviourSubComponent* UBehaviourComponent::AddSubComponentByClass(TSubclassOf<UBehaviourSubComponent> SubCompClass)
{
	UClass* RootDisallowClass = nullptr;
	const auto bDisallowMultipleComponent = IsDisallowMultipleComponent(SubCompClass, RootDisallowClass);

	if (bDisallowMultipleComponent)
	{
		for (const auto& SubComponent : SubComponents)
		{
			if (IsValid(SubComponent) && SubComponent->IsA(RootDisallowClass))
			{
				if (SubCompClass == RootDisallowClass)
				{
					// TODO 非编辑器下没有GetDisplayNameText函数
					// UE_LOG(LogUGUI, Error, TEXT("AddSubComponentByClass --- Can't add '%s' because a '%s' already added to the BehaviourComponent!"),
					//    *SubCompClass->GetDisplayNameText().ToString(), *SubCompClass->GetDisplayNameText().ToString());
				}
				else
				{
					// TODO 非编辑器下没有GetDisplayNameText函数
					// UE_LOG(LogUGUI, Error, TEXT("AddSubComponentByClass --- Can't add '%s' because a '%s' already added to the BehaviourComponent! A BehaviourComponent can only contain one '{2}' component."),
					//	  *SubCompClass->GetDisplayNameText().ToString(), *SubComponent->GetClass()->GetDisplayNameText().ToString(), *RootDisallowClass->GetDisplayNameText().ToString());
				}	
				return nullptr;
			}
		}
	}

	TArray<UClass*, TInlineAllocator<8>> RequireSubClasses;
	GetRequireSubClasses(SubCompClass, RequireSubClasses);
	for (const auto& SubComponent : SubComponents)
	{
		if (IsValid(SubComponent))
		{
			RequireSubClasses.Remove(SubComponent->GetClass());
		}
	}

	for (int32 Index = RequireSubClasses.Num() - 1; Index >= 0; --Index)
	{
		const auto& SubClass = RequireSubClasses[Index];
		if (SubClass)
		{
			const auto SubObject = NewObject<UBehaviourSubComponent>(this, SubClass, NAME_None, EObjectFlags::RF_Transient);
			if (!IsValid(SubObject))
				continue;

			SubComponents.Add(SubObject);

			SubObject->InternalAdd();

			if (bHasBeenAwaken)
			{
				SubObject->InternalAwake();
			}

			if (bHasBeenEnabled)
			{
				SubObject->SetEnabled(true);
			}
		}
	}

	const auto SubObject = NewObject<UBehaviourSubComponent>(this, SubCompClass, NAME_None, EObjectFlags::RF_Transient);
	if (!IsValid(SubObject))
		return nullptr;

	SubComponents.Add(SubObject);

	SubObject->InternalAdd();

	if (bHasBeenAwaken)
	{
		SubObject->InternalAwake();
	}

	if (bHasBeenEnabled)
	{
		SubObject->SetEnabled(true);
	}
	
	return SubObject;
}

bool UBehaviourComponent::IsDisallowMultipleComponent(UClass* InClass, UClass*& RootDisallowClass)
{
	// TODO 非编辑器下没有HasMetaData函数,后面用函数重载
	/*bool bIsDisallow = false;
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
	return bIsDisallow;*/
	return false;
}

bool UBehaviourComponent::RemoveSubComponentByClass(TSubclassOf<UBehaviourSubComponent> SubCompClass, bool bRemoveAssociatedComponents)
{
	TArray<int32, TInlineAllocator<8>> Components;

	for (int32 Index = 0, Count = SubComponents.Num(); Index < Count; ++Index)
	{
		const auto SubComponent = SubComponents[Index];
		if (IsValid(SubComponent))
		{
			if (SubComponent->IsA(SubCompClass))
			{
				Components.Add(Index);
			}
		}
	}

	if (Components.Num() == 0)
		return true;

	TArray<UBehaviourSubComponent*, TInlineAllocator<8>> AssociatedComponents;
	TArray<UClass*, TInlineAllocator<8>> RequireSubClasses;

	for (int32 Index = 0, Count = SubComponents.Num(); Index < Count; ++Index)
	{
		if (Components.Contains(Index))
		{
			continue;
		}

		RequireSubClasses.Reset();
		
		const auto SubComponent = SubComponents[Index];
		if (IsValid(SubComponent))
		{
			GetRequireSubClasses(SubComponent->GetClass(), RequireSubClasses);
		}

		if (RequireSubClasses.Num() > 0)
		{
			for (const auto& RequireSubClass : RequireSubClasses)
			{
				bool bBreak = false;
				
				for (const auto& ElemIndex : Components)
				{
					if (SubComponents.IsValidIndex(ElemIndex))
					{
						if (IsValid(SubComponents[ElemIndex]) && SubComponents[ElemIndex]->IsA(RequireSubClass))
						{
							bBreak = true;
							AssociatedComponents.Add(SubComponent);
							break;
						}
					}
				}

				if (bBreak)
				{
					break;
				}
			}

			if (!bRemoveAssociatedComponents && AssociatedComponents.Num() > 0)
			{
				// TODO 非编辑器下没有GetDisplayNameText函数
				//UE_LOG(LogUGUI, Error, TEXT("RemoveSubComponentByClass --- Can't remove '%s' because '%s' depends on it"),
				//	*SubCompClass->GetDisplayNameText().ToString(), *AssociatedComponents[0]->GetClass()->GetDisplayNameText().ToString());
				return false;
			}
		}
	}

	for (int32 Index = Components.Num() - 1; Index >= 0; --Index)
	{
		const int32 ElemIndex = Components[Index];
		if (SubComponents.IsValidIndex(ElemIndex))
		{
			const auto SubComponent = SubComponents[ElemIndex];
			SubComponents.RemoveAt(ElemIndex, 1, false);
			
			if (IsValid(SubComponent))
			{
				SubComponent->SetEnabled(false);
				SubComponent->InternalRemove();
				SubComponent->InternalDestroy();
			}
		}
	}

	for (auto& AssociatedComponent : AssociatedComponents)
	{
		if (IsValid(AssociatedComponent))
		{
			AssociatedComponent->SetEnabled(false);
			AssociatedComponent->InternalDestroy();
		}
		
		SubComponents.Remove(AssociatedComponent);
	}

	return true;
}

void UBehaviourComponent::GetRequireSubClasses(UClass* InClass, TArray<UClass*, TInlineAllocator<8>>& RequireSubClasses) const
{
	// TODO 非编辑器下没有GetMetaData函数
	/*while (InClass)
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
	}*/
}

UObject* UBehaviourComponent::GetComponent(UClass* InClass, bool bIncludeInactive) 
{
	const auto SceneComp = Cast<USceneComponent>(this);
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

UObject* UBehaviourComponent::GetComponentByInterface(TSubclassOf<UInterface> Interface, bool bIncludeInactive)
{
	const auto SceneComp = Cast<USceneComponent>(this);
	if (IsValid(SceneComp))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
		if (IsValid(BehaviourComp))
		{
			if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
			{
				if (BehaviourComp->GetClass()->ImplementsInterface(Interface))
				{
					return BehaviourComp;
				}

				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
					{
						if (SubComponent->GetClass()->ImplementsInterface(Interface))
						{
							return SubComponent;
						}
					}
				}
			}
		}
		else if (SceneComp->GetClass()->ImplementsInterface(Interface))
		{
			return SceneComp;
		}
	}

	return nullptr;
}

UObject* UBehaviourComponent::GetComponentInChildren(UClass* InClass, bool bIncludeInactive)
{
	return InternalGetComponentInChildren(this, InClass, bIncludeInactive);
}

UObject* UBehaviourComponent::InternalGetComponentInChildren(USceneComponent* SceneComp, UClass* InClass, bool bIncludeInactive) const
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

UObject* UBehaviourComponent::GetComponentInParent(UClass* InClass, bool bIncludeInactive)
{
	auto SceneComp = Cast<USceneComponent>(this);
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

void UBehaviourComponent::GetComponents(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive)
{
	Components.Reset();

	const auto SceneComp = Cast<USceneComponent>(this);
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

void UBehaviourComponent::GetComponentsInChildren(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive)
{
	Components.Reset();
	
	InternalGetComponentsInChildren(this, Components, InClass, bIncludeInactive);
}

void UBehaviourComponent::InternalGetComponentsInChildren(USceneComponent* SceneComp, TArray<UObject*>& Components,
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

void UBehaviourComponent::GetComponentsInParent(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive)
{
	Components.Reset();

	auto SceneComp = Cast<USceneComponent>(this);
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

		SceneComp = SceneComp->GetAttachParent();
	}
}

/////////////////////////////////////////////////////
