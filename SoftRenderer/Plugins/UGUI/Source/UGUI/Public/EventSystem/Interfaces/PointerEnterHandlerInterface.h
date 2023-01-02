#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "PointerEnterHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UPointerEnterHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnPointerEnter callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IPointerEnterHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect pointer enter events
	 */
    virtual void OnPointerEnter(UPointerEventData* EventData) = 0;
	
};
