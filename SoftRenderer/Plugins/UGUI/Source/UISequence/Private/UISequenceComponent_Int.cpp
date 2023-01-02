#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"

bool EvaluateInt(const FUISequenceIntCurve& IntCurve, float InTime, int32& OutValue)
{
	if (IntCurve.Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(IntCurve.Times, InTime) - 1);
		OutValue = IntCurve.Values[Index];
		return true;
	}
	else if (IntCurve.bHasDefaultValue)
	{
		OutValue = IntCurve.DefaultValue;
		return true;
	}

	return false;
}

void SetIntDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			int32 IntValue = 0;
			if (EvaluateInt(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
			{
				InvokeSetterFunction(Obj, Func, IntValue);
			}
		}
		else if (const FIntProperty* IntProperty = CastField<FIntProperty>(Prop))
		{
			int32 IntValue = 0;
			if (EvaluateInt(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
			{
				IntProperty->SetPropertyValue_InContainer(Obj, IntValue);
			}
		}
	}
}

static UISequenceInterpFunc IntDefaultFunction = SetIntDefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformIntSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetZOrder(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		int32 IntValue = 0;
		if (EvaluateInt(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
		{
			RectTransform->SetZOrder(IntValue);
		}
	}
}

void UUISequenceComponent::InitIntSetterFunctions()
{
	RectTransformIntSetterFunctions.Emplace(TEXT("ZOrder"), SetZOrder);
}

void UUISequenceComponent::FindIntSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformIntSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &IntDefaultFunction;
}
