#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"

void SetVector2DefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
	    if (IsValid(Func))
	    {
	    	const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
        		
        	FVector2D Vector2DValue;
        	if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
        		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
        	{
        		InvokeSetterFunction(Obj, Func, Vector2DValue);
        	}
	    }
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("Vector2D"))
				return;

			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

			FVector2D Vector2DValue;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
			{
				FVector2D* Vector2D = (FVector2D*)Prop->ContainerPtrToValuePtr<void>(Obj);
				Vector2D->X = Vector2DValue.X;
				Vector2D->Y = Vector2DValue.Y;
			}
		}
	}
}

static UISequenceInterpFunc Vector2DefaultFunction = SetVector2DefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformVector2SetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetAnchorMin(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetAnchorMin(Vector2DValue);
		}
	}
}

void SetAnchorMax(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetAnchorMax(Vector2DValue);
		}
	}
}

void SetAnchoredPosition(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetAnchoredPosition(Vector2DValue);
		}
	}
}

void SetSizeDelta(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetSizeDelta(Vector2DValue);
		}
	}
}

void SetPivot(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetPivot(Vector2DValue);
		}
	}
}

void SetOffsetMin(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetOffsetMin(Vector2DValue);
		}
	}
}

void SetOffsetMax(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector2D Vector2DValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector2DValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector2DValue.Y))
		{
			RectTransform->SetOffsetMax(Vector2DValue);
		}
	}
}

void UUISequenceComponent::InitVector2SetterFunctions()
{
	RectTransformVector2SetterFunctions.Emplace(TEXT("AnchorMin"), SetAnchorMin);
	RectTransformVector2SetterFunctions.Emplace(TEXT("AnchorMax"), SetAnchorMax);
	RectTransformVector2SetterFunctions.Emplace(TEXT("AnchoredPosition"), SetAnchoredPosition);
	RectTransformVector2SetterFunctions.Emplace(TEXT("SizeDelta"), SetSizeDelta);
	RectTransformVector2SetterFunctions.Emplace(TEXT("Pivot"), SetPivot);
	RectTransformVector2SetterFunctions.Emplace(TEXT("OffsetMin"), SetOffsetMin);
	RectTransformVector2SetterFunctions.Emplace(TEXT("OffsetMax"), SetOffsetMax);
}

void UUISequenceComponent::FindVector2SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformVector2SetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &Vector2DefaultFunction;
}
