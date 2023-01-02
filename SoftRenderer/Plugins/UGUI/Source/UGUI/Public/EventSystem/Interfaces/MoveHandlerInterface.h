#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "MoveHandlerInterface.generated.h"

class UAxisEventData;

UINTERFACE(BlueprintType)
class UGUI_API UMoveHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnMove callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IMoveHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by a BaseInputModule when a move event occurs.
	 */
	virtual void OnMove(UAxisEventData* EventData) = 0;

};
