#include "EventSystem/UGUIGameViewportClient.h"
#include "GameFramework/InputSettings.h"
#include "UGUISubsystem.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UUGUIGameViewportClient

UUGUIGameViewportClient::UUGUIGameViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUGUIGameViewportClient::PostInitProperties()
{
	Super::PostInitProperties();

	RebuildingKeyMap();
}

void UUGUIGameViewportClient::Init(FWorldContext& WorldContext, UGameInstance* OwningGameInstance,
	bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	UUGUISubsystem* Subsystem = GEngine->GetEngineSubsystem<UUGUISubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->OnAfterSubsystemTick.AddUObject(this, &UUGUIGameViewportClient::TickInputStates);
	}
}

void UUGUIGameViewportClient::DetachViewportClient()
{
	UUGUISubsystem* Subsystem = GEngine->GetEngineSubsystem<UUGUISubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->OnAfterSubsystemTick.RemoveAll(this);
	}
	
	Super::DetachViewportClient();
}

bool UUGUIGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	InputKeyInternal(EventArgs.Key, EventArgs.Event, EventArgs.AmountDepressed, EventArgs.IsGamepad());
	return Super::InputKey(EventArgs);
}

bool UUGUIGameViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime,
	int32 NumSamples, bool bGamePad)
{
	InputAxisInternal(Key, Delta, DeltaTime, NumSamples, bGamePad);
	return Super::InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamePad);
}

bool UUGUIGameViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
	InputCharInternal(InViewport, ControllerId, Character);
	return Super::InputChar(InViewport, ControllerId, Character);
}

bool UUGUIGameViewportClient::InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type,
                                         const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex)
{
	InputTouchInternal(Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchPadIndex);
	return Super::InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchPadIndex);
}

void UUGUIGameViewportClient::LostFocus(FViewport* InViewport)
{
	Super::LostFocus(InViewport);

	OnLostFocus();
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
