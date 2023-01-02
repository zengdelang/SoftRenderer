#include "Core/WidgetActor.h"
#include "Core/Layout/ChildWidgetActorComponent.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "Core/Render/CanvasSubComponent.h"

/////////////////////////////////////////////////////
// AWidgetActor

#if WITH_EDITORONLY_DATA
FOnBackgroundImageFilePathChanged AWidgetActor::OnBackgroundImageFilePathChanged;
#endif

AWidgetActor::AWidgetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bAllowTickOnDedicatedServer = false;
	
	bAllowTickBeforeBeginPlay = false;
	bAllowReceiveTickEventOnDedicatedServer = false;

#if WITH_EDITORONLY_DATA
	SpriteScale = 0;

	ParentContainerComponent = nullptr;
	AttachParentComponent = nullptr;
#endif

	ParametersObject = nullptr;

	bRebuildChildrenOrder = false;
	bDisableCanvasUpdateRectTransform = false;
	bStripCanvasComponent = false;
	bStripCanvasScalerComponent = false;
	bActorEnabled = false;

#if WITH_EDITOR
	bRecordForUndo = true;
#endif
}

void AWidgetActor::Serialize(FArchive& Ar)
{
	const auto World = GetWorld();
	if (IsValid(World) && World->WorldType == EWorldType::Type::Editor)
	{
		bRebuildChildrenOrder = true;
		ComponentChildrenOrders.Empty();
#if WITH_EDITORONLY_DATA
		SerializeChildrenOrders(RootComponent);
#endif
	}

	Super::Serialize(Ar);
}

void AWidgetActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	if (bRebuildChildrenOrder)
	{
		if (bStripCanvasComponent && IsValid(RootComponent))
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(RootComponent);
			if (IsValid(BehaviourComp))
			{
				BehaviourComp->RemoveSubComponentByClass(UCanvasSubComponent::StaticClass(), true);
				BehaviourComp->RemoveSubComponentByClass(UCanvasScalerSubComponent::StaticClass(), true);
			}
		}
		else
		{
			if (bStripCanvasScalerComponent && IsValid(RootComponent))
			{
				const auto BehaviourComp = Cast<UBehaviourComponent>(RootComponent);
				if (IsValid(BehaviourComp))
				{
					BehaviourComp->RemoveSubComponentByClass(UCanvasScalerSubComponent::StaticClass(), true);
				}
			}
		}

		RebuildChildrenOrders(RootComponent);

		if (AttachSceneComponent.IsValid() && RootComponent->GetAttachParent() == nullptr)
		{
			RootComponent->AttachToComponent(AttachSceneComponent.Get(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
		
		if (IsValid(ParametersObject))
        {
        	SetCustomParametersObject(ParametersObject);
        }

		if (UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(RootComponent))
		{
			BehaviourComp->bIsRootComponent = true;
		}
        	
		AwakeComponents(RootComponent);
	}
}

#if WITH_EDITORONLY_DATA

void AWidgetActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(AWidgetActor, BackgroundImageFilePath))
	{
		OnBackgroundImageFilePathChanged.Broadcast(this, BackgroundImageFilePath);
	}
}

#endif

void AWidgetActor::OnConstruction(const FTransform& Transform)
{
	if (bStripCanvasComponent && IsValid(RootComponent))
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(RootComponent);
		if (IsValid(BehaviourComp))
		{
			BehaviourComp->RemoveSubComponentByClass(UCanvasSubComponent::StaticClass(), true);
			BehaviourComp->RemoveSubComponentByClass(UCanvasScalerSubComponent::StaticClass(), true);
		}
	}
	else
	{
		if (bStripCanvasScalerComponent && IsValid(RootComponent))
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(RootComponent);
			if (IsValid(BehaviourComp))
			{
				BehaviourComp->RemoveSubComponentByClass(UCanvasScalerSubComponent::StaticClass(), true);
			}
		}
	}

	if (AttachSceneComponent.IsValid() && RootComponent->GetAttachParent() == nullptr)
	{
		RootComponent->AttachToComponent(AttachSceneComponent.Get(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
	
	if (IsValid(ParametersObject))
    {
    	SetCustomParametersObject(ParametersObject);
    }

	if (UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(RootComponent))
	{
		BehaviourComp->bIsRootComponent = true;
	}
    	
	AwakeComponents(RootComponent);

#if WITH_EDITORONLY_DATA
	if (IsValid(RootComponent) && RootComponent->GetAttachParent() != ParentContainerComponent)
	{
		UChildWidgetActorComponent* ChildWidgetActorComp = Cast<UChildWidgetActorComponent>(ParentContainerComponent);
		if (IsValid(ChildWidgetActorComp))
		{
			ChildWidgetActorComp->SetChildActorForEditor(this);
		}
	}

	OnWidgetActorConstruction.Broadcast();

	// if rerun
	if (IsValid(AttachParentComponent))
	{
		Destroy();
	}
#endif
}

#if WITH_EDITOR

bool AWidgetActor::Modify(bool bAlwaysMarkDirty)
{
	if (bRecordForUndo)
	{
		return Super::Modify(bAlwaysMarkDirty);
	}
	return false;
}

#endif

void AWidgetActor::AwakeComponents(USceneComponent* SceneComp) const
{
	if (!IsValid(SceneComp))
		return;

	const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
	if (BehaviourComp)
	{
		BehaviourComp->AwakeFromLoad();
		
		bool bNeedLoop = true;
		while(bNeedLoop)
		{
			bNeedLoop = false;
			BehaviourComp->ResetAttachChildChanged();

			const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
			for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
			{
				AwakeComponents(BehaviourChildren[Index]);
				
				if (BehaviourComp->IsAttachChildChanged())
				{
					bNeedLoop = true;
					break;
				}
			}
		}
	}
	else
	{
		for (auto& ChildComp : SceneComp->GetAttachChildren())
		{
			AwakeComponents(ChildComp);
		}
	}
}

void AWidgetActor::RebuildChildrenOrders(USceneComponent* Comp)
{
	if (!IsValid(Comp))
		return;

	const auto OrderPtr = ComponentChildrenOrders.Find(Comp->GetFName());
	if (OrderPtr)
	{
		TMap<FName, USceneComponent*> CacheChildren;
		for (int32 Index = Comp->GetAttachChildren().Num() - 1; Index >= 0; --Index)
		{
			const auto ChildComp = Comp->GetAttachChildren()[Index];
			if (IsValid(ChildComp))
			{
				ChildComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
				CacheChildren.Emplace(ChildComp->GetFName(), ChildComp);
			}
		}

		for (const auto& ChildName : OrderPtr->ChildrenNames)
		{
			const auto CompPtr = CacheChildren.Find(ChildName);
			if (CompPtr && IsValid(*CompPtr))
			{
				(*CompPtr)->AttachToComponent(Comp, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
	}

	for (const auto& ChildComp : Comp->GetAttachChildren())
	{
		if (IsValid(ChildComp))
		{
			RebuildChildrenOrders(ChildComp);
		}
	}
}

#if WITH_EDITORONLY_DATA

void AWidgetActor::SerializeChildrenOrders(const USceneComponent* Comp)
{
	if (!IsValid(Comp))
		return;

	FChildComponentOrder ChildComponentOrder;
	for (const auto& ChildComp : Comp->GetAttachChildren())
	{
		if (IsValid(ChildComp) && ChildComp ->GetOwner() == this && !ChildComp->HasAnyFlags(RF_Transient))
		{
			ChildComponentOrder.ChildrenNames.Emplace(ChildComp->GetFName());
		}
	}

	if (ChildComponentOrder.ChildrenNames.Num() > 0)
	{
		ComponentChildrenOrders.Emplace(Comp->GetFName(), ChildComponentOrder);
	}

	for (const auto& ChildComp : Comp->GetAttachChildren())
	{
		if (IsValid(ChildComp) && ChildComp->GetOwner() == this && !ChildComp->HasAnyFlags(RF_Transient))
		{
			SerializeChildrenOrders(ChildComp);
		}
	}
}

#endif

UClass* AWidgetActor::GetCustomParametersObjectClass()
{
	return ReceiveGetCustomParametersObjectClass();
}

void AWidgetActor::SetCustomParametersObject(UObject* InParametersObject)
{
	ReceiveCustomParametersObject(InParametersObject);
}

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI

void AWidgetActor::RerunConstructionScripts()
{
#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
	if (IsSliceSpawning())
	{
		SlicePostActorConstruction();
	}
#endif
	
	Super::RerunConstructionScripts();
}

void AWidgetActor::SlicePreRegisterComponents()
{
	OnSlicePreRegisterComponents.Broadcast();
}

ESpawnActorSliceAction AWidgetActor::SliceCustomActions()
{
	for (int32 Index = ChildSliceActors.Num() - 1; Index >= 0; --Index)
	{
		const auto& ChildSliceActor = ChildSliceActors[Index];
		if (!ChildSliceActor.IsValid())
		{
			ChildSliceActors.RemoveAt(Index);
			continue;
		}

		if (ChildSliceActor->SliceCreateSCSNodes() == ESpawnActorSliceAction::SpawnActorSliceAction_Done)
		{
			ChildSliceActors.RemoveAt(Index);
			return ESpawnActorSliceAction::SpawnActorSliceAction_Continue;
		}

		return ESpawnActorSliceAction::SpawnActorSliceAction_Continue;
	}
	
	return ESpawnActorSliceAction::SpawnActorSliceAction_Done;
}

void AWidgetActor::SlicePostActorConstructionDone()
{
	OnSliceSpawnActorDone.Broadcast();
}

#endif

/////////////////////////////////////////////////////
