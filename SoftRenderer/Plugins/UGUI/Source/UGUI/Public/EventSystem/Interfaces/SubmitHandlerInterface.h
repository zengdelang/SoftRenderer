#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "SubmitHandlerInterface.generated.h"

class UBaseEventData;

UINTERFACE(BlueprintType)
class UGUI_API USubmitHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnSubmit callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API ISubmitHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	virtual void OnSubmit(UBaseEventData* EventData) = 0;

};
