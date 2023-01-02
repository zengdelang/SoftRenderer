#pragma once

#include "CoreMinimal.h"
#include "EventHandle.generated.h"

/**
 * Enum that tracks event State.
 */
UENUM(BlueprintType)
enum class EEventHandle : uint8
{
    EventHandle_Unused UMETA(DisplayName = "Unused"),
	
    EventHandle_Used UMETA(DisplayName = "Used"),
};
