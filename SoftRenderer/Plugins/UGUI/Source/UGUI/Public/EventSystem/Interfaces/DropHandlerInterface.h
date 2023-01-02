#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "DropHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UDropHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnDrop callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IDropHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by a BaseInputModule on a target that can accept a drop.
	 */
	virtual void OnDrop(UPointerEventData* EventData) = 0;

};
