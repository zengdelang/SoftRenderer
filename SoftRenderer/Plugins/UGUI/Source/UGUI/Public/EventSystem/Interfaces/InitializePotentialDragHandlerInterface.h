#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "InitializePotentialDragHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UInitializePotentialDragHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnInitializePotentialDrag callbacks.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IInitializePotentialDragHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by a BaseInputModule when a drag has been found but before it is valid to begin the drag.
	 */
	virtual void OnInitializePotentialDrag(UPointerEventData* EventData) = 0;

};
