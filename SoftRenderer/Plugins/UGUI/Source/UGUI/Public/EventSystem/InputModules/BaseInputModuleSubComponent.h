#pragma once

#include "BaseInput.h"
#include "CoreMinimal.h"
#include "Core/BehaviourSubComponent.h"
#include "EventSystem/RaycastResult.h"
#include "EventSystem/EventData/AxisEventData.h"
#include "BaseInputModuleSubComponent.generated.h"

class UPointerEventData;
class UEventSystemComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UBaseInputModuleSubComponent : public UBehaviourSubComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	UEventSystemComponent* EventSystem;

	UPROPERTY(Transient)
	UObject* InputOverride;

	UPROPERTY(Transient)
	UObject* DefaultInput;

	UPROPERTY(Transient)
	UAxisEventData* AxisEventData;
	
	UPROPERTY(Transient)
	UBaseEventData* BaseEventData;

	UPROPERTY(Transient)
	TScriptInterface<IEventViewportClientInterface> ViewportClient;
	
protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourSubComponent Interface.

public:
	UFUNCTION(BlueprintCallable, Category = BaseInputModule)
	UEventSystemComponent* GetEventSystem() const
	{
		return EventSystem;
	}

	UFUNCTION(BlueprintCallable, Category = BaseInputModule)
	UObject* GetInputOverride() const
	{
		return InputOverride;
	}

	/**
	 * Used to override the default BaseInput for the input module.
	 *
	 * With this it is possible to bypass the Input system with your own but still use the same InputModule. For example this can be used to feed fake input into the UI or interface with a different input system.
	 */
	UFUNCTION(BlueprintCallable, Category = BaseInputModule)
	void SetInputOverride(UObject* InInputOverride)
	{
		InputOverride = InInputOverride;
	}

	IUGUIInputInterface* GetInput();

protected:
	/**
	 * Given some input data generate an AxisEventData that can be used by the event system.
	 */
	virtual UAxisEventData* GetAxisEventData(float X, float Y, float MoveDeadZone)
	{
		if (AxisEventData == nullptr)
		{
			AxisEventData = NewObject<UAxisEventData>(this);
			AxisEventData->SetEventSystem(EventSystem);
		}

		AxisEventData->Reset();
		AxisEventData->MoveVector.X = X;
		AxisEventData->MoveVector.Y = Y;
		AxisEventData->MoveDir = DetermineMoveDirection(X, Y, MoveDeadZone);
		return AxisEventData;
	}
	
	/**
	 * Generate a BaseEventData that can be used by the EventSystem.
	 */
	virtual UBaseEventData* GetBaseEventData()
	{
		if (BaseEventData == nullptr)
		{
			BaseEventData = NewObject<UBaseEventData>(this);
			BaseEventData->SetEventSystem(EventSystem);
		}

		BaseEventData->Reset();
		return BaseEventData;
	}
	
public:
	/**
	 * Process the current tick for the module.
	 */
	virtual void Process() {};

protected:
	/**
	 * Given an input movement, determine the best MoveDirection.
	 */
	static EMoveDirection DetermineMoveDirection(float X, float Y);

	/**
	 * Given an input movement, determine the best MoveDirection.
	 */
	static EMoveDirection DetermineMoveDirection(float X, float Y, float DeadZone);

	/**
	 * Given 2 GameObjects, return a common root GameObject (or null).
	 */
	static USceneComponent* FindCommonRoot(USceneComponent* SceneComp1, const USceneComponent* SceneComp2);

protected:
	/**
	 * walk up the tree till a common root between the last entered and the current entered is found
	 * send exit events up to (but not including) the common root. Then send enter events up to
	 * (but not including the common root).
	 */
	void HandlePointerExitAndEnter(UPointerEventData* CurrentPointerData, USceneComponent* NewEnterTarget) const;
	
public:
	/**
	 * If the module is pointer based, then override this to return true if the pointer is over an event system object.
	 */
	virtual bool IsPointerOverGameObject(int32 PointerId)
	{
		return false;
	}
	
	/**
	 *  Should the module be activated.
	 */
	virtual bool ShouldActivateModule()
	{
		return IsEnabledInHierarchy();
	}
	
	/**
	 * Called when the module is deactivated. Override this if you want custom code to execute when you deactivate your module.
	 */
	virtual void DeactivateModule() {};

	/**
	 * Called when the module is activated. Override this if you want custom code to execute when you activate your module.
	 */
	virtual void ActivateModule() {};

	/**
	 * Update the internal state of the Module.
	 */
	virtual void UpdateModule() {};

	/**
	 * Check to see if the module is supported. Override this if you have a platform specific module (eg. TouchInputModule that you do not want to activate on standalone.)
	 */
	virtual bool IsModuleSupported()
	{
		return true;
	}

public:
	virtual FString ToString() const { return FString(); };
	virtual void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const {}
	
};
