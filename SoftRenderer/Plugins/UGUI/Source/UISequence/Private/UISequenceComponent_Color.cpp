#include "UISequenceComponent.h"
#include "Core/Widgets/GraphicComponent.h"

void SetColorDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
			FLinearColor LinearColor;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, LinearColor.R) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, LinearColor.G) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, LinearColor.B) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, LinearColor.A))
			{
				InvokeSetterFunction(Obj, Func, LinearColor);
			}
		}
		else if (const FStructProperty* StructProperty = CastField<FStructProperty>(Prop))
		{
			if (StructProperty->Struct->GetFName() != TEXT("LinearColor"))
				return;
			
			const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);
		
			FLinearColor LinearColor;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, LinearColor.R) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, LinearColor.G) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, LinearColor.B) &&
				SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, LinearColor.A))
			{
				FLinearColor* Color = (FLinearColor*)Prop->ContainerPtrToValuePtr<void>(Obj);
				Color->R = LinearColor.R;
				Color->G = LinearColor.G;
				Color->B = LinearColor.B;
				Color->A = LinearColor.A;
			}
		}
	}
}

static UISequenceInterpFunc ColorDefaultFunction = SetColorDefaultFunction;
TMap<FName, UISequenceInterpFunc> GraphicColorSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetColor(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UGraphicComponent* Graphic = Cast<UGraphicComponent>(Obj))
	{
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(DataIndex);

		FLinearColor LinearColor;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, LinearColor.R) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, LinearColor.G) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, LinearColor.B) &&
			SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, LinearColor.A))
		{
			Graphic->SetColor(LinearColor);
		}
	}
}

void UUISequenceComponent::InitColorSetterFunctions()
{
	GraphicColorSetterFunctions.Emplace(TEXT("Color"), SetColor);
}

void UUISequenceComponent::FindColorSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<UGraphicComponent>(Object))
	{
		if (const auto FunctionPtr = GraphicColorSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &ColorDefaultFunction;
}
