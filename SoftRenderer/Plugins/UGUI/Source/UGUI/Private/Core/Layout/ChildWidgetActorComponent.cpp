#include "Core/Layout/ChildWidgetActorComponent.h"
#include "Animation/UIAnimationSlotComponent.h"
#include "UGUI.h"

#ifndef OPTIMIZE_SPAWN_ACTOR_FOR_UI
#define OPTIMIZE_SPAWN_ACTOR_FOR_UI 0
#endif

/////////////////////////////////////////////////////
// UChildWidgetActorComponent

UChildWidgetActorComponent::UChildWidgetActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), ChildWidgetActor(nullptr)
{
	bStripCanvasComponent = true;
	ParametersObject = nullptr;
}

#if WITH_EDITORONLY_DATA

void UChildWidgetActorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UChildWidgetActorComponent, ChildWidgetActorClass))
	{
		ParametersObject = nullptr;
		if (IsValid(ChildWidgetActorClass.Get()))
		{
			EObjectFlags NewObjectFlags = RF_Transactional;
			if (HasAnyFlags(RF_Transient))
			{
				NewObjectFlags |= RF_Transient;
			}

			AWidgetActor* DefaultChildWidgetActor = Cast<AWidgetActor>(ChildWidgetActorClass->GetDefaultObject());
			if (IsValid(DefaultChildWidgetActor) && IsValid(DefaultChildWidgetActor->GetCustomParametersObjectClass()))
			{
				ParametersObject = NewObject<UObject>(this, DefaultChildWidgetActor->GetCustomParametersObjectClass(), NAME_None, NewObjectFlags);
			}
		}
	}
}

#endif

void UChildWidgetActorComponent::Awake()
{
	for (const auto& AttachChild : GetAttachChildren())
	{
		const auto UIAnimationAttachComp = Cast<UUIAnimationAttachComponent>(AttachChild);
		if (IsValid(UIAnimationAttachComp))
		{
			AttachChildrenNodes.Add(UIAnimationAttachComp->SlotName, UIAnimationAttachComp);
		}
	}
	
	Super::Awake();
	
	CreateChildWidgetActor();
}

void UChildWidgetActorComponent::OnDestroy()
{
	DestroyChildWidgetActor();
	AttachChildrenNodes.Empty();
	
	Super::OnDestroy();
}

void UChildWidgetActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		DestroyChildWidgetActor();
		AttachChildrenNodes.Empty();
	}
	
	Super::EndPlay(EndPlayReason);
}

DECLARE_CYCLE_STAT(TEXT("UIBehaviour --- DynamicSpawnWidgetActor"), STAT_UnrealGUI_ChildWidgetComp_SpawnActor, STATGROUP_UnrealGUI);
void UChildWidgetActorComponent::CreateChildWidgetActor()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_ChildWidgetComp_SpawnActor);

	const AActor* MyOwner = GetOwner();
	
	// Kill spawned actor if we have one
	DestroyChildWidgetActor();

	// If we have a class to spawn.
	if (ChildWidgetActorClass != nullptr)
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			// Before we spawn let's try and prevent cyclic disaster
			bool bSpawn = true;

			const AActor* Actor = MyOwner;
			while (Actor && bSpawn)
			{
				if (Actor->GetClass() == ChildWidgetActorClass)
				{
					bSpawn = false;
					UE_LOG(LogUGUI, Error, TEXT("Found cycle in child widget actor component '%s'.  Not spawning Actor of class '%s' to break."), *GetPathName(), *ChildWidgetActorClass->GetName());
				}
				
				Actor = Actor->GetParentActor();
			}

			if (bSpawn)
			{
				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				Params.bDeferConstruction = true; // We defer construction so that we set ParentComponent prior to component registration so they appear selected
				Params.bAllowDuringConstructionScript = true;
				Params.Owner = GetOwner();
				Params.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
				Params.bSpawnForUI = true;
#endif

				// Spawn actor of desired class
				ConditionalUpdateComponentToWorld();

				const FVector Location = GetComponentLocation();
				const FRotator Rotation = GetComponentRotation();
				
				ChildWidgetActor = Cast<AWidgetActor>(World->SpawnActor(ChildWidgetActorClass, &Location, &Rotation, Params));

				// If spawn was successful, 
				if (IsValid(ChildWidgetActor))
				{
					ChildWidgetActor->bDisableCanvasUpdateRectTransform = true;
					ChildWidgetActor->bStripCanvasComponent = bStripCanvasComponent;
					ChildWidgetActor->ParametersObject = ParametersObject;

#if WITH_EDITOR
					ChildWidgetActor->bRecordForUndo = false;
#endif
					
					ChildWidgetActor->AttachSceneComponent = this;

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
					AWidgetActor* OwnerWidgetActor = Cast<AWidgetActor>(GetOwner());
					if (IsValid(OwnerWidgetActor) && OwnerWidgetActor->IsSliceSpawning())
					{
						ChildWidgetActor->OnSliceSpawnActorDone.AddUObject(this, &UChildWidgetActorComponent::OnSliceSpawnActorDone);
				
						// Parts that we deferred from SpawnActor
						ChildWidgetActor->SliceFinishSpawning(FTransform::Identity, false, nullptr, true);

						OwnerWidgetActor->ChildSliceActors.Add(ChildWidgetActor);
					}
					else
#endif
					{
						// Parts that we deferred from SpawnActor
#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
						ChildWidgetActor->FinishSpawning(GetComponentTransform(), false, nullptr, Params.bSpawnForUI);
#else
						ChildWidgetActor->FinishSpawning(GetComponentTransform(), false);
#endif
						
						if (USceneComponent* ChildRoot = ChildWidgetActor->GetRootComponent())
						{
							const auto ChildRootRectTransform = Cast<URectTransformComponent>(ChildRoot);
							if (IsValid(ChildRootRectTransform))
							{
								ChildRootRectTransform->SetLocalTransform(FTransform::Identity);
							}

#if WITH_EDITORONLY_DATA
							ChildWidgetActor->ParentContainerComponent = this;
#endif
							
							URectTransformComponent* RectTransform = Cast<URectTransformComponent>(ChildRoot);
							if (IsValid(RectTransform))
							{
								RectTransform->SetAnchorAndOffset(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0));
							}

#if WITH_EDITOR
							if (AttachChildrenNodes.Num() > 0 && IsValid(GetWorld()) && GetWorld()->IsGameWorld())
#else
							if (AttachChildrenNodes.Num() > 0)
#endif
							{
								TInlineComponentArray<UUIAnimationSlotComponent*> AnimationsSlotComponents;
								ChildWidgetActor->GetComponents(AnimationsSlotComponents);
								for (const auto& ChildComp : AnimationsSlotComponents)
								{
									if (IsValid(ChildComp))
									{
										const auto AttachChildNodePtr = AttachChildrenNodes.Find(ChildComp->GetFName());
										if (AttachChildNodePtr)
										{
											const auto AttachChildRectTransform = Cast<URectTransformComponent>(*AttachChildNodePtr);
											if (IsValid(AttachChildRectTransform))
											{
												AttachChildRectTransform->AttachToComponent(ChildComp, FAttachmentTransformRules::KeepRelativeTransform);
												AttachChildRectTransform->SetLocalTransform(FTransform::Identity);
												AttachChildRectTransform->SetAnchorAndOffset(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0));
											}
										}
									}
								}
							}
							
#if WITH_EDITOR
							if (World->WorldType == EWorldType::Type::Editor)
							{
								World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
									{
										if (IsValid(ChildWidgetActor))
										{
											if (USceneComponent* ChildRoot = ChildWidgetActor->GetRootComponent())
											{
												URectTransformComponent* RectTransform = Cast<URectTransformComponent>(ChildRoot);
												if (IsValid(RectTransform))
												{
													RectTransform->SetAnchoredPosition3D(FVector(RectTransform->GetAnchoredPosition(), 0));
												}
											}
										}
									}));
							}
#endif
						}
					}
				}
			}
		}
	}
}

void UChildWidgetActorComponent::DestroyChildWidgetActor()
{
	for (const auto& AttachChildNode : AttachChildrenNodes)
	{
		if (IsValid(AttachChildNode.Value))
		{
			AttachChildNode.Value->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	
	auto IsLevelBeingRemoved = [this]() -> bool
	{
		if (const AActor* MyOwner = GetOwner())
		{
			if (const ULevel* MyLevel = MyOwner->GetLevel())
			{
				return MyLevel->bIsBeingRemoved;
			}
		}

		return false;
	};

	if (ChildWidgetActor && !IsLevelBeingRemoved())
	{
		if (!GExitPurge)
		{
			// if still alive, destroy, otherwise just clear the pointer
			const bool bIsChildActorPendingKillOrUnreachable = ChildWidgetActor->IsPendingKillOrUnreachable();

			UWorld* World = ChildWidgetActor->GetWorld();
			
			// World may be nullptr during shutdown
			if (World != nullptr)
			{
				if (!bIsChildActorPendingKillOrUnreachable)
				{
					World->DestroyActor(ChildWidgetActor);
				}
			}
		}
		ChildWidgetActor = nullptr;
	}
}

#if WITH_EDITORONLY_DATA

void UChildWidgetActorComponent::SetChildActorForEditor(AWidgetActor* WidgetActor)
{
	if (!IsValid(WidgetActor))
		return;

	if (ChildWidgetActor != WidgetActor)
	{
		DestroyChildWidgetActor();
	}

	ChildWidgetActor = WidgetActor;

	if (USceneComponent* ChildRoot = ChildWidgetActor->GetRootComponent())
	{
		TGuardValue<TEnumAsByte<EComponentMobility::Type>> MobilityGuard(ChildRoot->Mobility, Mobility);
		ChildRoot->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ChildRoot->SetRelativeTransform(FTransform::Identity);

		URectTransformComponent* RectTransform = Cast<URectTransformComponent>(ChildRoot);
		if (IsValid(RectTransform))
		{
			RectTransform->SetAnchorAndOffset(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0));
		}
	}

	if (IsValid(ParametersObject))
	{
		ChildWidgetActor->SetCustomParametersObject(ParametersObject);
	}
}

void UChildWidgetActorComponent::SetChildWidgetActorClassForEditor(TSubclassOf<AWidgetActor> InClass)
{
	ChildWidgetActorClass = InClass;
	ParametersObject = nullptr;

	if (IsValid(ChildWidgetActorClass.Get()))
	{
		EObjectFlags NewObjectFlags = RF_Transactional;
		if (HasAnyFlags(RF_Transient))
		{
			NewObjectFlags |= RF_Transient;
		}

		AWidgetActor* DefaultChildWidgetActor = Cast<AWidgetActor>(ChildWidgetActorClass->GetDefaultObject());
		if (IsValid(DefaultChildWidgetActor) && IsValid(DefaultChildWidgetActor->GetCustomParametersObjectClass()))
		{
			ParametersObject = NewObject<UObject>(this, DefaultChildWidgetActor->GetCustomParametersObjectClass(), NAME_None, NewObjectFlags);
		}
	}
	
	if (!IsTemplate())
	{
		if (HasBeenAwaken())
		{	 
			DestroyChildWidgetActor();
			CreateChildWidgetActor();
		}
	}
}

#endif

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI

void UChildWidgetActorComponent::OnSliceSpawnActorDone()
{
	const UWorld* World = GetWorld();
	if (!IsValid(ChildWidgetActor) || !IsValid(World))
	{
		return;
	}

	ChildWidgetActor->OnSliceSpawnActorDone.RemoveAll(this);
	
	if (USceneComponent* ChildRoot = ChildWidgetActor->GetRootComponent())
	{
		const auto ChildRootRectTransform = Cast<URectTransformComponent>(ChildRoot);
		if (IsValid(ChildRootRectTransform))
		{
			ChildRootRectTransform->SetLocalTransform(FTransform::Identity);
		}

#if WITH_EDITORONLY_DATA
		ChildWidgetActor->ParentContainerComponent = this;
#endif
							
		URectTransformComponent* RectTransform = Cast<URectTransformComponent>(ChildRoot);
		if (IsValid(RectTransform))
		{
			RectTransform->SetAnchorAndOffset(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0));
		}

#if WITH_EDITOR
		if (AttachChildrenNodes.Num() > 0 && IsValid(GetWorld()) && GetWorld()->IsGameWorld())
#else
		if (AttachChildrenNodes.Num() > 0)
#endif
		{
			TInlineComponentArray<UUIAnimationSlotComponent*> AnimationsSlotComponents;
			ChildWidgetActor->GetComponents(AnimationsSlotComponents);
			for (const auto& ChildComp : AnimationsSlotComponents)
			{
				if (IsValid(ChildComp))
				{
					const auto AttachChildNodePtr = AttachChildrenNodes.Find(ChildComp->GetFName());
					if (AttachChildNodePtr)
					{
						const auto AttachChildRectTransform = Cast<URectTransformComponent>(*AttachChildNodePtr);
						if (IsValid(AttachChildRectTransform))
						{
							AttachChildRectTransform->AttachToComponent(ChildComp, FAttachmentTransformRules::KeepRelativeTransform);
							AttachChildRectTransform->SetLocalTransform(FTransform::Identity);
							AttachChildRectTransform->SetAnchorAndOffset(FVector2D(0, 0), FVector2D(1, 1), FVector2D(0, 0), FVector2D(0, 0));
						}
					}
				}
			}
		}
							
#if WITH_EDITOR
		if (World->WorldType == EWorldType::Type::Editor)
		{
			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (IsValid(ChildWidgetActor))
				{
					if (USceneComponent* ChildRoot = ChildWidgetActor->GetRootComponent())
					{
						URectTransformComponent* RectTransform = Cast<URectTransformComponent>(ChildRoot);
						if (IsValid(RectTransform))
						{
							RectTransform->SetAnchoredPosition3D(FVector(RectTransform->GetAnchoredPosition(), 0));
						}
					}
				}
			}));
		}
#endif
	}
}

#endif

void UChildWidgetActorComponent::SetChildWidgetActorClass(TSubclassOf<AWidgetActor> InClass)
{
	ChildWidgetActorClass = InClass;
	ParametersObject = nullptr;
	if (!IsTemplate())
	{
		if (HasBeenAwaken())
		{	 
			DestroyChildWidgetActor();
			CreateChildWidgetActor();
		}
	}
}

/////////////////////////////////////////////////////
