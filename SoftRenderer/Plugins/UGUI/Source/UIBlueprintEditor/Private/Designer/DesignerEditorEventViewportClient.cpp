#include "Designer/DesignerEditorEventViewportClient.h"
#include "SCSUIEditorViewportClient.h"
#include "GameFramework/InputSettings.h"
#include "UGUISubsystem.h"

/////////////////////////////////////////////////////
// UDesignerEditorEventViewportClient

UDesignerEditorEventViewportClient::UDesignerEditorEventViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UDesignerEditorEventViewportClient::PostInitProperties()
{
	Super::PostInitProperties();

	RebuildingKeyMap();
}

void UDesignerEditorEventViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	InputKeyInternal(Key, Event, AmountDepressed, bGamepad);
}

void UDesignerEditorEventViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime,
	int32 NumSamples, bool bGamePad)
{
	InputAxisInternal(Key, Delta, DeltaTime, NumSamples, bGamePad);
}

void UDesignerEditorEventViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
	InputCharInternal(InViewport, ControllerId, Character);
}

void UDesignerEditorEventViewportClient::InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type,
                                         const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex)
{
	InputTouchInternal(Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchPadIndex);
}

void UDesignerEditorEventViewportClient::LostFocus(FViewport* InViewport)
{
	OnLostFocus();
}

void UDesignerEditorEventViewportClient::TickInput()
{
	TickInputStates();
}

bool UDesignerEditorEventViewportClient::HasFocus()
{
	if (ViewportClient.IsValid() && ViewportClient.Pin()->Viewport)
	{
		return ViewportClient.Pin()->IsFocused(ViewportClient.Pin()->Viewport);
	}
	return false;
}

bool UDesignerEditorEventViewportClient::IsCursorVisible()
{
	if (ViewportClient.IsValid() && ViewportClient.Pin()->Viewport)
	{
		return ViewportClient.Pin()->GetCursor(ViewportClient.Pin()->Viewport, 0, 0) != EMouseCursor::Type::None;
	}
	return false;
}

FVector2D UDesignerEditorEventViewportClient::GetMouseScreenPosition()
{
	FVector2D MousePosition = FVector2D::ZeroVector;
	
	if (ViewportClient.IsValid() && ViewportClient.Pin()->Viewport && FSlateApplication::Get().IsMouseAttached())
	{
		FIntPoint MousePos;
		ViewportClient.Pin()->Viewport->GetMousePos(MousePos);
		MousePosition = FVector2D(MousePos);
	}

	return MousePosition;
}

/////////////////////////////////////////////////////
