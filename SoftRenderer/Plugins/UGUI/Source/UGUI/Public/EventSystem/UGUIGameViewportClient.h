#pragma once

#include "CoreMinimal.h"
#include "EventViewportClientInterface.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerInput.h"
#include "UGUIGameViewportClient.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UGUI_API UUGUIGameViewportClient : public UGameViewportClient, public IEventViewportClientInterface
{
	GENERATED_UCLASS_BODY()
	
public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
	
public:
	//~ Begin UGameViewportClient Interface.
	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
	virtual void DetachViewportClient() override;

	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamePad = false) override;
	virtual bool InputChar(FViewport* InViewport,int32 ControllerId, TCHAR Character) override;
	virtual bool InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex) override;
	virtual void LostFocus(FViewport* InViewport) override;
	//~ End UGameViewportClient Interface.

public:
	virtual bool HasFocus() override
	{
		if (Viewport)
		{
			return IsFocused(Viewport);
		}
		return false;
	}

	virtual bool IsCursorVisible() override
	{
		if (Viewport)
		{
			return GetCursor(Viewport, 0, 0) != EMouseCursor::Type::None;
		}
		return false;
	}

	virtual FVector2D GetMouseScreenPosition() override
	{
		FVector2D MousePosition = FVector2D(-1, -1);

		if (Viewport && FSlateApplication::Get().IsMouseAttached())
		{
			FIntPoint MousePos;
			Viewport->GetMousePos(MousePos);
			MousePosition = FVector2D(MousePos);
		}
		
		return MousePosition;
	}
};
