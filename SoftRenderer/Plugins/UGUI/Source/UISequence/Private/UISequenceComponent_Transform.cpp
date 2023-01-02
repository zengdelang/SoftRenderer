#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"

void SetTransformDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			const FUISequenceTransformCurve& TransformCurve = SequenceComp->GetTransformCurve(DataIndex);

			FVector LocationValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationX), Time, LocationValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationY), Time, LocationValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationZ), Time, LocationValue.Z);

			FVector RotatorValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationX), Time, RotatorValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationY), Time, RotatorValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationZ), Time, RotatorValue.Z);

			FVector ScaleValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleX), Time, ScaleValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleY), Time, ScaleValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleZ), Time, ScaleValue.Z);
		
			InvokeSetterFunction(Obj, Func, FTransform(FRotator(RotatorValue.Y, RotatorValue.Z, RotatorValue.X), LocationValue, ScaleValue));
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("Transform"))
				return;

			const FUISequenceTransformCurve& TransformCurve = SequenceComp->GetTransformCurve(DataIndex);

			FVector LocationValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationX), Time, LocationValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationY), Time, LocationValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationZ), Time, LocationValue.Z);

			FVector RotatorValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationX), Time, RotatorValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationY), Time, RotatorValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationZ), Time, RotatorValue.Z);

			FVector ScaleValue;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleX), Time, ScaleValue.X);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleY), Time, ScaleValue.Y);
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleZ), Time, ScaleValue.Z);

			FTransform* Transform = (FTransform*)Prop->ContainerPtrToValuePtr<void>(Obj);
			Transform->SetLocation(LocationValue);
			Transform->SetRotation(FRotator(RotatorValue.Y, RotatorValue.Z, RotatorValue.X).Quaternion());
			Transform->SetScale3D(ScaleValue);
		}
	}
}

static UISequenceInterpFunc TransformDefaultFunction = SetTransformDefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformTransformSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetLocalTransform(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		const FUISequenceTransformCurve& TransformCurve = SequenceComp->GetTransformCurve(DataIndex);

		FTransform LocalTransform = RectTransform->GetLocalTransform();
		
		bool bSetLocalLocation = false;
		FVector LocationValue = RectTransform->GetLocalLocation();
		if (TransformCurve.LocationX != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationX), Time, LocationValue.X);
		}
		
		if (TransformCurve.LocationY != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationY), Time, LocationValue.Y);
		}
		
		if (TransformCurve.LocationZ != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationZ), Time, LocationValue.Z);
		}

		if (bSetLocalLocation)
		{
			LocalTransform.SetTranslation(LocationValue);
		}

		bool bSetLocalRotation = false;
		FRotator LocalRotation = RectTransform->GetLocalRotation();
		FVector RotatorValue;
		if (TransformCurve.RotationX != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationX), Time, RotatorValue.X);
			LocalRotation.Roll = RotatorValue.X;
		}
		
		if (TransformCurve.RotationY != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationY), Time, RotatorValue.Y);
			LocalRotation.Pitch = RotatorValue.Y;
		}
		
		if (TransformCurve.RotationZ != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationZ), Time, RotatorValue.Z);
			LocalRotation.Yaw = RotatorValue.Z;
		}
		
		if (bSetLocalRotation)
		{
			LocalTransform.SetRotation(LocalRotation.Quaternion());
		}
		
		bool bSetLocalScale = false;
		FVector ScaleValue = RectTransform->GetLocalScale();
		if (TransformCurve.ScaleX != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleX), Time, ScaleValue.X);
		}

		if (TransformCurve.ScaleY != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleY), Time, ScaleValue.Y);
		}

		if (TransformCurve.ScaleZ != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleZ), Time, ScaleValue.Z);
		}
		
		if (bSetLocalScale)
		{
			LocalTransform.SetScale3D(ScaleValue);
		}
		
		if (bSetLocalLocation || bSetLocalRotation || bSetLocalScale)
		{
			RectTransform->SetLocalTransform(LocalTransform);
		}
	}
}

void SetSceneComponentTransform(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (USceneComponent* SceneComponent = Cast<USceneComponent>(Obj))
	{
		const FUISequenceTransformCurve& TransformCurve = SequenceComp->GetTransformCurve(DataIndex);

		FTransform LocalTransform = SceneComponent->GetRelativeTransform();
		
		bool bSetLocalLocation = false;
		FVector LocationValue = SceneComponent->GetRelativeLocation();
		if (TransformCurve.LocationX != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationX), Time, LocationValue.X);
		}
		
		if (TransformCurve.LocationY != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationY), Time, LocationValue.Y);
		}
		
		if (TransformCurve.LocationZ != UINT16_MAX)
		{
			bSetLocalLocation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.LocationZ), Time, LocationValue.Z);
		}

		if (bSetLocalLocation)
		{
			LocalTransform.SetTranslation(LocationValue);
		}

		bool bSetLocalRotation = false;
		FRotator LocalRotation = SceneComponent->GetRelativeRotation();
		FVector RotatorValue;
		if (TransformCurve.RotationX != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationX), Time, RotatorValue.X);
			LocalRotation.Roll = RotatorValue.X;
		}
		
		if (TransformCurve.RotationY != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationY), Time, RotatorValue.Y);
			LocalRotation.Pitch = RotatorValue.Y;
		}
		
		if (TransformCurve.RotationZ != UINT16_MAX)
		{
			bSetLocalRotation = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.RotationZ), Time, RotatorValue.Z);
			LocalRotation.Yaw = RotatorValue.Z;
		}
		
		if (bSetLocalRotation)
		{
			LocalTransform.SetRotation(LocalRotation.Quaternion());
		}
		
		bool bSetLocalScale = false;
		FVector ScaleValue = SceneComponent->GetRelativeScale3D();
		if (TransformCurve.ScaleX != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleX), Time, ScaleValue.X);
		}

		if (TransformCurve.ScaleY != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleY), Time, ScaleValue.Y);
		}

		if (TransformCurve.ScaleZ != UINT16_MAX)
		{
			bSetLocalScale = true;
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(TransformCurve.ScaleZ), Time, ScaleValue.Z);
		}
		
		if (bSetLocalScale)
		{
			LocalTransform.SetScale3D(ScaleValue);
		}
		
		if (bSetLocalLocation || bSetLocalRotation || bSetLocalScale)
		{
			SceneComponent->SetRelativeTransform(LocalTransform);
		}
	}
}

static UISequenceInterpFunc SetSceneComponentTransformFunction = SetSceneComponentTransform;

void UUISequenceComponent::InitTransformSetterFunctions()
{
	RectTransformTransformSetterFunctions.Emplace(TEXT("LocalTransform"), SetLocalTransform);
}

void UUISequenceComponent::FindTransformSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformTransformSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}

		if (Track.TrackName == TEXT("Transform"))
		{
			TrackObject.FunctionPtr = &SetSceneComponentTransformFunction;
			return;
		}
	}
	
	if (Cast<USceneComponent>(Object))
	{
		TrackObject.FunctionPtr = &SetSceneComponentTransformFunction;
		return;
	}
	
	TrackObject.FunctionPtr = &TransformDefaultFunction;
}
