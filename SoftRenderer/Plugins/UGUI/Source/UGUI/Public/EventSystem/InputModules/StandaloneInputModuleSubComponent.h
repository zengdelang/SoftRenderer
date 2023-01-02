#pragma once

#include "CoreMinimal.h"
#include "PointerInputModuleSubComponent.h"
#include "StandaloneInputModuleSubComponent.generated.h"

/**
 * A BaseInputModule designed for mouse / keyboard / controller input.
 *
 * Input module for working with, mouse, keyboard, or controller.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Standalone Input Module", RequireSubClasses = "EventSystemSubComponent"))
class UGUI_API UStandaloneInputModuleSubComponent : public UPointerInputModuleSubComponent
{
	GENERATED_UCLASS_BODY()

private:
	float PrevActionTime;
	FVector2D LastMoveVector;
	int32 ConsecutiveMoveCount = 0;

	FVector2D LastMousePosition;
	FVector2D MousePosition;;

	UPROPERTY(Transient)
	USceneComponent* CurrentFocusedGameObject;

	UPROPERTY(Transient)
	UPointerEventData* InputPointerEvent;

public:
	/**
	 * Name of the horizontal axis for movement (if axis events are used).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName HorizontalAxis;

	/**
	 * Name of the vertical axis for movement (if axis events are used).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName VerticalAxis;

	/**
	 * Name of the submit button.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName SubmitButton;

	/**
	 * Name of the submit button.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	FName CancelButton;

	/**
	 * Number of keyboard / controller inputs allowed per second.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float InputActionsPerSecond;

	/**
	 * Delay in seconds before the input actions per second repeat rate takes effect.
	 *
	 * If the same direction is sustained, the inputActionsPerSecond property can be used to control the rate at which events are fired. However, it can be desirable that the first repetition is delayed, so the user doesn't get repeated actions by accident.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	float RepeatDelay;

	/**
	 * Force this module to be active.
	 *
	 * If there is no module active with higher priority (ordered in the inspector) this module will be forced active even if valid enabling conditions are not met.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Module)
	uint8 bForceModuleActive : 1;

private:
	static bool ShouldIgnoreEventsOnNoFocus();

public:
	virtual void UpdateModule() override;
	
private:
	void ReleaseMouse(UPointerEventData* PointerEvent, USceneComponent* CurrentOverGo);

public:
	virtual bool IsModuleSupported() override;
	virtual bool ShouldActivateModule() override;
	virtual void ActivateModule() override;
	virtual void DeactivateModule() override;
	
public:
	//~ Begin UBaseInputModule Interface
	virtual void Process() override;
	//~ End UBaseInputModule Interface.
	
private:
	bool ProcessTouchEvents();

protected:
	/**
	 * This method is called by Unity whenever a touch event is processed. Override this method with a custom implementation to process touch events yourself.
	 *
	 * This method can be overridden in derived classes to change how touch press events are handled.
	 */
	void ProcessTouchPress(UPointerEventData* PointerEvent, bool bPressed, bool bReleased, bool bMoved);

	/**
	 * Calculate and send a submit event to the current selected object.
	 *
	 * If the submit event was used by the selected object.
	 */
	bool SendSubmitEventToSelectedObject();

private:
	FVector2D GetRawMoveVector();
	
protected:
	/**
	 * Calculate and send a move event to the current selected object.
	 */
	bool SendMoveEventToSelectedObject();

	void ProcessMouseEvent()
	{
		ProcessMouseEvent(0);
	}

	/**
	 * Process all mouse events.
	 */
	void ProcessMouseEvent(int32 Id);

	bool SendUpdateEventToSelectedObject();

	void ProcessMousePress(const FPIMMouseButtonEventData& Data);
	
};
