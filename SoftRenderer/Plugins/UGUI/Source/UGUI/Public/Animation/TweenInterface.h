#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TweenInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UTweenInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API ITweenInterface
{
    GENERATED_BODY()

protected:
    
    virtual void InitTweenRunner() = 0;

    virtual void InternalPlay() = 0;

    virtual void InternalGoToBegin() = 0;

    virtual void InternalGoToEnd() = 0;

    virtual void InternalToggle() = 0;

    virtual void InternalStartTween() = 0;

};
