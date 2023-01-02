#include "UISequenceComponent.h"

int32 GUISequenceLinearCubicInterpolation = 1;
static FAutoConsoleVariableRef CVarUISequenceLinearCubicInterpolation(
	TEXT("UISequence.LinearCubicInterpolation"),
	GUISequenceLinearCubicInterpolation,
	TEXT("If 1 Linear Keys Act As Cubic Interpolation with Linear Tangents, if 0 Linear Key Forces Linear Interpolation to Next Key."),
	ECVF_Default);

static bool EvaluateExtrapolation(const FUISequenceFloatCurve& FloatCurve, float InTime, float& OutValue, const TArray<float>& Times,
	const TArray<FUISequenceFloatValue>& Values, UUISequenceComponent* SequenceComp)
{
	// If the time is outside of the curve, deal with extrapolation
	if (InTime < Times[0])
	{
		if (FloatCurve.PreInfinityExtrap == RCCE_None)
		{
			return false;
		}

		if (FloatCurve.PreInfinityExtrap == RCCE_Constant)
		{
			OutValue = Values[0].Value;
			return true;
		}

		if (FloatCurve.PreInfinityExtrap == RCCE_Linear)
		{
			const FUISequenceFloatValue& FirstValue = Values[0];

			if (FirstValue.InterpMode == RCIM_Constant)
			{
				OutValue = FirstValue.Value;
			}
			else if(FirstValue.InterpMode == RCIM_Cubic)
			{
				const FFrameTime FrameTime = ConvertFrameTime(SequenceComp->GetInputRate().AsFrameTime(Times[0] - InTime), SequenceComp->GetDisplayRate(), SequenceComp->GetTickResolution());
				OutValue = FirstValue.Value - FrameTime.AsDecimal() * FirstValue.Tangent.ArriveTangent;
			}
			else if(FirstValue.InterpMode == RCIM_Linear)
			{
				const float InterpStartFrame = Times[1];
				const float DeltaFrame       = InterpStartFrame - Times[0];
				
				if (FMath::IsNearlyZero(DeltaFrame))
				{
					OutValue = FirstValue.Value;
				}
				else
				{
					OutValue = FMath::Lerp(Values[1].Value, FirstValue.Value, (InterpStartFrame - InTime) / DeltaFrame);
				}
			}
			return true;
		}
	}
	else if (InTime > Times.Last())
	{
		if (FloatCurve.PostInfinityExtrap == RCCE_None)
		{
			return false;
		}

		if (FloatCurve.PostInfinityExtrap == RCCE_Constant)
		{
			OutValue = Values.Last().Value;
			return true;
		}

		if (FloatCurve.PostInfinityExtrap == RCCE_Linear)
		{
			const FUISequenceFloatValue& LastValue = Values.Last();

			if (LastValue.InterpMode == RCIM_Constant)
			{
				OutValue = LastValue.Value;
			}
			else if(LastValue.InterpMode == RCIM_Cubic)
			{
				const FFrameTime FrameTime = ConvertFrameTime(SequenceComp->GetInputRate().AsFrameTime(InTime - Times.Last()), SequenceComp->GetDisplayRate(), SequenceComp->GetTickResolution());
				OutValue = LastValue.Value + FrameTime.AsDecimal() * LastValue.Tangent.LeaveTangent;
			}
			else if(LastValue.InterpMode == RCIM_Linear)
			{
				const int32 NumKeys          = Times.Num();
				const float InterpStartFrame = Times[NumKeys - 2];
				const float DeltaFrame       = Times.Last() - InterpStartFrame;

				if (FMath::IsNearlyZero(DeltaFrame))
				{
					OutValue = LastValue.Value;
				}
				else
				{
					OutValue = FMath::Lerp(Values[NumKeys - 2].Value, LastValue.Value, (InTime - InterpStartFrame) / DeltaFrame);
				}
			}
			return true;
		}
	}

	return false;
}

static void EvaluateTime(const TArray<float>& InTimes, float InTime, int32& OutIndex1, int32& OutIndex2, float& OutInterp)
{
	const int32 Index2 = Algo::UpperBound(InTimes, InTime);
	const int32 Index1 = Index2 - 1;

	OutIndex1 = Index1 >= 0            ? Index1 : INDEX_NONE;
	OutIndex2 = Index2 < InTimes.Num() ? Index2 : INDEX_NONE;

	if (Index1 >= 0 && Index2 < InTimes.Num())
	{
		// Stay in integer space as long as possible
		const float Time1 = InTimes[Index1], Time2 = InTimes[Index2];
		const float Difference = Time2 - Time1;
		
		OutInterp = (InTime - Time1) / static_cast<float>(Difference);
	}
	else
	{
		OutInterp = 0.f;
	}
}

struct FCycleParams
{
	float Time;
	int32 CycleCount;
	float ValueOffset;

	FCycleParams(float InTime)
		: Time(InTime)
		, CycleCount(0)
		, ValueOffset(0.f)
	{}

	FORCEINLINE void ComputePreValueOffset(float FirstValue, float LastValue)
	{
		ValueOffset = (FirstValue - LastValue) * CycleCount;
	}
	
	FORCEINLINE void ComputePostValueOffset(float FirstValue, float LastValue)
	{
		ValueOffset = (LastValue - FirstValue) * CycleCount;
	}
	
	FORCEINLINE void Oscillate(float MinFrame, float MaxFrame)
	{
		if (CycleCount % 2 == 1)
		{
			Time = MinFrame + (MaxFrame - Time);
		}
	}
};

static FCycleParams CycleTime(float MinFrame, float MaxFrame, float InTime)
{
	FCycleParams Params(InTime);
	
	const float Duration = MaxFrame - MinFrame;
	if (Duration <= 0)
	{
		Params.Time = MaxFrame;
		Params.CycleCount = 0;
	}
	else if (InTime < MinFrame)
	{
		const int32 CycleCount = FMath::FloorToInt((MaxFrame - InTime) / Duration);

		Params.Time = InTime + Duration * CycleCount;
		Params.CycleCount = CycleCount;
	}
	else if (InTime > MaxFrame)
	{
		const int32 CycleCount = FMath::FloorToInt((InTime - MinFrame) / Duration);

		Params.Time = InTime - Duration * CycleCount;
		Params.CycleCount = CycleCount;
	}

	return Params;
}

/** Util to find float value on bezier defined by 4 control points */
static float BezierInterp(float P0, float P1, float P2, float P3, float Alpha)
{
	const float P01   = FMath::Lerp(P0,   P1,   Alpha);
	const float P12   = FMath::Lerp(P1,   P2,   Alpha);
	const float P23   = FMath::Lerp(P2,   P3,   Alpha);
	const float P012  = FMath::Lerp(P01,  P12,  Alpha);
	const float P123  = FMath::Lerp(P12,  P23,  Alpha);
	const float P0123 = FMath::Lerp(P012, P123, Alpha);

	return P0123;
}

/* Solve Cubic Euqation using Cardano's forumla
* Adopted from Graphic Gems 1
* https://github.com/erich666/GraphicsGems/blob/master/gems/Roots3And4.c
*  Solve cubic of form
*
* @param Coeff Coefficient parameters of form  Coeff[0] + Coeff[1]*x + Coeff[2]*x^2 + Coeff[3]*x^3 + Coeff[4]*x^4 = 0
* @param Solution Up to 3 real solutions. We don't include imaginary solutions, would need a complex number objecct
* @return Returns the number of real solutions returned in the Solution array.
*/
static int32 SolveCubic(double Coeff[4], double Solution[3])
{
	auto cbrt = [](double x) -> double
	{
		return ((x) > 0.0 ? pow((x), 1.0 / 3.0) : ((x) < 0.0 ? -pow((double)-(x), 1.0 / 3.0) : 0.0));
	};
	int32     NumSolutions = 0;

	/* normal form: x^3 + Ax^2 + Bx + C = 0 */

	double A = Coeff[2] / Coeff[3];
	double B = Coeff[1] / Coeff[3];
	double C = Coeff[0] / Coeff[3];

	/*  substitute x = y - A/3 to eliminate quadric term:
	x^3 +px + q = 0 */

	double SqOfA = A * A;
	double P = 1.0 / 3 * (-1.0 / 3 * SqOfA + B);
	double Q = 1.0 / 2 * (2.0 / 27 * A * SqOfA - 1.0 / 3 * A * B + C);

	/* use Cardano's formula */

	double CubeOfP = P * P * P;
	double D = Q * Q + CubeOfP;

	if (FMath::IsNearlyZero(D))
	{
		if (FMath::IsNearlyZero(Q)) /* one triple solution */
		{
			Solution[0] = 0;
			NumSolutions = 1;
		}
		else /* one single and one double solution */
		{
			double u = cbrt(-Q);
			Solution[0] = 2 * u;
			Solution[1] = -u;
			NumSolutions = 2;
		}
	}
	else if (D < 0) /* Casus irreducibilis: three real solutions */
	{
		double phi = 1.0 / 3 * acos(-Q / sqrt(-CubeOfP));
		double t = 2 * sqrt(-P);

		Solution[0] = t * cos(phi);
		Solution[1] = -t * cos(phi + PI / 3);
		Solution[2] = -t * cos(phi - PI / 3);
		NumSolutions = 3;
	}
	else /* one real solution */
	{
		double sqrt_D = sqrt(D);
		double u = cbrt(sqrt_D - Q);
		double v = -cbrt(sqrt_D + Q);

		Solution[0] = u + v;
		NumSolutions = 1;
	}

	/* resubstitute */

	double Sub = 1.0 / 3 * A;

	for (int32 Index = 0; Index < NumSolutions; ++Index)
		Solution[Index] -= Sub;

	return NumSolutions;
}

/*
*   Convert the control values for a polynomial defined in the Bezier
*		basis to a polynomial defined in the power basis (t^3 t^2 t 1).
*/
static void BezierToPower(	double A1, double B1, double C1, double D1,
	double *A2, double *B2, double *C2, double *D2)
{
	double A = B1 - A1;
	double B = C1 - B1;
	double C = D1 - C1;
	double D = B - A;
	*A2 = C- B - D;
	*B2 = 3.0 * D;
	*C2 = 3.0 * A;
	*D2 = A1;
}

bool UUISequenceComponent::EvaluateFloat(const FUISequenceFloatCurve& FloatCurve, float InTime, float& OutValue)
{
	const auto& Times = FloatCurve.Times;
	const auto& Values = FloatCurve.Values;
	const int32 NumKeys = Times.Num();

	// No keys means default value, or nothing
	if (NumKeys == 0)
	{
		if (FloatCurve.bHasDefaultValue)
		{
			OutValue = FloatCurve.DefaultValue;
			return true;
		}
		return false;
	}

	// For single keys, we can only ever return that value
	if (NumKeys == 1)
	{
		OutValue = Values[0].Value;
		return true;
	}

	// Evaluate with extrapolation if we're outside the bounds of the curve
	if (EvaluateExtrapolation(FloatCurve, InTime, OutValue, Times, Values, this))
	{
		return true;
	}

	const float MinFrame = Times[0];
	const float MaxFrame = Times.Last();

	// Compute the cycled time
	FCycleParams Params = CycleTime(MinFrame, MaxFrame, InTime);

	// Deal with offset cycles and oscillation
	if (InTime < MinFrame)
	{
		switch (FloatCurve.PreInfinityExtrap)
		{
		case RCCE_CycleWithOffset: Params.ComputePreValueOffset(Values[0].Value, Values[NumKeys - 1].Value); break;
		case RCCE_Oscillate:       Params.Oscillate(MinFrame, MaxFrame);                       break;
		default:;
		}
	}
	else if (InTime > MaxFrame)
	{
		switch (FloatCurve.PostInfinityExtrap)
		{
		case RCCE_CycleWithOffset: Params.ComputePostValueOffset(Values[0].Value, Values[NumKeys - 1].Value); break;
		case RCCE_Oscillate:       Params.Oscillate(MinFrame, MaxFrame);                        break;
		default:;
		}
	}

	if (!ensureMsgf(Params.Time >= MinFrame && Params.Time <= MaxFrame, TEXT("Invalid time computed for float curve evaluation")))
	{
		return false;
	}

	// Evaluate the curve data
	float Interp = 0.f;
	int32 Index1 = INDEX_NONE, Index2 = INDEX_NONE;
	EvaluateTime(Times, Params.Time, Index1, Index2, Interp);
	const int32 CheckBothLinear = GUISequenceLinearCubicInterpolation;

	if (Index1 == INDEX_NONE)
	{
		OutValue = Params.ValueOffset + Values[Index2].Value;
	}
	else if (Index2 == INDEX_NONE)
	{
		OutValue = Params.ValueOffset + Values[Index1].Value;
	}
	else
	{
		const FUISequenceFloatValue& Key1 = Values[Index1];
		const FUISequenceFloatValue& Key2 = Values[Index2];
		TEnumAsByte<ERichCurveInterpMode> InterpMode = Key1.InterpMode;
	    if(InterpMode == RCIM_Linear && (CheckBothLinear  && Key2.InterpMode == RCIM_Cubic))
		{
			InterpMode = RCIM_Cubic;
		}
		
		switch (InterpMode)
		{
	    case RCIM_Cubic:
	    {
	    	const FFrameNumber TimeIndex1 = ConvertFrameTime(GetInputRate().AsFrameTime(Times[Index1]), GetDisplayRate(), GetTickResolution()).FloorToFrame();
	    	const FFrameNumber TimeIndex2 = ConvertFrameTime(GetInputRate().AsFrameTime(Times[Index2]), GetDisplayRate(), GetTickResolution()).FloorToFrame();
	    		
			const float OneThird = 1.0f / 3.0f;
			if ((Key1.Tangent.TangentWeightMode == RCTWM_WeightedNone || Key1.Tangent.TangentWeightMode == RCTWM_WeightedArrive)
				&& (Key2.Tangent.TangentWeightMode == RCTWM_WeightedNone || Key2.Tangent.TangentWeightMode == RCTWM_WeightedLeave))
			{
				const int32 Diff = TimeIndex2.Value - TimeIndex1.Value;
				const float P0 = Key1.Value;
				const float P1 = P0 + (Key1.Tangent.LeaveTangent * Diff * OneThird);
				const float P3 = Key2.Value;
				const float P2 = P3 - (Key2.Tangent.ArriveTangent * Diff * OneThird);

				OutValue = Params.ValueOffset + BezierInterp(P0, P1, P2, P3, Interp);
				break;
			}
			else //its weighted
			{
				const float TimeInterval = TickResolution.AsInterval();
				const float ToSeconds = 1.0f / TimeInterval;

				const double Time1 = TickResolution.AsSeconds(TimeIndex1.Value);
				const double Time2 = TickResolution.AsSeconds(TimeIndex2.Value);
				const float X = Time2 - Time1;
				float CosAngle, SinAngle;
				float Angle = FMath::Atan(Key1.Tangent.LeaveTangent * ToSeconds);
				FMath::SinCos(&SinAngle, &CosAngle, Angle);
				float LeaveWeight;
				if (Key1.Tangent.TangentWeightMode == RCTWM_WeightedNone || Key1.Tangent.TangentWeightMode == RCTWM_WeightedArrive)
				{
					const float LeaveTangentNormalized = Key1.Tangent.LeaveTangent / (TimeInterval);
					const float Y = LeaveTangentNormalized * X;
					LeaveWeight = FMath::Sqrt(X*X + Y * Y) * OneThird;
				}
				else
				{
					LeaveWeight = Key1.Tangent.LeaveTangentWeight;
				}
				const float Key1TanX = CosAngle * LeaveWeight + Time1;
				const float Key1TanY = SinAngle * LeaveWeight + Key1.Value;

				Angle = FMath::Atan(Key2.Tangent.ArriveTangent * ToSeconds);
				FMath::SinCos(&SinAngle, &CosAngle, Angle);
				float ArriveWeight;
				if (Key2.Tangent.TangentWeightMode == RCTWM_WeightedNone || Key2.Tangent.TangentWeightMode == RCTWM_WeightedLeave)
				{
					const float ArriveTangentNormalized = Key2.Tangent.ArriveTangent / (TimeInterval);
					const float Y = ArriveTangentNormalized * X;
					ArriveWeight = FMath::Sqrt(X*X + Y * Y) * OneThird;
				}
				else
				{
					ArriveWeight =  Key2.Tangent.ArriveTangentWeight;
				}
				const float Key2TanX = -CosAngle * ArriveWeight + Time2;
				const float Key2TanY = -SinAngle * ArriveWeight + Key2.Value;

				//Normalize the Time Range
				const float RangeX = Time2 - Time1;

				const float Dx1 = Key1TanX - Time1;
				const float Dx2 = Key2TanX - Time1;

				// Normalize values
				const float NormalizedX1 = Dx1 / RangeX;
				const float NormalizedX2 = Dx2 / RangeX;
				
				double Coeff[4];
				double Results[3];

				//Convert Bezier to Power basis, also float to double for precision for root finding.
				BezierToPower(
					0.0, NormalizedX1, NormalizedX2, 1.0,
					&(Coeff[3]), &(Coeff[2]), &(Coeff[1]), &(Coeff[0])
				);

				Coeff[0] = Coeff[0] - Interp;
				
				int32 NumResults = SolveCubic(Coeff, Results);
				float NewInterp = Interp;
				if (NumResults == 1)
				{
					NewInterp = Results[0];
				}
				else
				{
					NewInterp = TNumericLimits<float>::Lowest(); //just need to be out of range
					for (double Result : Results)
					{
						if ((Result >= 0.0f) && (Result <= 1.0f))
						{
							if (NewInterp < 0.0f || Result > NewInterp)
							{
								NewInterp = Result;
							}
						}
					}

					if (NewInterp == TNumericLimits<float>::Lowest())
					{
						NewInterp = 0.f;
					}

				}
				//now use NewInterp and adjusted tangents plugged into the Y (Value) part of the graph.
				const float P0 = Key1.Value;
				const float P1 = Key1TanY;
				const float P3 = Key2.Value;
				const float P2 = Key2TanY;

				OutValue = Params.ValueOffset + BezierInterp(P0, P1, P2, P3,  NewInterp);
			}
			break;
		}
		case RCIM_Linear:
			OutValue = Params.ValueOffset + FMath::Lerp(Key1.Value, Key2.Value, Interp);
			break;

		default:
			OutValue = Params.ValueOffset + Key1.Value;
			break;
		}
	}

	return true;
}
