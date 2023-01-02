#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/SpecializedCollections/CanvasElementIndexedSet.h"

void SetRotatorDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
			FVector RotatorValue;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, RotatorValue.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, RotatorValue.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, RotatorValue.Z))
			{
				InvokeSetterFunction(Obj, Func, FRotator(RotatorValue.Y, RotatorValue.Z, RotatorValue.X));
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("Rotator"))
				return;

			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

			FVector RotatorValue;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, RotatorValue.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, RotatorValue.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, RotatorValue.Z))
			{
				FRotator* Rotator = (FRotator*)Prop->ContainerPtrToValuePtr<void>(Obj);
				Rotator->Roll = RotatorValue.X;
				Rotator->Pitch = RotatorValue.Y;
				Rotator->Yaw = RotatorValue.Z;
			}
		}
	}
}

static UISequenceInterpFunc RotatorDefaultFunction = SetRotatorDefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformRotatorSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetLocalRotation(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
		FVector RotatorValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, RotatorValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, RotatorValue.Y) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, RotatorValue.Z))
		{
			RectTransform->SetLocalRotation(FRotator(RotatorValue.Y, RotatorValue.Z, RotatorValue.X));
		}
	}
}

void UUISequenceComponent::InitRotatorSetterFunctions()
{
	RectTransformRotatorSetterFunctions.Emplace(TEXT("LocalRotation"), SetLocalRotation);
}

void UUISequenceComponent::FindRotatorSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformRotatorSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &RotatorDefaultFunction;
}
