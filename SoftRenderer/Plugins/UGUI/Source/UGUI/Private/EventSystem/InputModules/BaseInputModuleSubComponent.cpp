#include "EventSystem/InputModules/BaseInputModuleSubComponent.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "EventSystem/EventSystemComponent.h"
#include "EventSystem/ExecuteEvents.h"
#include "UGUISubsystem.h"

/////////////////////////////////////////////////////
// UBaseInputModule

UBaseInputModuleSubComponent::UBaseInputModuleSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EventSystem = nullptr;
	InputOverride = nullptr;
	DefaultInput = nullptr;
	AxisEventData = nullptr;
	BaseEventData = nullptr;
	ViewportClient = nullptr;
}

void UBaseInputModuleSubComponent::OnEnable()
{
	Super::OnEnable();

	if (!ViewportClient)
	{
		ViewportClient = UUGUISubsystem::GetEventViewportClient(this);
	}

	EventSystem = Cast<UEventSystemComponent>(GetComponent(UEventSystemComponent::StaticClass()));
	if (IsValid(EventSystem))
	{
		EventSystem->UpdateModules();
	}
}

void UBaseInputModuleSubComponent::OnDisable()
{
	if (IsValid(EventSystem))
	{
		EventSystem->UpdateModules();
	}
	
	Super::OnDisable();
}

IUGUIInputInterface* UBaseInputModuleSubComponent::GetInput()
{
	if (IsValid(InputOverride))
		return Cast<IUGUIInputInterface>(InputOverride);

	if (DefaultInput == nullptr)
	{
		auto Input = Cast<IUGUIInputInterface>(GetOuter());
		if (Input)
		{
			DefaultInput = Cast<UObject>(Input);
		}
		else
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(GetOuter());
			if (IsValid(BehaviourComp) && BehaviourComp->IsActiveAndEnabled())
			{
				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				for (const auto& SubComponent : AllSubComponents)
				{
					if (IsValid(SubComponent) && SubComponent->IsActiveAndEnabled())
					{
						Input = Cast<IUGUIInputInterface>(SubComponent);
						if (Input)
						{
							DefaultInput = Cast<UObject>(Input);
							break;
						}
					}
				}
			}
		}

		if (DefaultInput == nullptr)
		{
			DefaultInput = NewObject<UBaseInput>(this);
		}
	}

	return Cast<IUGUIInputInterface>(DefaultInput);
}

EMoveDirection UBaseInputModuleSubComponent::DetermineMoveDirection(float X, float Y)
{
	return DetermineMoveDirection(X, Y, 0.6f);
}

EMoveDirection UBaseInputModuleSubComponent::DetermineMoveDirection(float X, float Y, float DeadZone)
{
	// if vector is too small... just return
	if ((X * X + Y * Y) < DeadZone * DeadZone)
		return EMoveDirection::MoveDirection_None;

	if (FMath::Abs(X) > FMath::Abs(Y))
	{
		if (X > 0)
			return EMoveDirection::MoveDirection_Right;
		return EMoveDirection::MoveDirection_Left;
	}
	
	if (Y > 0)
		return EMoveDirection::MoveDirection_Up;
	return EMoveDirection::MoveDirection_Down;
}

USceneComponent* UBaseInputModuleSubComponent::FindCommonRoot(USceneComponent* SceneComp1, const USceneComponent* SceneComp2)
{
	if (!IsValid(SceneComp1) || !IsValid(SceneComp2))
		return nullptr;

	auto S1 = SceneComp1;
	while (IsValid(S1))
	{
		auto S2 = SceneComp2;
		while (IsValid(S2))
		{
			if (S1 == S2)
				return S1;
			S2 = S2->GetAttachParent();
		}
		S1 = S1->GetAttachParent();
	}
	return nullptr;
}

void UBaseInputModuleSubComponent::HandlePointerExitAndEnter(UPointerEventData* CurrentPointerData, USceneComponent* NewEnterTarget) const
{
	if (!IsValid(CurrentPointerData))
		return;
	
	// if we have no target / pointerEnter has been deleted
	// just send exit events to anything we are tracking
	// then exit
	if (!IsValid(NewEnterTarget) || !IsValid(CurrentPointerData->PointerEnter))
	{
		for (int32 Index = 0, Count = CurrentPointerData->Hovered.Num(); Index < Count; ++Index)
		{
			FExecuteEvents::Execute<IPointerExitHandlerInterface>(CurrentPointerData->Hovered[Index], CurrentPointerData);
		}
		
		CurrentPointerData->Hovered.Empty();

		if (!IsValid(NewEnterTarget))
		{
			CurrentPointerData->PointerEnter = nullptr;
			return;
		}
	}

	// if we have not changed hover target
	if (CurrentPointerData->PointerEnter == NewEnterTarget && IsValid(NewEnterTarget))
		return;

	const USceneComponent* CommonRoot = FindCommonRoot(CurrentPointerData->PointerEnter, NewEnterTarget);

	// and we already an entered object from last time
	if (IsValid(CurrentPointerData->PointerEnter))
	{
		// send exit handler call to all elements in the chain
		// until we reach the new target, or null!
		USceneComponent* S = CurrentPointerData->PointerEnter;

		while (IsValid(S))
		{
			// if we reach the common root break out!
			if (IsValid(CommonRoot) && CommonRoot == S)
				break;

			FExecuteEvents::Execute<IPointerExitHandlerInterface>(S, CurrentPointerData);
			CurrentPointerData->Hovered.Remove(S);
			S = S->GetAttachParent();
		}
	}

	// now issue the enter call up to but not including the common root
	CurrentPointerData->PointerEnter = NewEnterTarget;
	if (IsValid(NewEnterTarget))
	{
		USceneComponent* S = NewEnterTarget;
		
		while (IsValid(S) && S != CommonRoot)
		{
			FExecuteEvents::Execute<IPointerEnterHandlerInterface>(S, CurrentPointerData);
			CurrentPointerData->Hovered.Add(S);
			S = S->GetAttachParent();
		}
	}
}

/////////////////////////////////////////////////////
