#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMultiVectorTweenCallback, FVector);

/**
 * Multi vector tween class
 */
class UGUI_API FMultiVectorTween : public ITweenValue
{
public:
	void SetWayPoints(const TArray<FVector>& InWayPoints);

	virtual void TweenValue(float InPercentage) override;

public:
	FOnMultiVectorTweenCallback OnMultiVectorTweenCallback;

private:
	TArray<FVector> WayPoints;
	TArray<float> PercentageArray;
	
};
