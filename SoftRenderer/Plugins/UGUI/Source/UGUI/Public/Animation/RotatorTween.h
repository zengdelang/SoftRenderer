#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnRotatorTweenCallback, FRotator);

/**
 * Rotator tween class,
 */
class UGUI_API FRotatorTween : public ITweenValue
{
public:
	FRotator Start;
	FRotator Target;

	FOnRotatorTweenCallback OnRotatorTweenCallback;
	
public:
	virtual void TweenValue(float InPercentage) override;
	
};
