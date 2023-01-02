#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "PointerUpHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UPointerUpHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnPointerUp callbacks.
 * Note: In order to receive OnPointerUp callbacks, you must also implement the EventSystems.IPointerDownHandler|IPointerDownHandler interface
 * 
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IPointerUpHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect pointer up events.
	 */
    virtual void OnPointerUp(UPointerEventData* EventData) = 0;

};
