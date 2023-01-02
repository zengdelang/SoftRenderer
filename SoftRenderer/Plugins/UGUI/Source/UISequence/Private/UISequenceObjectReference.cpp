#include "UISequenceObjectReference.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"

FUISequenceObjectReference FUISequenceObjectReference::CreateForComponent(UActorComponent* InComponent)
{
	check(InComponent);

	FUISequenceObjectReference NewReference;
	NewReference.Type = EUISequenceObjectReferenceType::Component;

	const AActor* Actor = InComponent->GetOwner();
	if (Actor)
	{
		NewReference.PathToComponent = InComponent->GetPathName(Actor);
		return NewReference;
	}

	const UBlueprintGeneratedClass* GeneratedClass = InComponent->GetTypedOuter<UBlueprintGeneratedClass>();
	if (GeneratedClass && GeneratedClass->SimpleConstructionScript)
	{
		const UBlueprint* Blueprint = Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
		if (Blueprint)
		{
			for (const USCS_Node* Node : GeneratedClass->SimpleConstructionScript->GetAllNodes())
			{
				if (Node->ComponentTemplate == InComponent)
				{
					NewReference.PathToComponent = Node->GetVariableName().ToString();
					return NewReference;
				}
			}
		}
	}

	ensureMsgf(false, TEXT("Unable to find parent actor for component. Reference will be unresolvable."));
	return NewReference;
}

FUISequenceObjectReference FUISequenceObjectReference::CreateForActor(AActor* InActor, AActor* ContextActor)
{
	if (InActor == ContextActor)
	{
		return CreateForContextActor();
	}

	FUISequenceObjectReference NewReference;
	check(InActor && ContextActor && InActor->GetLevel() == ContextActor->GetLevel());

	NewReference.Type = EUISequenceObjectReferenceType::ExternalActor;
	NewReference.ActorId = FLazyObjectPtr(InActor).GetUniqueID().GetGuid();
	return NewReference;
}

FUISequenceObjectReference FUISequenceObjectReference::CreateForContextActor()
{
	FUISequenceObjectReference NewReference;
	NewReference.Type = EUISequenceObjectReferenceType::ContextActor;
	return NewReference;
}

UObject* FUISequenceObjectReference::Resolve(AActor* SourceActor) const
{
	check(SourceActor);

	switch(Type)
	{
	case EUISequenceObjectReferenceType::ContextActor:
		return SourceActor;

	case EUISequenceObjectReferenceType::ExternalActor:
		if (ActorId.IsValid())
		{
			// Fixup for PIE
			const int32 PIEInstanceID = SourceActor->GetOutermost()->PIEInstanceID;
			FUniqueObjectGuid FixedUpId(ActorId);
			if (PIEInstanceID != -1)
			{
				FixedUpId = FixedUpId.FixupForPIE(PIEInstanceID);
			}
			
			FLazyObjectPtr LazyPtr;
			LazyPtr = FixedUpId;

			if (AActor* FoundActor = Cast<AActor>(LazyPtr.Get()))
			{
				if (FoundActor->GetLevel() == SourceActor->GetLevel())
				{
					return FoundActor;
				}
			}
		}
		break;

	case EUISequenceObjectReferenceType::Component:
		if (!PathToComponent.IsEmpty())
		{
			return FindObject<UActorComponent>(SourceActor, *PathToComponent);
		}
		break;
	}

	return nullptr;
}

bool FUISequenceObjectReferenceMap::HasBinding(const FGuid& ObjectId) const
{
	return BindingIds.Contains(ObjectId);
}

void FUISequenceObjectReferenceMap::RemoveBinding(const FGuid& ObjectId)
{
	const int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index != INDEX_NONE)
	{
		BindingIds.RemoveAtSwap(Index, 1, false);
		References.RemoveAtSwap(Index, 1, false);
	}
}

void FUISequenceObjectReferenceMap::CreateBinding(const FGuid& ObjectId, const FUISequenceObjectReference& ObjectReference)
{
	int32 ExistingIndex = BindingIds.IndexOfByKey(ObjectId);
	if (ExistingIndex == INDEX_NONE)
	{
		ExistingIndex = BindingIds.Num();

		BindingIds.Add(ObjectId);
		References.AddDefaulted();
	}

	References[ExistingIndex].Array.AddUnique(ObjectReference);
}

void FUISequenceObjectReferenceMap::ResolveBinding(const FGuid& ObjectId, AActor* SourceActor, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	const int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index == INDEX_NONE)
	{
		return;
	}

	for (const FUISequenceObjectReference& Reference : References[Index].Array)
	{
		if (UObject* Object = Reference.Resolve(SourceActor))
		{
			OutObjects.Add(Object);
		}
	}
}

const FUISequenceObjectReference* FUISequenceObjectReferenceMap::FindObjectReference(const FGuid& ObjectId)
{
	const int32 Index = BindingIds.IndexOfByKey(ObjectId);
	if (Index != INDEX_NONE)
	{
		return &References[Index].Array[0];
	}
	return nullptr;
}
