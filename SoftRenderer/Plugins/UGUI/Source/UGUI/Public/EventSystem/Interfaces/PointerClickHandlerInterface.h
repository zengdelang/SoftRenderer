#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "PointerClickHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UPointerClickHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnPointerClick callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IPointerClickHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect clicks.
	 */
	virtual void OnPointerClick(UPointerEventData* EventData) = 0;

};
