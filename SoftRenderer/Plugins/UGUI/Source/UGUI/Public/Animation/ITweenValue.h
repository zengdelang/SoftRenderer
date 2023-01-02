#pragma once

#include "CoreMinimal.h"

class UGUI_API ITweenValue
{
public:
	virtual ~ITweenValue() {}
	
public:
	virtual void TweenValue(float InPercentage) = 0;

	virtual bool IgnoreTimeScale()
	{
		return bIgnoreTimeScale;
	}

	virtual void SetDuration(float InDuration)
	{
		Duration = InDuration;
	}

	virtual float GetDuration()
	{
		return Duration;
	}

public:
	bool bIgnoreTimeScale;
	float StartDelay = 0.0f;

protected:
	float Duration = 0.01f;
	
};
