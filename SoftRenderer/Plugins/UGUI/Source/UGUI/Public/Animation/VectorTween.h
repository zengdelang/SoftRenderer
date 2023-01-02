#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnVectorTweenCallback, FVector);

/**
 * Vector tween class
 */
class UGUI_API FVectorTween : public ITweenValue
{
public:
	FVector Start;
	FVector Target;

	FOnVectorTweenCallback OnVectorTweenCallback;
	
public:
	virtual void TweenValue(float InPercentage) override;
	
};
