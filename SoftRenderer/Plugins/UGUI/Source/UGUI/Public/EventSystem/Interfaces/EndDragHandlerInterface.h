#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "EndDragHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UEndDragHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnEndDrag callbacks.
 * Note: You need to implement IDragHandler in addition to IEndDragHandler.
 *  
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IEndDragHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by a BaseInputModule when a drag is ended.
	 */
	virtual void OnEndDrag(UPointerEventData* EventData) = 0;

};
