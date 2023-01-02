#pragma once

#include "CoreMinimal.h"
#include "EventData/AxisEventData.h"
#include "EventData/PointerEventData.h"
#include "Interfaces/CancelHandlerInterface.h"
#include "Interfaces/DeselectHandlerInterface.h"
#include "Interfaces/DropHandlerInterface.h"
#include "Interfaces/MoveHandlerInterface.h"
#include "Interfaces/ScrollHandlerInterface.h"
#include "Interfaces/SelectHandlerInterface.h"
#include "Interfaces/SubmitHandlerInterface.h"
#include "Interfaces/UpdateSelectedHandlerInterface.h"
#include "Interfaces/PointerEnterHandlerInterface.h"
#include "Interfaces/PointerExitHandlerInterface.h"
#include "Interfaces/PointerDownHandlerInterface.h"
#include "Interfaces/PointerUpHandlerInterface.h"
#include "Interfaces/PointerClickHandlerInterface.h"
#include "Interfaces/InitializePotentialDragHandlerInterface.h"
#include "Interfaces/BeginDragHandlerInterface.h"
#include "Interfaces/DragHandlerInterface.h"
#include "Interfaces/EndDragHandlerInterface.h"
#include "UGUI.h"

class UBaseEventData;

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnPointerEnter"), STAT_UnrealGUI_Execute_OnPointerEnter, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnPointerExit"), STAT_UnrealGUI_Execute_OnPointerExit, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnPointerDown"), STAT_UnrealGUI_Execute_OnPointerDown, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnPointerUp"), STAT_UnrealGUI_Execute_OnPointerUp, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnPointerClick"), STAT_UnrealGUI_Execute_OnPointerClick, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnInitializePotentialDrag"), STAT_UnrealGUI_Execute_OnInitializePotentialDrag, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnBeginDrag"), STAT_UnrealGUI_Execute_OnBeginDrag, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnDrag"), STAT_UnrealGUI_Execute_OnDrag, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnEndDrag"), STAT_UnrealGUI_Execute_OnEndDrag, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnDrop"), STAT_UnrealGUI_Execute_OnDrop, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnScroll"), STAT_UnrealGUI_Execute_OnScroll, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnUpdateSelected"), STAT_UnrealGUI_Execute_OnUpdateSelected, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnSelect"), STAT_UnrealGUI_Execute_OnSelect, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnDeselect"), STAT_UnrealGUI_Execute_OnDeselect, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnMove"), STAT_UnrealGUI_Execute_OnMove, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnSubmit"), STAT_UnrealGUI_Execute_OnSubmit, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- Execute_OnCancel"), STAT_UnrealGUI_Execute_OnCancel, STATGROUP_UnrealGUI);

class UGUI_API FExecuteEvents
{
private:
	static void Execute(IPointerEnterHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnPointerEnter);
		Handler->OnPointerEnter(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IPointerExitHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnPointerExit);
		Handler->OnPointerExit(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IPointerDownHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnPointerDown);
		Handler->OnPointerDown(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IPointerUpHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnPointerUp);
		Handler->OnPointerUp(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IPointerClickHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnPointerClick);
		Handler->OnPointerClick(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IInitializePotentialDragHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnInitializePotentialDrag);
		Handler->OnInitializePotentialDrag(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IBeginDragHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnBeginDrag);
		Handler->OnBeginDrag(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IDragHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnDrag);
		Handler->OnDrag(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IEndDragHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnEndDrag);
		Handler->OnEndDrag(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IDropHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnDrop);
		Handler->OnDrop(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IScrollHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnScroll);
		Handler->OnScroll(Cast<UPointerEventData>(EventData));
	}

	static void Execute(IUpdateSelectedHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnUpdateSelected);
		Handler->OnUpdateSelected(EventData);
	}
	
	static void Execute(ISelectHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnSelect);
		Handler->OnSelect(EventData);
	}

	static void Execute(IDeselectHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnDeselect);
		Handler->OnDeselect(EventData);
	}

	static void Execute(IMoveHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnMove);
		Handler->OnMove(Cast<UAxisEventData>(EventData));
	}

	static void Execute(ISubmitHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnSubmit);
		Handler->OnSubmit(EventData);
	}

	static void Execute(ICancelHandlerInterface* Handler, UBaseEventData* EventData)
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Execute_OnCancel);
		Handler->OnCancel(EventData);
	}
	
public:
	template<class T>
	static bool Execute(USceneComponent* Target, UBaseEventData* EventData)
	{
		TArray<T*, TInlineAllocator<8>> Components;
		GetEventList<T>(Target, Components);

		for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
		{
			Execute(Components[Index], EventData);
		}
		
		return Components.Num() > 0;
	}

	/**
	 * Execute the specified event on the first game object underneath the current touch.
	 */
	template<class T>
	static USceneComponent* ExecuteHierarchy(USceneComponent* Root, UBaseEventData* EventData)
	{
		TArray<USceneComponent*, TInlineAllocator<16>> Components;
		GetEventChain(Root, Components);

		for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
		{
			const auto SceneComp = Components[Index];
			if (Execute<T>(SceneComp, EventData))
				return SceneComp;
		}

		return nullptr;
	}

	/**
	 * Whether the specified game object will be able to handle the specified event.
	 */
	template<class T>
	static bool CanHandleEvent(USceneComponent* Target)
	{
		TArray<T*, TInlineAllocator<16>> Components;
		GetEventList<T>(Target, Components);
		return Components.Num() != 0;
	}

	/**
	 * Bubble the specified event on the game object, figuring out which object will actually receive the event.
	 */
	template<class T>
	static USceneComponent* GetEventHandler(USceneComponent* Root)
	{
		if (!IsValid(Root))
			return nullptr;

		auto SceneComp = Root;
		while (IsValid(SceneComp))
		{
			if (CanHandleEvent<T>(SceneComp))
				return SceneComp;
			SceneComp = SceneComp->GetAttachParent();
		}
		return nullptr;
	}
	
private:
	static void GetEventChain(USceneComponent* Root, TArray<USceneComponent*, TInlineAllocator<16>>& EventChain)
	{
		EventChain.Empty();
		
		if (!IsValid(Root))
			return;

		auto SceneComp = Root;
		while (IsValid(SceneComp))
		{
			EventChain.Add(SceneComp);
			SceneComp = SceneComp->GetAttachParent();
		}
	}

	/**
	 * Get the specified object's event event.
	 */
	template<class T, class AllocatorType>
	static void GetEventList(USceneComponent* Target, TArray<T*, AllocatorType>& Results)
	{
		if (!IsValid(Target))
			return;

		const auto BehaviourComp = Cast<UBehaviourComponent>(Target);
		if (IsValid(BehaviourComp) && !BehaviourComp->IsActiveAndEnabled())
			return;

		T* Comp = Cast<T>(Target);
		if (Comp)
		{
			Results.Add(Comp);
		}

		if (IsValid(BehaviourComp))
		{
			const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
			for (const auto& SubComponent : AllSubComponents)
			{
				if (IsValid(SubComponent) && SubComponent->IsActiveAndEnabled())
				{
					Comp = Cast<T>(SubComponent);
					if (Comp)
					{
						Results.Add(Comp);
					}
				}
			}
		}
	}
};
