#pragma once

#include "CoreMinimal.h"
#include "BaseEventData.h"
#include "EventSystem/RaycastResult.h"
#include "PointerEventData.generated.h"

/**
 * Input press tracking.
 */
UENUM(BlueprintType)
enum class EPointerInputButton : uint8
{
    /**
     * Left button
     */
	InputButton_Left UMETA(DisplayName = "Left"),

    /**
     * Right button
     */
	InputButton_Right UMETA(DisplayName = "Right"),

    /**
     * Middle button
     */
	InputButton_Middle UMETA(DisplayName = "Middle"),
};

/**
 * The state of a press for the given frame.
 */
UENUM(BlueprintType)
enum class EPointerFramePressState : uint8
{
    /**
     * Button was pressed this frame.
     */
    FramePressState_Pressed UMETA(DisplayName = "Pressed"),

    /**
     * Button was released this frame.
     */
    FramePressState_Released UMETA(DisplayName = "Released"),

    /**
     * Button was pressed and released this frame.
     */
    FramePressState_PressedAndReleased  UMETA(DisplayName = "PressedAndReleased"),

	/**
	 * Same as last frame.
	 */
    FramePressState_NotChanged UMETA(DisplayName = "NotChanged"),
};

/**
 * Each touch event creates one of these containing all the relevant information.
 */
UCLASS(BlueprintType)
class UGUI_API UPointerEventData : public UBaseEventData
{
	GENERATED_UCLASS_BODY()

protected:
    /**
     * The object that received OnPointerDown
     */
    UPROPERTY(Transient)
    USceneComponent* PointerPress;

    /**
     * The raw GameObject for the last press event. This means that it is the 'pressed' GameObject even if it can not receive the press event itself.
     */
    UPROPERTY(Transient)
    USceneComponent* LastPress;
	
public:
	/**
	 * The object that received 'OnPointerEnter'.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    USceneComponent* PointerEnter;

    /**
     * The object that the press happened on even if it can not handle the press event.
     */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    USceneComponent* RawPointerPress;

    /**
     * The object that is receiving 'OnDrag'.
     */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    USceneComponent* PointerDrag;

	/**
	 * RaycastResult associated with the current event.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    FRaycastResult PointerCurrentRaycast;

    /**
     * RaycastResult associated with the pointer press.
     */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    FRaycastResult PointerPressRaycast;

    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    TArray<USceneComponent*> Hovered;

    /**
     * Id of the pointer (touch id).
     */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    int32 PointerId;

	/**
	 * Current pointer position.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    FVector Position;

	/**
	 * Pointer delta since last update.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    FVector Delta;

	/**
	 * Position of the press.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    FVector PressPosition;

	/**
	 * The last time a click event was sent. Used for double click
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    float ClickTime;

	/**
	 * Number of clicks in a row.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    int32 ClickCount;

	/**
	 * The amount of scroll since the last update.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    float ScrollDelta;

	/**
	 * The EventSystems.PointerEventData.InputButton for this event.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    EPointerInputButton Button;

	/**
	 * Is it possible to click this frame
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    uint8 bEligibleForClick : 1;

	/**
	 * Should a drag threshold be used?
	 *
	 * If you do not want a drag threshold set this to false in IInitializePotentialDragHandler.OnInitializePotentialDrag.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    uint8 bUseDragThreshold : 1;

	/**
	 * Is a drag operation currently occuring.
	 */
    UPROPERTY(Transient, BlueprintReadOnly, Category = PointerEventData)
    uint8 bDragging : 1;

public:
    /**
     * Is the pointer moving.
     */
	UFUNCTION(BlueprintCallable, Category = PointerEventData)
    bool IsPointerMoving() const
	{
        return FVector2D(Delta).SizeSquared() > 0.0f;
	}

	/**
	 * Is scroll being used on the input device.
	 */
    UFUNCTION(BlueprintCallable, Category = PointerEventData)
    bool IsScrolling() const
    {
        return ScrollDelta * ScrollDelta > 0.0f;
    }

    UFUNCTION(BlueprintCallable, Category = PointerEventData)
    USceneComponent* GetLastPointerPress() const
    {
        return LastPress;
    }
	
	/**
	 * The GameObject that received the OnPointerDown.
	 */
    UFUNCTION(BlueprintCallable, Category = PointerEventData)
    USceneComponent* GetPointerPress() const
	{
        return PointerPress;
	}

    UFUNCTION(BlueprintCallable, Category = PointerEventData)
    void SetPointerPress(USceneComponent* InPointerPress)
	{
        if (PointerPress == InPointerPress)
            return;

        LastPress = PointerPress;
        PointerPress = InPointerPress;
	}

    virtual FString ToString() const;
	virtual void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const;
	
};
