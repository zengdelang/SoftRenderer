#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CanvasRaycastFilterInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UCanvasRaycastFilterInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API ICanvasRaycastFilterInterface
{
	GENERATED_BODY()

public:
	virtual bool IsRaycastLocationValid(class IMaskableGraphicElementInterface* MaskableGraphicElement, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphicsScreenPoint) = 0;
	
};
