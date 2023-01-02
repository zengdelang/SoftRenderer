#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "PointerDownHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UPointerDownHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnPointerDown callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IPointerDownHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect pointer down events.
	 */
	virtual void OnPointerDown(UPointerEventData* EventData) = 0;

};
