#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFloatTweenCallback, float);

/**
 * Float tween class
 */
class UGUI_API FFloatTween : public ITweenValue
{
public:
    float StartValue;
    float TargetValue;

	FOnFloatTweenCallback OnFloatTweenCallback;

public:
	virtual void SetDuration(float InDuration) override;
	
public:
	virtual void TweenValue(float InPercentage) override;
	
};
