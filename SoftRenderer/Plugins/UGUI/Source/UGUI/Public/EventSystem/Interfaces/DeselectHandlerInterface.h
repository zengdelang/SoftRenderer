#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "DeselectHandlerInterface.generated.h"

class UBaseEventData;

UINTERFACE(BlueprintType)
class UGUI_API UDeselectHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnDeselect callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IDeselectHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by the EventSystem when a new object is being selected.
	 */
	virtual void OnDeselect(UBaseEventData* EventData) = 0;

};
