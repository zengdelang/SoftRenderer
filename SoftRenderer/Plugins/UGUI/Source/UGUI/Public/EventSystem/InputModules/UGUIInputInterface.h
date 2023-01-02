#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EventSystem/UGUIGameViewportClient.h"
#include "UGUIInputInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UUGUIInputInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface to the Input system used by the BaseInputModule. With this it is possible to bypass the Input system with your own but still use the same InputModule. For example this can be used to feed fake input into the UI or interface with a different input system.
 */
class UGUI_API IUGUIInputInterface
{
	GENERATED_BODY()

public:
	/**
	 * Interface to ViewportInput.GetMouseButtonDown. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual bool GetMouseButtonDown(int32 Button) = 0;

	/**
	 * Interface to ViewportInput.GetMouseButtonUp. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual bool GetMouseButtonUp(int32 Button) = 0;

	/**
	 * Interface to ViewportInput.GetMouseButton. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual bool GetMouseButton(int32 Button) = 0;
	
	/**
	 * Interface to ViewportInput.MousePosition. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual FVector2D MousePosition() = 0;

	/**
	 * Interface to ViewportInput.MouseScrollDelta. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual float MouseScrollDelta() = 0;

	/**
	 * Interface to ViewportInput.TouchCount. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual int32 TouchCount() = 0;

	/**
	 * Interface to ViewportInput.GetTouch. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual FViewportTouchState* GetTouch(int32 Index) = 0;

	/**
	 * Interface to ViewportInput.GetAxis. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual float GetAxis(const FName& AxisName) = 0;

	/**
	 * Interface to ViewportInput.GetButtonDown. Can be overridden to provide custom input instead of using the Input class.
	 */
	virtual bool GetButtonDown(const FName& ButtonName) = 0;

	virtual bool PopKeyCharacterEvent(FKeyCharacterEvent& OutKeyCharacterEvent) = 0;
	
};
