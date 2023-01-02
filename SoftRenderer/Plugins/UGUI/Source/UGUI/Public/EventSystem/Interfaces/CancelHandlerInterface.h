#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "CancelHandlerInterface.generated.h"

class UBaseEventData;

UINTERFACE(BlueprintType)
class UGUI_API UCancelHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnCancel callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API ICancelHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	virtual void OnCancel(UBaseEventData* EventData) = 0;

};
