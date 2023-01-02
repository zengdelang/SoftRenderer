#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "SelectHandlerInterface.generated.h"

class UBaseEventData;

UINTERFACE(BlueprintType)
class UGUI_API USelectHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnSelect callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API ISelectHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	virtual void OnSelect(UBaseEventData* EventData) = 0;

};
