#include "Core/MathUtility.h"

bool FMathUtility::Approximately(float A, float B)
{
	return FMath::Abs(B - A) < FMath::Max(1E-06f * FMath::Max(FMath::Abs(A), FMath::Abs(B)), MIN_flt * 8);
}

float FMathUtility::SmoothDamp(float Current, float Target, float& CurrentVelocity, float SmoothTime, float MaxSpeed,
	float DeltaTime)
{
	SmoothTime = FMath::Max(0.0001f, SmoothTime);
	
	const float Num1 = 2 / SmoothTime;
	const float Num2 = Num1 * DeltaTime;
	const float Num3 = 1.0 / (1.0 + Num2 + 0.48 * Num2 * Num2 + 0.235 * Num2 * Num2 * Num2);
	const float Num4 = Current - Target;
	const float Num5 = Target;
	const float Max = MaxSpeed * SmoothTime;
	const float Num6 = FMath::Clamp(Num4, -Max, Max);
	
	Target = Current - Num6;
	
	const float Num7 = (CurrentVelocity + Num1 * Num6) * DeltaTime;
	CurrentVelocity = (CurrentVelocity - Num1 * Num7) * Num3;
	float Num8 = Target + (Num6 + Num7) * Num3;
	
	if (Num5 - Current > 0.0 == Num8 > Num5)
	{
		Num8 = Num5;
		CurrentVelocity = (Num8 - Num5) / DeltaTime;
	}
	
	return Num8;
}

float FMathUtility::InverseLerp(float A, float B, float Value)
{
	return (double)A != (double)B ? FMath::Clamp((float)(((double)Value - (double)A) / ((double)B - (double)A)), 0.0f ,1.0f) : 0.0f;
}

float FMathUtility::Repeat(float T, float Length)
{
	return FMath::Clamp(T - FMath::Floor(T / Length) * Length, 0.0f, Length);
}

float FMathUtility::PingPong(float T, float Length)
{
	T = FMathUtility::Repeat(T, Length * 2);
	return Length - FMath::Abs(T - Length);
}
