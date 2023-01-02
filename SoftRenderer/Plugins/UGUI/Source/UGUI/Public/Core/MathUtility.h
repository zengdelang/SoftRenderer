#pragma once

#include "CoreMinimal.h"

class UGUI_API FMathUtility
{
public:
	static bool Approximately(float A, float B);

	static float SmoothDamp(float Current, float Target, float& CurrentVelocity,
		float SmoothTime, float MaxSpeed, float DeltaTime);

	static float InverseLerp(float A, float B, float Value);

	static float Repeat(float T, float Length);

	static float PingPong(float T, float Length);
	
};
