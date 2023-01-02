#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ClipperInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UClipperInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface that can be used to receive clipping callbacks as part of the canvas update loop.
 */
class UGUI_API IClipperInterface
{
	GENERATED_BODY()

public:
	/**
	 * Function to to cull / clip children elements.
	 *
	 * Called after layout and before Graphic update of the Canvas update loop.
	 */
    virtual void PerformClipping() = 0;
	
};
