#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "UpdateSelectedHandlerInterface.generated.h"

class UBaseEventData;

UINTERFACE(BlueprintType)
class UGUI_API UUpdateSelectedHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnUpdateSelected callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IUpdateSelectedHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by the EventSystem when the object associated with this EventTrigger is updated.
	 */
	virtual void OnUpdateSelected(UBaseEventData* EventData) = 0;

};
