#pragma once

#include "CoreMinimal.h"
#include "BaseEventData.h"
#include "EventSystem/MoveDirection.h"
#include "AxisEventData.generated.h"

/**
 * Event Data associated with Axis Events (Controller / Keyboard).
 */
UCLASS(BlueprintType)
class UGUI_API UAxisEventData : public UBaseEventData
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * Raw input vector associated with this event.
	 */
	FVector2D MoveVector;

	/**
	 * MoveDirection for this event.
	 */
	EMoveDirection MoveDir;

};
