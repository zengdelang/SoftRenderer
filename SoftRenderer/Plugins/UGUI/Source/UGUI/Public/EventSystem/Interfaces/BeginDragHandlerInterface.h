#pragma once

#include "CoreMinimal.h"
#include "EventSystemHandlerInterface.h"
#include "BeginDragHandlerInterface.generated.h"

class UPointerEventData;

UINTERFACE(BlueprintType)
class UGUI_API UBeginDragHandlerInterface : public UEventSystemHandlerInterface
{
	GENERATED_BODY()
};

/**
 * Interface to implement if you wish to receive OnBeginDrag callbacks.
 * Note: You need to implement IDragHandler in addition to IBeginDragHandler.
 *
 * Criteria for this event is implementation dependent. For example see StandAloneInputModule.
 */
class UGUI_API IBeginDragHandlerInterface : public IEventSystemHandlerInterface
{
	GENERATED_BODY()

public:
	/**
	 * Called by a BaseInputModule before a drag is started.
	 */
	virtual void OnBeginDrag(UPointerEventData* EventData) = 0;

};
