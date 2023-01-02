#pragma once

#include "CoreMinimal.h"
#include "EventSystem/EventViewportClientInterface.h"
#include "DesignerEditorEventViewportClient.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UIBLUEPRINTEDITOR_API UDesignerEditorEventViewportClient : public UObject, public IEventViewportClientInterface
{
	GENERATED_UCLASS_BODY()

public:
	TWeakPtr<class FSCSUIEditorViewportClient> ViewportClient;
	
public:
	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	//~ End UObject Interface
	
public:
	virtual void InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad);
	virtual void InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamePad = false);
	virtual void InputChar(FViewport* InViewport,int32 ControllerId, TCHAR Character);
	virtual void InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex);
	virtual void LostFocus(FViewport* InViewport);

public:
	virtual void TickInput();

public:
	virtual bool HasFocus() override;

	virtual bool IsCursorVisible() override;

	virtual FVector2D GetMouseScreenPosition() override;
	
};
