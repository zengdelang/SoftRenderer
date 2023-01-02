#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "DragHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UDragHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnDrag callbacks.
 * 
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IDragHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * When dragging is occurring this will be called every time the cursor is moved.
	 */
	virtual void OnDrag(UPointerEventData* EventData) = 0;

};
