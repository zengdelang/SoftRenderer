#pragma once

#include "CoreMinimal.h"
#include "TextAnchor.generated.h"

UENUM(BlueprintType)
enum class ETextAnchor : uint8
{
    TextAnchor_UpperLeft UMETA(DisplayName = "Upper Left"),
	
    TextAnchor_UpperCenter UMETA(DisplayName = "Upper Center"),
	
    TextAnchor_UpperRight UMETA(DisplayName = "Upper Right"),
	
    TextAnchor_MiddleLeft UMETA(DisplayName = "Middle Left"),
	
    TextAnchor_MiddleCenter UMETA(DisplayName = "Middle Center"),
	
    TextAnchor_MiddleRight UMETA(DisplayName = "Middle Right"),
	
    TextAnchor_LowerLeft UMETA(DisplayName = "Lower Left"),
	
    TextAnchor_LowerCenter UMETA(DisplayName = "Lower Center"),
	
    TextAnchor_LowerRight UMETA(DisplayName = "Lower Right"),
};
