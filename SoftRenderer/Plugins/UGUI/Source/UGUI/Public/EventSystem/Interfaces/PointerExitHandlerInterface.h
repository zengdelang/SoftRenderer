#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "PointerExitHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UPointerExitHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnPointerExit callbacks.
 * 
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IPointerExitHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect pointer exit events
	 */
    virtual void OnPointerExit(UPointerEventData* EventData) = 0;

};
