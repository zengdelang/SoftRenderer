#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

bool EvaluateBool(const FUISequenceBoolCurve& BoolCurve, float InTime, bool& OutValue)
{
	if (BoolCurve.Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(BoolCurve.Times, InTime) - 1);
		OutValue = BoolCurve.Values[Index];
		return true;
	}
	else if (BoolCurve.bHasDefaultValue)
	{
		OutValue = BoolCurve.DefaultValue;
		return true;
	}

	return false;
}

void SetBoolDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			bool BoolValue = false;
			if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
			{
				InvokeSetterFunction(Obj, Func, BoolValue);
			}
		}
		else if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(Prop))
		{
			bool BoolValue = false;
			if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
			{
				BoolProperty->SetPropertyValue_InContainer(Obj, BoolValue);
			}
		}
	}
}

static UISequenceInterpFunc BoolDefaultFunction = SetBoolDefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformBoolSetterFunctions;
TMap<FName, UISequenceInterpFunc> GraphicBoolSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetEnabled(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		bool BoolValue = false;
		if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
		{
			RectTransform->SetEnabled(BoolValue);
		}
	}
}

void SetGraying(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		bool BoolValue = false;
		if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
		{
			RectTransform->SetGraying(BoolValue);
		}
	}
}

void SetInvertColor(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		bool BoolValue = false;
		if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
		{
			RectTransform->SetInvertColor(BoolValue);
		}
	}
}

void SetMaskable(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UMaskableGraphicComponent* MaskableGraphic = Cast<UMaskableGraphicComponent>(Obj))
	{
		bool BoolValue = false;
		if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
		{
			MaskableGraphic->SetMaskable(BoolValue);
		}
	}
}

void SetRaycastTarget(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UMaskableGraphicComponent* MaskableGraphic = Cast<UMaskableGraphicComponent>(Obj))
	{
		bool BoolValue = false;
		if (EvaluateBool(SequenceComp->GetBoolCurveData(DataIndex), Time, BoolValue))
		{
			MaskableGraphic->SetRaycastTarget(BoolValue);
		}
	}
}

void UUISequenceComponent::InitBoolSetterFunctions()
{
	RectTransformBoolSetterFunctions.Emplace(TEXT("bEnabled"), SetEnabled);
	RectTransformBoolSetterFunctions.Emplace(TEXT("bGraying"), SetGraying);
	RectTransformBoolSetterFunctions.Emplace(TEXT("bInvertColor"), SetInvertColor);
	
	GraphicBoolSetterFunctions.Emplace(TEXT("bMaskable"), SetMaskable);
	GraphicBoolSetterFunctions.Emplace(TEXT("bRaycastTarget"), SetRaycastTarget);
}

void UUISequenceComponent::FindBoolSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<UMaskableGraphicComponent>(Object))
	{
		if (const auto FunctionPtr = GraphicBoolSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	else if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformBoolSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &BoolDefaultFunction;
}
