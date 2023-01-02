#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EventSystemHandlerInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UEventSystemHandlerInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Base class that all EventSystem events inherit from.
 */
class UGUI_API IEventSystemHandlerInterface
{
    GENERATED_BODY()
	
};
