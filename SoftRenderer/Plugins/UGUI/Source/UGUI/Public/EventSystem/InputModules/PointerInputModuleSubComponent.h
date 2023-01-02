#pragma once

#include "CoreMinimal.h"
#include "BaseInputModuleSubComponent.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "PointerInputModuleSubComponent.generated.h"

USTRUCT()
struct FPIMMouseButtonEventData
{
    GENERATED_BODY()

public:
	/**
	 * The state of the button this frame.
	 */
    EPointerFramePressState ButtonState;

	/**
	 * Pointer data associated with the mouse event.
	 */
	UPROPERTY()
	UPointerEventData* ButtonData;

public:
	/**
	 * Was the button pressed this frame?
	 */
	bool PressedThisFrame() const
	{
		return ButtonState == EPointerFramePressState::FramePressState_Pressed || ButtonState == EPointerFramePressState::FramePressState_PressedAndReleased;
	}

	/**
	 * Was the button released this frame?
	 */
	bool ReleasedThisFrame() const
	{
		return ButtonState == EPointerFramePressState::FramePressState_Released || ButtonState == EPointerFramePressState::FramePressState_PressedAndReleased;
	}
	
};

USTRUCT()
struct FPIMButtonState
{
    GENERATED_BODY()

public:
	EPointerInputButton Button;

	UPROPERTY()
	FPIMMouseButtonEventData EventData;

public:
	FPIMButtonState(): Button(), EventData()
    {
    }

    FPIMButtonState(EPointerInputButton InButton)
		: Button(InButton)
		, EventData()
    {

    }
};

USTRUCT()
struct FPIMMouseState
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<FPIMButtonState> TrackedButtons;
	
public:
	FPIMButtonState& GetButtonState(EPointerInputButton Button)
	{
		for (int32 Index = 0, Count = TrackedButtons.Num(); Index < Count; ++Index)
		{
			if (TrackedButtons[Index].Button == Button)
			{
				return TrackedButtons[Index];
			}
		}

		TrackedButtons.Emplace(FPIMButtonState(Button));
		return TrackedButtons[TrackedButtons.Num() - 1];
	}

	void SetButtonState(EPointerInputButton Button, EPointerFramePressState StateForMouseButton, UPointerEventData* Data)
	{
		auto& ToModify = GetButtonState(Button);
		ToModify.EventData.ButtonState = StateForMouseButton;
		ToModify.EventData.ButtonData = Data;
	}
	
};

/**
 * A BaseInputModule for pointer input.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UPointerInputModuleSubComponent : public UBaseInputModuleSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * Id of the cached left mouse pointer event.
	 */
    static int32 KMouseLeftId;

    /**
     * Id of the cached right mouse pointer event.
     */
    static int32 KMouseRightId;

    /**
     * Id of the cached middle mouse pointer event.
     */
    static int32 KMouseMiddleId;

    /**
     * Touch id for when simulating touches on a non touch device.
     */
    static int32 KFakeTouchesId;

protected:
	UPROPERTY(Transient)
    TMap<int32, UPointerEventData*> PointerData;

private:
	UPROPERTY(Transient)
	FPIMMouseState MouseState;
	
protected:
	/**
	 * Search the cache for currently active pointers, return true if found.
	 */
    bool GetPointerData(int32 Id, UPointerEventData*& Data, bool bCreate);

    /**
     * Remove the PointerEventData from the cache.
     */
    void RemovePointerData(const UPointerEventData* Data);

	/**
	 * Given a touch populate the PointerEventData and return if we are pressed or released.
	 */
    UPointerEventData* GetTouchPointerEventData(FViewportTouchState* Input, bool& bPressed, bool& bReleased, bool& bMoved);

	/**
	 * Copy one PointerEventData to another.
	 */
	static void CopyFromTo(const UPointerEventData* From, UPointerEventData* To);

    EPointerFramePressState StateForMouseButton(int32 ButtonId);

	/**
	 * Return the current MouseState. Using the default pointer.
	 */
	virtual FPIMMouseState& GetMousePointerEventData()
	{
		return GetMousePointerEventData(0);
	}

	/**
	 * Return the current MouseState.
	 */
	virtual FPIMMouseState& GetMousePointerEventData(int32 Id);

	/**
	 * Return the last PointerEventData for the given touch / mouse id.
	 */
	UPointerEventData* GetLastPointerEventData(int32 Id);

private:
	static bool ShouldStartDrag(const FVector& PressPos, const FVector& CurrentPos, float Threshold, bool bUseDragThreshold);

protected:
	/**
	 * Process movement for the current frame with the given pointer event.
	 */
	virtual void ProcessMove(UPointerEventData* PointerEvent);

	/**
	 * Process the drag for the current frame with the given pointer event.
	 */
	virtual void ProcessDrag(UPointerEventData* PointerEvent);

public:
	virtual bool IsPointerOverGameObject(int32 PointerId) override;

protected:
	/**
	 * Clear all pointers and deselect any selected objects in the EventSystem.
	 */
	void ClearSelection();

	/**
	 * Deselect the current selected GameObject if the currently pointed-at GameObject is different.
	 */
	void DeselectIfSelectionChanged(USceneComponent* CurrentOverGo, UBaseEventData* PointerEvent) const;

public:
	virtual FString ToString() const override;
	virtual void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const override;
	
};
