#include "UISequenceComponent.h"
#include "Components/DecalComponent.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"

void SetMaterialParameterDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	const FUIMaterialParameterCurve& ParameterCurve = SequenceComp->GetParameterCurve(DataIndex);

	UMaterialInterface* Material = nullptr;
	
	if (const IUIPrimitiveElementInterface* UIPrimitiveElement = Cast<IUIPrimitiveElementInterface>(Obj))
	{
		Material =  UIPrimitiveElement->GetOverrideMaterial(ParameterCurve.MaterialIndex);
	}
	else if (const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Obj))
	{
		Material = PrimitiveComponent->GetMaterial(ParameterCurve.MaterialIndex);
	}
	else if (const UDecalComponent* DecalComponent = Cast<UDecalComponent>(Obj))
	{
		Material = DecalComponent->GetDecalMaterial();
	}

	if (!Material)
	{
		return;
	}
	
	UMaterialInstanceDynamic* DynamicMaterialInstance = Cast<UMaterialInstanceDynamic>(Material);
	if (!DynamicMaterialInstance)
	{
		const FString DynamicName = Material->GetName() + "_Animated";
		const FName UniqueDynamicName = MakeUniqueObjectName(Obj, UMaterialInstanceDynamic::StaticClass(), *DynamicName);

		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(Material, Obj, UniqueDynamicName );
		
		if (IUIPrimitiveElementInterface* UIPrimitiveElement = Cast<IUIPrimitiveElementInterface>(Obj))
		{
			UIPrimitiveElement->SetOverrideMaterial(ParameterCurve.MaterialIndex, DynamicMaterialInstance);
		}
		else if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Obj))
		{
			PrimitiveComponent->SetMaterial(ParameterCurve.MaterialIndex, DynamicMaterialInstance);
		}
		else if (UDecalComponent* DecalComponent = Cast<UDecalComponent>(Obj))
		{
			DecalComponent->SetDecalMaterial(DynamicMaterialInstance);
		}
	}

	for (const auto& ScalarParam : ParameterCurve.Scalars)
	{
		float FloatValue = 0;
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(ScalarParam.SectionIndex), Time, FloatValue);
		DynamicMaterialInstance->SetScalarParameterValue(ScalarParam.ParameterName, FloatValue);
	}
		
	for (const auto& VectorParam : ParameterCurve.Vectors)
	{
		FVector VectorValue;
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(VectorParam.SectionIndex);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, VectorValue.X);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, VectorValue.Y);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, VectorValue.Z);
		DynamicMaterialInstance->SetVectorParameterValue(VectorParam.ParameterName, VectorValue);
	}
		
	for (const auto& ColorParam : ParameterCurve.Colors)
	{
		FLinearColor LinearColor;
		const FUISequenceVectorCurve& VectorCurve = SequenceComp->GetVectorCurveData(ColorParam.SectionIndex);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.X), Time, LinearColor.R);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Y), Time, LinearColor.G);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.Z), Time, LinearColor.B);
		SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(VectorCurve.W), Time, LinearColor.A);
		DynamicMaterialInstance->SetVectorParameterValue(ColorParam.ParameterName, LinearColor);
	}
}

static UISequenceInterpFunc MaterialParameterDefaultFunction = SetMaterialParameterDefaultFunction;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UUISequenceComponent::InitMaterialParameterSetterFunctions()
{

}

void UUISequenceComponent::FindMaterialParameterSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	TrackObject.FunctionPtr = &MaterialParameterDefaultFunction;
}
