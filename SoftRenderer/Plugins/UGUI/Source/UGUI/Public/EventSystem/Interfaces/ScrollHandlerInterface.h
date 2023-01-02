#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "ScrollHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UScrollHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnScroll callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IScrollHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Use this callback to detect scroll events.
	 */
	virtual void OnScroll(UPointerEventData* EventData) = 0;

};
