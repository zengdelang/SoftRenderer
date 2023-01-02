#include "EventSystem/EventViewportClientInterface.h"
#include "GameFramework/InputSettings.h"
#include "UGUI.h"

IEventViewportClientInterface::IEventViewportClientInterface()
{
	PopKeyEventIndex = 0;
	
	MouseScrollDelta = 0;
	
	bKeyMapsBuilt = false;
	bDirtyMouseBtnStates = false;
	bDirtyKeyStates = false;
	bDirtyTouchStates = false;
	bDirtyMouseScroll = false;
}

void IEventViewportClientInterface::RebuildingKeyMap()
{
	const UInputSettings* InputSettings = GetDefault<UInputSettings>();
	if (InputSettings)
	{
		AxisConfig = InputSettings->AxisConfig;
		AxisMappings = InputSettings->GetAxisMappings();
		ActionMappings = InputSettings->GetActionMappings();

		//append on speech action mappings
		const TArray<FInputActionSpeechMapping>& SpeechMappings = InputSettings->GetSpeechMappings();
		for (const FInputActionSpeechMapping& SpeechMapping : SpeechMappings)
		{
			FInputActionKeyMapping& ConvertedSpeechToActionMap = ActionMappings.AddDefaulted_GetRef();
			ConvertedSpeechToActionMap.ActionName = SpeechMapping.GetActionName();
			ConvertedSpeechToActionMap.Key = SpeechMapping.GetKeyName();
		}
	}
	
	ActionKeyMap.Reset();
	AxisKeyMap.Reset();
	AxisProperties.Reset();
	bKeyMapsBuilt = false;
}

void IEventViewportClientInterface::ConditionalBuildKeyMappings_Internal()
{
	if (ActionKeyMap.Num() == 0)
	{
		struct
		{
			static void Build(const TArray<FInputActionKeyMapping>& Mappings, TMap<FName, FViewportActionKeyDetails>& KeyMap)
			{
				for (const FInputActionKeyMapping& ActionMapping : Mappings)
				{
					TArray<FInputActionKeyMapping>& KeyMappings = KeyMap.FindOrAdd(ActionMapping.ActionName).Actions;
					KeyMappings.AddUnique(ActionMapping);
				}
			}
		} ActionMappingsUtility;

		ActionMappingsUtility.Build(ActionMappings, ActionKeyMap);
	}

	if (AxisKeyMap.Num() == 0)
	{
		struct
		{
			static void Build(const TArray<FInputAxisKeyMapping>& Mappings, TMap<FName, FViewportAxisKeyDetails>& AxisMap)
			{
				for (const FInputAxisKeyMapping& AxisMapping : Mappings)
				{
					bool bAdd = true;
					FViewportAxisKeyDetails& KeyDetails = AxisMap.FindOrAdd(AxisMapping.AxisName);
					for (const FInputAxisKeyMapping& KeyMapping : KeyDetails.KeyMappings)
					{
						if (KeyMapping.Key == AxisMapping.Key)
						{
							UE_LOG(LogUGUI, Error, TEXT("Duplicate mapping of key %s for axis %s"), *KeyMapping.Key.ToString(), *AxisMapping.AxisName.ToString());
							bAdd = false;
							break;
						}
					}
					if (bAdd)
					{
						KeyDetails.KeyMappings.Add(AxisMapping);
					}
				}
			}
		} AxisMappingsUtility;

		AxisMappingsUtility.Build(AxisMappings, AxisKeyMap);
	}

	bKeyMapsBuilt = true;
}

void IEventViewportClientInterface::InputKeyInternal(const FKey& Key, EInputEvent Event, float AmountDepressed, bool bGamePad)
{
	if (!FSlateApplication::IsInitialized())
		return;
	
	if (Event == EInputEvent::IE_Pressed || Event == EInputEvent::IE_Repeat)
	{
		FKeyCharacterEvent KeyCharacter(Key, FSlateApplication::Get().GetModifierKeys(), FSlateApplication::Get().GetUserIndexForKeyboard(),
			Event == EInputEvent::IE_Repeat, 0, false);
		KeyCharacterEvents.Emplace(KeyCharacter);
	}
 
	if (Key.IsMouseButton())
	{
		int32 MouseButton = -1;
		if (Key == EKeys::LeftMouseButton)
		{
			MouseButton = 0;
		}
		else if (Key == EKeys::RightMouseButton)
		{
			MouseButton = 1;
		}
		else if (Key == EKeys::MiddleMouseButton)
		{
			MouseButton = 2;
		}

		if (MouseButton >= 0)
		{
			if (MouseButtonStates.Num() == 0)
			{
				MouseButtonStates.SetNum(3);
			}

			auto& MouseBtnState = MouseButtonStates[MouseButton];
			if (Event == EInputEvent::IE_Pressed || Event == EInputEvent::IE_DoubleClick)
			{
				MouseBtnState.bPress = true;
				MouseBtnState.bPressDownThisFrame = true;
				MouseBtnState.bPressUpThisFrame = false;

				bDirtyMouseBtnStates = true;
			}
			else if (Event == EInputEvent::IE_Released)
			{
				MouseBtnState.bPress = false;
				MouseBtnState.bPressDownThisFrame = false;
				MouseBtnState.bPressUpThisFrame = true;

				bDirtyMouseBtnStates = true;
			}
		}
	}

	// first event associated with this key, add it to the map
	FViewportKeyState& KeyState = KeyStateMap.FindOrAdd(Key);

	switch (Event)
	{
	case IE_Pressed:
		KeyState.RawValueAccumulator = AmountDepressed;

		KeyState.bPress = true;
		KeyState.bPressDownThisFrame = true;
		KeyState.bPressUpThisFrame = false;

		bDirtyKeyStates = true;
		break;
		
	case IE_Repeat:
		KeyState.RawValueAccumulator = AmountDepressed;
		
		bDirtyKeyStates = true;
		break;
		
	case IE_Released:
		KeyState.RawValueAccumulator = 0.f;
		
		KeyState.bPress = false;
		KeyState.bPressDownThisFrame = false;
		KeyState.bPressUpThisFrame = true;

		bDirtyKeyStates = true;
		break;
		
	case IE_DoubleClick:
		KeyState.RawValueAccumulator = AmountDepressed;

		bDirtyKeyStates = true;
		break;
		
	default:
		break;
	}
}

void IEventViewportClientInterface::InputAxisInternal(const FKey& Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamePad)
{
	if (Key == EKeys::MouseWheelAxis)
	{
		MouseScrollDelta = Delta;
		bDirtyMouseScroll = true;
	}
	
	// first event associated with this key, add it to the map
	FViewportKeyState& KeyState = KeyStateMap.FindOrAdd(Key);

	// look for event edges
	if (KeyState.RawValueAccumulator == 0.f && Delta != 0.f)
	{
		KeyState.bPress = true;
		KeyState.bPressDownThisFrame = true;
		KeyState.bPressUpThisFrame = false;
	}
	else if (KeyState.RawValueAccumulator != 0.f && Delta == 0.f)
	{
		KeyState.bPress = false;
		KeyState.bPressDownThisFrame = false;
		KeyState.bPressUpThisFrame = true;
	}

	// accumulate deltas until processed next
	KeyState.RawValueAccumulator += Delta;

	bDirtyKeyStates = true;
}

void IEventViewportClientInterface::InputCharInternal(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
	FKeyCharacterEvent KeyCharacter(FKey(), FSlateApplication::Get().GetModifierKeys(), FSlateApplication::Get().GetUserIndexForKeyboard(),
			false, Character, true);
	KeyCharacterEvents.Emplace(KeyCharacter);
}

void IEventViewportClientInterface::InputTouchInternal(uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation,
                                                 float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex)
{
	// get the current state of each finger
	checkf(TouchPadIndex == 0, TEXT("We currently assume one touchpad in UPlayerInput::InputTouch. If this triggers, add support for multiple pads"));

	// if the handle is out of bounds, we can't handle it
	if (Handle >= EKeys::NUM_TOUCH_KEYS)
	{
		return;
	}

	int32 TouchIndex = -1;
	for (int32 Index = 0, Count = TouchStates.Num(); Index < Count; ++Index)
	{
		const auto& TouchState = TouchStates[Index];
		if (TouchState.GetFingerId() == Handle && !TouchState.IsEndedPhase())
		{
			TouchIndex = Index;
			break;
		}
	}

	if (TouchIndex < 0)
	{
		FViewportTouchState NewTouchState;
		NewTouchState.SetFingerId(Handle);
		TouchIndex = TouchStates.Emplace(NewTouchState);
	}

	auto& TouchState = TouchStates[TouchIndex];
	TouchState.SetPhaseAndPosition(Type, TouchLocation, Force);

	bDirtyTouchStates = true;

	FViewportKeyState& KeyState = KeyStateMap.FindOrAdd(EKeys::TouchKeys[Handle]);
	switch (Type)
	{
	case ETouchType::Began:
		KeyState.RawValueAccumulator = TouchLocation.X;
		
		KeyState.bPress = true;
		KeyState.bPressDownThisFrame = true;
		KeyState.bPressUpThisFrame = false;	
		break;

	case ETouchType::Ended:
		KeyState.bPress = false;
		KeyState.bPressDownThisFrame = false;
		KeyState.bPressUpThisFrame = true;

		KeyState.RawValueAccumulator = 0;
		break;

	default:
		KeyState.RawValueAccumulator = TouchLocation.X;
		break;
	}

	bDirtyKeyStates = true;
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- TickInputStates"), STAT_UnrealGUI_TickInputStates, STATGROUP_UnrealGUI);
void IEventViewportClientInterface::TickInputStates()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_TickInputStates);
	
	if (bDirtyMouseBtnStates)
	{
		bDirtyMouseBtnStates = false;

		for (auto& MouseBtnState : MouseButtonStates)
		{
			MouseBtnState.bPressDownThisFrame = false;
			MouseBtnState.bPressUpThisFrame = false;
		}
	}

	if (bDirtyKeyStates)
	{
		bDirtyKeyStates = false;

		for (TMap<FKey, FViewportKeyState>::TIterator It(KeyStateMap); It; ++It)
		{
			FViewportKeyState* KeyState = &It.Value();
			
			KeyState->bPressDownThisFrame = false;
			KeyState->bPressUpThisFrame = false;

			if (It.Key().IsAxis1D())
			{
				KeyState->RawValueAccumulator = 0;
			}
		}
	}

	if (bDirtyTouchStates)
	{
		bDirtyTouchStates = false;
		
		for (int32 Index = TouchStates.Num() - 1; Index >= 0; --Index)
		{
			auto& TouchState = TouchStates[Index];
			if (TouchState.IsEndedPhase())
			{
				TouchStates.RemoveAt(Index, 1, false);
			}
			else
			{
				TouchState.ResetMoveState();
			}
		}
	}

	if (bDirtyMouseScroll)
	{
		bDirtyMouseScroll = false;
		MouseScrollDelta = 0;
	}

	if (KeyCharacterEvents.Num() > 0)
	{
		KeyCharacterEvents.Reset();
		PopKeyEventIndex = 0;
	}
}

bool IEventViewportClientInterface::PopKeyCharacterEvent(FKeyCharacterEvent& OutKeyCharacterEvent)
{
	if (KeyCharacterEvents.IsValidIndex(PopKeyEventIndex))
	{
		OutKeyCharacterEvent = KeyCharacterEvents[PopKeyEventIndex];
		++PopKeyEventIndex;
		return true;
	}
	return false;
}

void IEventViewportClientInterface::OnLostFocus()
{
	for (auto& MouseState : MouseButtonStates)
	{
		if (MouseState.bPress)
		{
			MouseState.bPress = false;
			MouseState.bPressDownThisFrame = false;
			MouseState.bPressUpThisFrame = true;
		}
	}

	for (auto& TouchState : TouchStates)
	{
		if (!TouchState.IsEndedPhase())
		{
			TouchState.MarkAsEndedPhase();
		}
	}
	
	for (TMap<FKey, FViewportKeyState>::TIterator It(KeyStateMap); It; ++It)
	{
		FViewportKeyState& KeyState = It.Value();
		KeyState.RawValueAccumulator = 0;
		if (KeyState.bPress)
		{
			KeyState.bPress = false;
			KeyState.bPressDownThisFrame = false;
			KeyState.bPressUpThisFrame = true;
		}
	}

	bDirtyMouseBtnStates = true;
	bDirtyKeyStates = true;
	bDirtyTouchStates = true;
}

void IEventViewportClientInterface::ConditionalInitAxisProperties()
{
	// Initialize AxisProperties map if needed.
	if (AxisProperties.Num() == 0)
	{
		// move stuff from config structure to our runtime structure
		for (const FInputAxisConfigEntry& AxisConfigEntry : AxisConfig)
		{
			const FKey AxisKey = AxisConfigEntry.AxisKeyName;
			if (AxisKey.IsValid())
			{
				AxisProperties.Add(AxisKey, AxisConfigEntry.AxisProperties);
			}
		}
	}
}

float IEventViewportClientInterface::MassageAxisInput(const FKey& InKey, float RawValue)
{
	float NewVal = RawValue;

	ConditionalInitAxisProperties();

	// no massaging for buttons atm, might want to support it for things like pressure-sensitivity at some point

	FInputAxisProperties const* const AxisProps = AxisProperties.Find(InKey);
	if (AxisProps)
	{
		// deal with axis dead zone
		if (AxisProps->DeadZone > 0.f)
		{
			// We need to translate and scale the input to the +/- 1 range after removing the dead zone.
			if (NewVal > 0)
			{
				NewVal = FMath::Max(0.f, NewVal - AxisProps->DeadZone) / (1.f - AxisProps->DeadZone);
			}
			else
			{
				NewVal = -FMath::Max(0.f, -NewVal - AxisProps->DeadZone) / (1.f - AxisProps->DeadZone);
			}
		}

		// apply any exponent curvature while we're in the [0..1] range
		if (AxisProps->Exponent != 1.f)
		{
			NewVal = FMath::Sign(NewVal) * FMath::Pow(FMath::Abs(NewVal), AxisProps->Exponent);
		}

		// now apply any scaling (sensitivity)
		NewVal *= AxisProps->Sensitivity;

		if (AxisProps->bInvert)
		{
			NewVal *= -1.f;
		}
	}

	return NewVal;
}