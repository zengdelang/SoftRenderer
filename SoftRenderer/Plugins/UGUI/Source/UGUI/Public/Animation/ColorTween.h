#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"

enum class EColorTweenMode : uint8
{
	All,
	RGB,
	Alpha
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnColorTweenCallback, FLinearColor);

/**
 * Color tween class,
 */
class UGUI_API FColorTween : public ITweenValue
{
public:
	FLinearColor StartColor;
	FLinearColor TargetColor;

	EColorTweenMode TweenMode;

	FOnColorTweenCallback OnColorTweenCallback;
	
public:
	virtual void TweenValue(float InPercentage) override;
	
};
