#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"

void SetVector3DefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
			FVector VectorValue;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, VectorValue.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, VectorValue.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, VectorValue.Z))
			{
				InvokeSetterFunction(Obj, Func, VectorValue);
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("Vector"))
				return;

			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

			FVector VectorValue;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, VectorValue.X) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, VectorValue.Y) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, VectorValue.Z))
			{
				FVector* Vector = (FVector*)Prop->ContainerPtrToValuePtr<void>(Obj);
				Vector->X = VectorValue.X;
				Vector->Y = VectorValue.Y;
				Vector->Z = VectorValue.Z;
			}
		}
	}
}

static UISequenceInterpFunc Vector3DefaultFunction = SetVector3DefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformVector3SetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetLocalScale(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector VectorValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, VectorValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, VectorValue.Y) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, VectorValue.Z))
		{
			RectTransform->SetLocalScale(VectorValue);
		}
	}
}

void SetLocalLocation(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FVector VectorValue;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, VectorValue.X) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, VectorValue.Y) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, VectorValue.Z))
		{
			RectTransform->SetLocalLocation(VectorValue);
		}
	}
}

void UUISequenceComponent::InitVector3SetterFunctions()
{
	RectTransformVector3SetterFunctions.Emplace(TEXT("LocalScale"), SetLocalScale);
	RectTransformVector3SetterFunctions.Emplace(TEXT("LocalLocation"), SetLocalLocation);
}

void UUISequenceComponent::FindVector3SetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformVector3SetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &Vector3DefaultFunction;
}
