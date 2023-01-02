#include "EventSystem/InputModules/BaseInput.h"
#include "UGUISubsystem.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UBaseInput

UBaseInput::UBaseInput(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UBaseInput::CacheViewportClient()
{
	if (!ViewportClient)
	{
		ViewportClient = UUGUISubsystem::GetEventViewportClient(this);
	}
}

bool UBaseInput::GetMouseButtonDown(int32 Button)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetMouseButtonDown(Button);
	}
	return false;
}

bool UBaseInput::GetMouseButtonUp(int32 Button)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetMouseButtonUp(Button);
	}
	return false;
}

bool UBaseInput::GetMouseButton(int32 Button)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetMouseButton(Button);
	}
	return false;
}

FVector2D UBaseInput::MousePosition()
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetMouseScreenPosition();
	}
	return FVector2D(-1, -1);
}

float UBaseInput::MouseScrollDelta()
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetMouseScrollDelta();
	}
	return 0;
}

int32 UBaseInput::TouchCount()
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->TouchCount();
	}
	return 0;
}

FViewportTouchState* UBaseInput::GetTouch(int32 Index)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetTouch(Index);
	}
	return nullptr;
}

float UBaseInput::GetAxis(const FName& AxisName)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetAxis(AxisName);
	}
	return 0;
}

bool UBaseInput::GetButtonDown(const FName& ButtonName)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->GetButtonDown(ButtonName);
	}
	return false;
}

bool UBaseInput::PopKeyCharacterEvent(FKeyCharacterEvent& OutKeyCharacterEvent)
{
	CacheViewportClient();
	if (ViewportClient)
	{
		return ViewportClient->PopKeyCharacterEvent(OutKeyCharacterEvent);
	}
	return false;
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
