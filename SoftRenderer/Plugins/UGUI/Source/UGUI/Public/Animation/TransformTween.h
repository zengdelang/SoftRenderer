#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"
#include "TweenDefines.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTransformTweenCallback, FTweenTransform);

/**
 * Transform tween class
 */
class UGUI_API FTransformTween : public ITweenValue
{
public:
	FTweenTransform StartValue;
	FTweenTransform TargetValue;

	FOnTransformTweenCallback OnTransformTweenCallback;

public:
	virtual void SetDuration(float InDuration) override;
	
public:
	virtual void TweenValue(float InPercentage) override;
	
};
