#pragma once

#include "CoreMinimal.h"
#include "SelectableComponent.h"
#include "EventSystem/Interfaces/BeginDragHandlerInterface.h"
#include "EventSystem/Interfaces/DragHandlerInterface.h"
#include "EventSystem/Interfaces/InitializePotentialDragHandlerInterface.h"
#include "ScrollbarComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollBarValueChangedEvent, float, InValue);

/**
 * Setting that indicates one of four directions the scrollbar will travel.
 */
UENUM(BlueprintType)
enum class EScrollBarDirection : uint8
{
	/**
	 * Starting position is the Left.
	 */
	Direction_LeftToRight UMETA(DisplayName = "LeftToRight"),

	/**
	 * Starting position is the Right
	 */
	Direction_RightToLeft UMETA(DisplayName = "RightToLeft"),

	/**
	 * Starting position is the Bottom.
	 */
	Direction_BottomToTop UMETA(DisplayName = "BottomToTop"),

	/**
	 * Starting position is the Top.
	 */
	Direction_TopToBottom UMETA(DisplayName = "TopToBottom"),
};

UENUM(BlueprintType)
enum class EScrollBarAxis : uint8
{
	Axis_Horizontal UMETA(DisplayName = "Horizontal"),

	Axis_Vertical UMETA(DisplayName = "Vertical"),
};

/**
 * A standard scrollbar with a variable sized handle that can be dragged between 0 and 1.
 *
 * The ScrollBar component is a Selectable that controls a handle which follow the current value and is sized according to the size property.
 * The anchors of the handle RectTransforms are driven by the Scrollbar. The handle can be a direct child of the GameObject with the Scrollbar, or intermediary RectTransforms can be placed in between for additional control.
 * When a change to the scrollbar value occurs, a callback is sent to any registered listeners of OnValueChanged.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent, DisplayName = ScrollBar))
class UGUI_API UScrollbarComponent : public USelectableComponent, public IBeginDragHandlerInterface, public IDragHandlerInterface, public IInitializePotentialDragHandlerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * The direction of the scrollbar from minimum to maximum value.
	 */
	UPROPERTY(EditAnywhere, Category = Scrollbar)
	EScrollBarDirection Direction;

	/**
	 * The current value of the scrollbar, between 0 and 1.
	 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = Scrollbar)
	float Value;

	/**
	 * The size of the scrollbar handle where 1 means it fills the entire scrollbar.
	 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = Scrollbar)
	float Size;

	/**
	 * The number of steps to use for the value. A value of 0 disables use of steps.
	 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "11", UIMin = "0", UIMax = "11"), Category = Scrollbar)
	int32 NumberOfSteps;

	UPROPERTY(EditAnywhere, Category = Scrollbar)
	TArray<int32> HandleRectPath;
	
public:
	/**
	 * Handling for when the scrollbar value is changed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnScrollBarValueChangedEvent OnValueChanged;

protected:
	/**
	 * The RectTransform to use for the handle.
	 */
	UPROPERTY(Transient)
	URectTransformComponent* HandleRect;

private:
	UPROPERTY(Transient)
	URectTransformComponent* ContainerRect;

	/**
	 * The offset from handle position to mouse down position
	 */
	FVector2D Offset;

	FTimerHandle TimerHandle;
	
	uint8 bIsPointerDownAndNotDragging : 1;
	uint8 bDelayedUpdateVisuals : 1;
	uint8 bNeedSetTimer : 1;
	
public:
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnRectTransformDimensionsChange() override;
	//~ End BehaviourComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	void DelayUpdateVisuals();
	
public:
	/**
	 * Handling for when the scrollbar value is begin being dragged.
	 */
	virtual void OnBeginDrag(UPointerEventData* EventData) override;

	/**
	 * Handling for when the scrollbar value is dragged.
	 */
	virtual void OnDrag(UPointerEventData* EventData) override;
	virtual void OnInitializePotentialDrag(UPointerEventData* EventData) override;

	/**
	 * Event triggered when pointer is pressed down on the scrollbar.
	 */
	virtual void OnPointerDown(UPointerEventData* EventData) override;

	/**
	 * Event triggered when pointer is released after pressing on the scrollbar.
	 */
	virtual void OnPointerUp(UPointerEventData* EventData) override;
	
protected:
	EScrollBarAxis GetAxis() const
	{
		return (Direction == EScrollBarDirection::Direction_LeftToRight || Direction == EScrollBarDirection::Direction_RightToLeft) ? EScrollBarAxis::Axis_Horizontal : EScrollBarAxis::Axis_Vertical;
	}

	bool ReverseValue() const
	{
		return Direction == EScrollBarDirection::Direction_RightToLeft || Direction == EScrollBarDirection::Direction_TopToBottom;
	}
	
protected:
	/**
	 * Size of each step.
	 */
	float GetStepSize() const
	{
		return (NumberOfSteps > 1) ? 1.0f / (NumberOfSteps - 1) : 0.1f;
	}
	
public:
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	EScrollBarDirection GetDirection() const
	{
		return Direction;
	}

	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetDirection(EScrollBarDirection InDirection)
	{
		if (Direction != InDirection)
		{
			Direction = InDirection;
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	float GetValue() const
	{
		float Val = Value;
		if (NumberOfSteps > 1)
			Val = FMath::RoundToFloat(Val * (NumberOfSteps - 1)) / (NumberOfSteps - 1);
		return Val;
	}

	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetValue(float InValue)
	{
		Set(InValue);
	}

public:
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	float GetSize() const
	{	 
		return Size;
	}

	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetSize(float InSize)
	{
		InSize = FMath::Clamp(InSize, 0.0f, 1.0f);
		if (Size != InSize)
		{
			Size = InSize;
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	int32 GetNumberOfSteps() const
	{
		return NumberOfSteps;
	}

	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetNumberOfSteps(int32 InNumberOfSteps)
	{ 
		if (NumberOfSteps != InNumberOfSteps)
		{
			NumberOfSteps = InNumberOfSteps;
			Set(Value);
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	URectTransformComponent* GetHandleRect() const
	{
		return HandleRect;
	}

	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetHandleRect(URectTransformComponent* InHandleRect)
	{
		if (HandleRect != InHandleRect)
		{
			HandleRect = InHandleRect;

			if (IsValid(HandleRect))
			{
				TargetGraphic = HandleRect;
			}

			UpdateCachedReferences();
			UpdateVisuals();
		}
	}
	
public:
	/**
	 * Set the value of the scrollbar without invoking OnValueChanged callback.
	 *
	 * @param  Input  The new value for the scrollbar.
	 */
	UFUNCTION(BlueprintCallable, Category = Scrollbar)
	void SetValueWithoutNotify(float Input)
	{
		Set(Input, false);
	}

protected:
	void UpdateCachedReferences();
	
	void Set(float Input, bool bSendCallback = true);

	/**
	 * Force-update the scroll bar. Useful if you've changed the properties and want it to update visually.
	 */
	void UpdateVisuals();

private:
	/**
	 * Update the scroll bar's position based on the mouse.
	 */
	void UpdateDrag(const UPointerEventData* EventData);

	void DoUpdateDrag(FVector2D HandleCorner, float RemainingSize);

	bool MayDrag(const UPointerEventData* EventData) const;

	void ClickRepeat(TWeakObjectPtr<UPointerEventData> EventData);

};
