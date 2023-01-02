#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerInput.h"
#include "EventViewportClientInterface.generated.h"

struct UGUI_API FViewportMouseButtonState
{
public:
	uint8 bPress : 1;
	uint8 bPressDownThisFrame : 1;
	uint8 bPressUpThisFrame : 1;

public:
	FViewportMouseButtonState()
		: bPress(false)
		, bPressDownThisFrame(false)
		, bPressUpThisFrame(false)
	{

	}
};

struct UGUI_API FViewportKeyState
{
public:
	uint8 bPress : 1;
	uint8 bPressDownThisFrame : 1;
	uint8 bPressUpThisFrame : 1;

	/** Used to accumulate input values during the frame and flushed after processing. */
	float RawValueAccumulator;
	
public:
	FViewportKeyState()
		: bPress(false)
		, bPressDownThisFrame(false)
		, bPressUpThisFrame(false)
		, RawValueAccumulator(0.f)
	{

	}
};

struct UGUI_API FViewportTouchState
{
protected:
	uint32 FingerId;

	FVector PressedPosition;
	FVector Position;
	FVector Delta;
	
	uint8 bIsPressed : 1;
	uint8 bIsBeganPhase : 1;
	uint8 bIsMovedPhase : 1;
	uint8 bIsEndedPhase : 1;

public:
	FORCEINLINE uint32 GetFingerId() const
	{
		return FingerId;
	}

	FORCEINLINE void SetFingerId(uint32 NewFingerId)
	{
		FingerId = NewFingerId;
	}

	FORCEINLINE bool IsEndedPhase() const
	{
		return bIsEndedPhase;
	}

	FORCEINLINE bool IsMovedPhase() const
	{
		return bIsMovedPhase;
	}

	FORCEINLINE void ResetMoveState()
	{
		if (bIsMovedPhase)
		{
			bIsMovedPhase = false;
			Delta = FVector::ZeroVector;
		}
	}

	FORCEINLINE void MarkAsEndedPhase()
	{
		bIsEndedPhase = true;
	}

	FORCEINLINE const FVector& GetPressedPosition() const 
	{
		return PressedPosition;
	}

	FORCEINLINE const FVector& GetPosition() const
	{
		return Position;
	}

	FORCEINLINE const FVector& GetDelta() const
	{
		return Delta;
	}

	FORCEINLINE void SetPhaseAndPosition(ETouchType::Type Phase, const FVector2D& TouchLocation, float Force)
	{
		if (Phase == ETouchType::Ended)
		{
			if (!bIsBeganPhase)
			{
				Delta = FVector::ZeroVector;
			}
			else
			{
				Delta = FVector(TouchLocation.X, TouchLocation.Y, Force) - Position;
			}
			
			bIsEndedPhase = true;
			Position.X = TouchLocation.X;
			Position.Y = TouchLocation.Y;
			Position.Z = 0;
		}
		else if (Phase == ETouchType::Began)
		{
			bIsBeganPhase = true;
			
			PressedPosition.X = TouchLocation.X;
			PressedPosition.Y = TouchLocation.Y;
			PressedPosition.Z = Force;
			
			Position.X = TouchLocation.X;
			Position.Y = TouchLocation.Y;
			Position.Z = Force;

			Delta = FVector::ZeroVector;
		}
		else
		{
			bIsMovedPhase = true;

			if (!bIsBeganPhase)
			{
				bIsBeganPhase = true;
				PressedPosition.X = TouchLocation.X;
				PressedPosition.Y = TouchLocation.Y;
				PressedPosition.Z = Force;

				Delta = FVector::ZeroVector;
			}
			else
			{
				Delta = FVector(TouchLocation.X, TouchLocation.Y, Force) - Position;
			}

			Position.X = TouchLocation.X;
			Position.Y = TouchLocation.Y;
			Position.Z = Force;
		}
	}

	FORCEINLINE	bool IsPressed()
	{
		if (!bIsPressed)
		{
			return false;
		}

		if (bIsBeganPhase)
		{
			bIsPressed = false;
			return true;
		}
		
		return false;
	}

public:
	FViewportTouchState()
		: FingerId(-1)
		, PressedPosition(0.f, 0.f, 0.f)
		, Position(0.f, 0.f, 0.f)
		, Delta(0.f, 0.f, 0.f)
		, bIsPressed(true)
		, bIsBeganPhase(false)
		, bIsMovedPhase(false)
		, bIsEndedPhase(false)
	{

	}
};

USTRUCT(BlueprintType)
struct FKeyCharacterEvent : public FInputEvent
{
	GENERATED_USTRUCT_BODY()

public:
	FKeyCharacterEvent()
		: FInputEvent(FModifierKeysState(), 0, false)
		, Key()
		, Character(0)
		, bIsCharacterEvent(true)
	{
		
	}

	FKeyCharacterEvent(const FKey InKey,
				const FModifierKeysState& InModifierKeys, 
				const uint32 InUserIndex,
				const bool bInIsRepeat,
				const TCHAR InCharacter,
				const bool bInIsCharacterEvent
	)
		: FInputEvent(InModifierKeys, InUserIndex, bInIsRepeat)
		, Key(InKey)
		, Character(InCharacter)
		, bIsCharacterEvent(bInIsCharacterEvent)
	{
		
	}

	FKey GetKey() const
	{
		return Key;
	}

	TCHAR GetCharacter() const
	{
		return Character;
	}

	bool IsCharacterEvent() const
	{
		return bIsCharacterEvent;
	}

private:
	// Name of the key that was pressed.
	FKey Key;

	// The character that was pressed.
	TCHAR Character;

	uint8 bIsCharacterEvent : 1;
};

UINTERFACE(BlueprintType)
class UGUI_API UEventViewportClientInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IEventViewportClientInterface
{
	GENERATED_BODY()
	
public:
	IEventViewportClientInterface();
	
protected:
	TArray<FViewportMouseButtonState> MouseButtonStates;

	/** The current game view of each key */
	TMap<FKey, FViewportKeyState> KeyStateMap;
	
	TArray<FViewportTouchState> TouchStates;

	TArray<FKeyCharacterEvent> KeyCharacterEvents;

	int32 PopKeyEventIndex;
	
private:
	/** This player's version of the Axis Properties */
	TArray<struct FInputAxisConfigEntry> AxisConfig;

	/** This player's version of the Action Mappings */
	TArray<struct FInputActionKeyMapping> ActionMappings;

	/** This player's version of Axis Mappings */
	TArray<struct FInputAxisKeyMapping> AxisMappings;

	private:	
	/** Runtime struct that caches the list of mappings for a given Action Name and the capturing chord if applicable */
	struct FViewportActionKeyDetails
	{
		/** List of all action key mappings that correspond to the action name in the containing map */
		TArray<FInputActionKeyMapping> Actions;

		/** For paired actions only, this represents the chord that is currently held and when it is released will represent the release event */
		FInputChord CapturingChord;
	};

	/** Runtime struct that caches the list of mappings for a given Axis Name and whether that axis is currently inverted */
	struct FViewportAxisKeyDetails
	{
		/** List of all axis key mappings that correspond to the axis name in the containing map */
		TArray<FInputAxisKeyMapping> KeyMappings;

		/** Whether this axis should invert its outputs */
		uint8 bInverted : 1;

		FViewportAxisKeyDetails()
			: bInverted(false)
		{
		}
	};
	
	/** Internal structure for storing axis config data. */
	TMap<FKey, FInputAxisProperties> AxisProperties;

	/** Map of Action Name to details about the keys mapped to that action */
	TMap<FName, FViewportActionKeyDetails> ActionKeyMap;

	/** Map of Axis Name to details about the keys mapped to that axis */
	TMap<FName, FViewportAxisKeyDetails> AxisKeyMap;

protected:
	float MouseScrollDelta;
	
	uint8 bKeyMapsBuilt : 1;
	uint8 bDirtyMouseBtnStates : 1;
	uint8 bDirtyKeyStates : 1;
	uint8 bDirtyTouchStates : 1;
	uint8 bDirtyMouseScroll : 1;

protected:
	void RebuildingKeyMap();

	/** Utility function to ensure the key mapping cache maps are built */
	FORCEINLINE void ConditionalBuildKeyMappings()
	{
		if (!bKeyMapsBuilt)
		{
			ConditionalBuildKeyMappings_Internal();
		}
	}

	void ConditionalBuildKeyMappings_Internal();
	
protected:
	void InputKeyInternal(const FKey& Key, EInputEvent Event, float AmountDepressed, bool bGamePad);
	void InputAxisInternal(const FKey& Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamePad);
	void InputCharInternal(FViewport* InViewport,int32 ControllerId, TCHAR Character);
	void InputTouchInternal(uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex);

protected:
	void TickInputStates();

public:
	bool PopKeyCharacterEvent(FKeyCharacterEvent& OutKeyCharacterEvent);
	
public:
	virtual void OnLostFocus();
	
	virtual bool HasFocus() = 0;
	
	virtual bool IsCursorVisible() = 0;

	virtual bool HasMouseEvents() { return MouseButtonStates.Num() > 0; }

	virtual FVector2D GetMouseScreenPosition() = 0;
	
public:
	virtual float GetMouseScrollDelta() const
	{
		return MouseScrollDelta;
	}
	
public:
	virtual bool GetMouseButtonDown(int32 Button)
	{
		if (MouseButtonStates.IsValidIndex(Button))
		{
			return MouseButtonStates[Button].bPressDownThisFrame;
		}	
		return false;
	}

	virtual bool GetMouseButtonUp(int32 Button)
	{
		if (MouseButtonStates.IsValidIndex(Button))
		{
			return MouseButtonStates[Button].bPressUpThisFrame;
		}
		return false;
	}

	virtual bool GetMouseButton(int32 Button)
	{
		if (MouseButtonStates.IsValidIndex(Button))
		{
			return MouseButtonStates[Button].bPress;
		}
		return false;
	}
	
public:
	virtual bool GetKeyDown(const FKey& Key)
	{
		const auto KeyStatePtr = KeyStateMap.Find(Key);
		if (KeyStatePtr)
		{
			return KeyStatePtr->bPressDownThisFrame;
		}
		return false;
	}

	virtual bool GetKeyUp(const FKey& Key)
	{
		const auto KeyStatePtr = KeyStateMap.Find(Key);
		if (KeyStatePtr)
		{
			return KeyStatePtr->bPressUpThisFrame;
		}
		return false;
	}

	virtual bool GetKey(const FKey& Key)
	{
		const auto KeyStatePtr = KeyStateMap.Find(Key);
		if (KeyStatePtr)
		{
			return KeyStatePtr->bPress;
		}
		return false;
	}

public:
	virtual int32 TouchCount()
	{
		return TouchStates.Num();
	}

	virtual FViewportTouchState* GetTouch(int32 Index)
	{
		if (TouchStates.IsValidIndex(Index))
		{
			return &TouchStates[Index];
		}
		return nullptr;
	}
	
public:
	virtual bool GetButtonDown(const FName& ActionName)
	{
		ConditionalBuildKeyMappings();
		
		if (const FViewportActionKeyDetails* KeyDetails = ActionKeyMap.Find(ActionName))
		{
			for (const FInputActionKeyMapping& KeyMapping : KeyDetails->Actions)
			{
				if (const FViewportKeyState* KeyState = KeyStateMap.Find(KeyMapping.Key))
				{
					if (KeyState->bPressDownThisFrame)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	virtual bool GetButtonUp(FName ActionName)
	{
		ConditionalBuildKeyMappings();
		
		if (const FViewportActionKeyDetails* KeyDetails = ActionKeyMap.Find(ActionName))
		{
			for (const FInputActionKeyMapping& KeyMapping : KeyDetails->Actions)
			{
				if (const FViewportKeyState* KeyState = KeyStateMap.Find(KeyMapping.Key))
				{
					if (KeyState->bPressUpThisFrame)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	virtual bool GetButton(FName ActionName)
	{
		ConditionalBuildKeyMappings();
		
		if (const FViewportActionKeyDetails* KeyDetails = ActionKeyMap.Find(ActionName))
		{
			for (const FInputActionKeyMapping& KeyMapping : KeyDetails->Actions)
			{
				if (const FViewportKeyState* KeyState = KeyStateMap.Find(KeyMapping.Key))
				{
					if (KeyState->bPress)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

public:
	virtual float GetAxis(const FName& AxisName)
	{
		ConditionalBuildKeyMappings();

		float AxisValue = 0.f;
		
		FViewportAxisKeyDetails* KeyDetails = AxisKeyMap.Find(AxisName);
		if (KeyDetails)
		{
			for (int32 AxisIndex = 0; AxisIndex < KeyDetails->KeyMappings.Num(); ++AxisIndex)
			{
				const FInputAxisKeyMapping& KeyMapping = (KeyDetails->KeyMappings)[AxisIndex];
				AxisValue += GetKeyValue(KeyMapping.Key) * KeyMapping.Scale;
			}
		}

		return AxisValue;
	}

protected:
	void ConditionalInitAxisProperties();
	float MassageAxisInput(const FKey& InKey, float RawValue);
	
	float GetKeyValue(const FKey& InKey)
	{
		FViewportKeyState const* const KeyState = KeyStateMap.Find(InKey);
		return KeyState ? MassageAxisInput(InKey, KeyState->RawValueAccumulator): 0.f;
	}
	
};
