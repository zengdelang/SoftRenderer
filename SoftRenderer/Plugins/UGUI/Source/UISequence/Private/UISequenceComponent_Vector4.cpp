#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"

void SetVector4DefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
			FVector4 Vector4Value;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector4Value.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector4Value.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, Vector4Value.Z) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, Vector4Value.W))
			{
				InvokeSetterFunction(Obj, Func, Vector4Value);
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("Vector4"))
				return;

			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

			FVector4 Vector4Value;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, Vector4Value.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, Vector4Value.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, Vector4Value.Z) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, Vector4Value.W))
			{
				FVector4* Vector4 = (FVector4*)Prop->ContainerPtrToValuePtr<void>(Obj);
				Vector4->X = Vector4Value.X;
				Vector4->Y = Vector4Value.Y;
				Vector4->Z = Vector4Value.Z;
				Vector4->W = Vector4Value.W;
			}
		}
	}
}

static UISequenceInterpFunc Vector4DefaultFunction = SetVector4DefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformVector4SetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UUISequenceComponent::InitVector4SetterFunctions()
{

}

void UUISequenceComponent::FindVector4SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformVector4SetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &Vector4DefaultFunction;
}
