#pragma once

#include "CoreMinimal.h"
#include "ImageElementInterface.h"
#include "SelectableComponent.h"
#include "Core/CanvasElementInterface.h"
#include "Core/MathUtility.h"
#include "EventSystem/Interfaces/DragHandlerInterface.h"
#include "EventSystem/Interfaces/InitializePotentialDragHandlerInterface.h"
#include "SliderComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSliderValueChangedEvent, float, InValue);

/**
 * Setting that indicates one of four directions.
 */
UENUM(BlueprintType)
enum class ESliderDirection : uint8
{
	/**
	 * From the left to the right
	 */
	SliderDirection_LeftToRight UMETA(DisplayName = "LeftToRight"),

	/**
	 * From the right to the left
	 */
	SliderDirection_RightToLeft UMETA(DisplayName = "RightToLeft"),

	/**
	 * From the top to the bottom.
	 */
	SliderDirection_BottomToTop UMETA(DisplayName = "BottomToTop"),

	/**
	 * Starting position is the Top.
	 */
	SliderDirection_TopToBottom UMETA(DisplayName = "TopToBottom"),
};

UENUM(BlueprintType)
enum class ESliderAxis : uint8
{
	SliderAxis_Horizontal UMETA(DisplayName = "Horizontal"),

	SliderAxis_Vertical UMETA(DisplayName = "Vertical"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderComponentPressedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSliderComponentReleasedEvent);

/**
 * A standard slider that can be moved between a minimum and maximum value.
 *
 * The slider component is a Selectable that controls a fill, a handle, or both. The fill, when used, spans from the minimum value to the current value while the handle, when used, follow the current value.
 * The anchors of the fill and handle RectTransforms are driven by the Slider. The fill and handle can be direct children of the GameObject with the Slider, or intermediary RectTransforms can be placed in between for additional control.
 * When a change to the slider value occurs, a callback is sent to any registered listeners of UI.Slider.OnValueChanged.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API USliderComponent : public USelectableComponent, public IDragHandlerInterface, public IInitializePotentialDragHandlerInterface, public ICanvasElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * The direction of the slider from minimum to maximum value.
	 */
	UPROPERTY(EditAnywhere, Category = Slider)
	ESliderDirection Direction;

	/**
	 * The minimum allowed value of the slider.
	 */
	UPROPERTY(EditAnywhere, Category = Slider)
	float MinValue;

	/**
	 * The maximum allowed value of the slider.
	 */
	UPROPERTY(EditAnywhere, Category = Slider)
	float MaxValue;

	/**
	 * The current value of the slider.
	 */
	UPROPERTY(EditAnywhere, Category = Slider)
	float Value;

	UPROPERTY(EditAnywhere, Category = Slider)
	TArray<int32> HandleRectPath;

	UPROPERTY(EditAnywhere, Category = Slider)
	TArray<int32> FillRectPath;
	
public:
	/**
	 * Callback executed when the value of the slider is changed.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnSliderValueChangedEvent OnValueChanged;

	/** Called when the slider is pressed */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnSliderComponentPressedEvent OnPressed;

	/** Called when the slider is released */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnSliderComponentReleasedEvent OnReleased;
	
protected:
	/**
	 * Optional RectTransform to use as a handle for the slider.
	 */
	UPROPERTY(Transient)
	URectTransformComponent* HandleRect;

	/**
	 * Optional RectTransform to use as fill for the slider.
	 */
	UPROPERTY(Transient)
	URectTransformComponent* FillRect;

private:
	UPROPERTY(Transient)
	TScriptInterface<IImageElementInterface> FillImage;

	UPROPERTY(Transient)
	URectTransformComponent* FillContainerRect;

	UPROPERTY(Transient)
	URectTransformComponent* HandleContainerRect;
	
private:
	/**
	 * The offset from handle position to mouse down position
	 */
	FVector2D Offset;

protected:
	/**
	 * Should the value only be allowed to be whole numbers?
	 */
	UPROPERTY(EditAnywhere, Category = Slider)
	uint8 bWholeNumbers : 1;
	
private:
	uint8 bDelayedUpdateVisuals : 1;

public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return this; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface
	
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
	 * Handling for when the slider value is dragged.
	 */
	virtual void OnDrag(UPointerEventData* EventData) override;
	virtual void OnInitializePotentialDrag(UPointerEventData* EventData) override;

	/**
	 * Event triggered when pointer is pressed down on the slider.
	 */
	virtual void OnPointerDown(UPointerEventData* EventData) override;
	virtual void OnPointerUp(UPointerEventData* EventData) override;
	
protected:
	ESliderAxis GetAxis() const
	{
		return (Direction == ESliderDirection::SliderDirection_LeftToRight || Direction == ESliderDirection::SliderDirection_RightToLeft) ? ESliderAxis::SliderAxis_Horizontal : ESliderAxis::SliderAxis_Vertical;
	}

	bool ReverseValue() const
	{
		return Direction == ESliderDirection::SliderDirection_RightToLeft || Direction == ESliderDirection::SliderDirection_TopToBottom;
	}
	
protected:
	/**
	 * Size of each step.
	 */
	float GetStepSize() const
	{
		return bWholeNumbers ? 1 : (MaxValue - MinValue) * 0.1f;
	}
	
public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	ESliderDirection GetDirection() const
	{
		return Direction;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetDirection(ESliderDirection InDirection)
	{
		if (Direction != InDirection)
		{
			Direction = InDirection;
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	float GetMinValue() const
	{
		return MinValue;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetMinValue(float InMinValue)
	{
		if (MinValue != InMinValue)
		{
			MinValue = InMinValue;
			Set(GetValue());
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	float GetMaxValue() const
	{
		return MaxValue;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetMaxValue(float InMaxValue)
	{
		if (MaxValue != InMaxValue)
		{
			MaxValue = InMaxValue;
			Set(GetValue());
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	float GetValue() const
	{
#if WITH_EDITOR
		if (bWholeNumbers)
			return FMath::RoundToFloat(ClampValue(Value));
		return ClampValue(Value);
#else
		if (bWholeNumbers)
			return FMath::RoundToFloat(Value);
		return Value;
#endif
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetValue(float InValue)
	{
		Set(InValue);
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	bool IsWholeNumbers() const
	{
		return bWholeNumbers;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetWholeNumbers(bool bInWholeNumbers)
	{
		if (bWholeNumbers != bInWholeNumbers)
		{
			bWholeNumbers = bInWholeNumbers;
			Set(GetValue());
			UpdateVisuals();
		}
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	float GetNormalizedValue() const
	{
		if (FMathUtility::Approximately(MinValue, MaxValue))
			return 0;
		return FMath::Clamp((GetValue() - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetNormalizedValue(float InValue)
	{
		SetValue(FMath::Lerp(MinValue, MaxValue, InValue));
	}

public:
	UFUNCTION(BlueprintCallable, Category = Slider)
	URectTransformComponent* GetHandleRect() const
	{
		return HandleRect;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
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
	UFUNCTION(BlueprintCallable, Category = Slider)
	URectTransformComponent* GetFillRect() const
	{
		return FillRect;
	}

	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetFillRect(URectTransformComponent* InFillRect)
	{
		if (FillRect != InFillRect)
		{
			FillRect = InFillRect;
			UpdateCachedReferences();
			UpdateVisuals();
		}
	}
	
public:
	/**
	 * Set the value of the slider without invoking OnValueChanged callback.
	 *
	 * @param  Input  The new value for the slider.
	 */
	UFUNCTION(BlueprintCallable, Category = Slider)
	void SetValueWithoutNotify(float Input)
	{
		Set(Input, false);
	}

protected:
	void UpdateCachedReferences();

	float ClampValue(float Input) const;

	/**
	 * Set the value of the slider.
	 *
	 * Process the input to ensure the value is between min and max value. If the input is different set the value and send the callback is required.
	 *
	 * @param  Input  The new value for the slider.
	 * @param  bSendCallback  If the OnValueChanged callback should be invoked.
	 */
	void Set(float Input, bool bSendCallback = true);

	/**
	 * Force-update the slider. Useful if you've changed the properties and want it to update visually.
	 */
	void UpdateVisuals();

private:
	/**
	 * Update the slider's position based on the mouse.
	 */
	void UpdateDrag(const UPointerEventData* EventData);

	bool MayDrag(const UPointerEventData* EventData) const;

};
