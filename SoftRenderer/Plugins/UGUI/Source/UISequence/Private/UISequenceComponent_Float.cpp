#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Widgets/ImageComponent.h"

void SetFloatDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			float FloatValue = 0;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
			{
				InvokeSetterFunction(Obj, Func, FloatValue);
			}
		}
		else if (const FFloatProperty* FloatProperty = CastField<FFloatProperty>(Prop))
		{
			float FloatValue = 0;
			if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
			{
				FloatProperty->SetPropertyValue_InContainer(Obj, FloatValue);
			}
		}
	}
}

static UISequenceInterpFunc FloatDefaultFunction = SetFloatDefaultFunction;
TMap<FName, UISequenceInterpFunc> RectTransformFloatSetterFunctions;
TMap<FName, UISequenceInterpFunc> ImageFloatSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetRenderOpacity(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		float FloatValue = 0;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
		{
			RectTransform->SetRenderOpacity(FloatValue);
		}
	}
}

void SetLocalPositionZ(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (URectTransformComponent* RectTransform = Cast<URectTransformComponent>(Obj))
	{
		float FloatValue = 0;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
		{
			RectTransform->SetLocalPositionZ(FloatValue);
		}
	}
}

void SetFillAmount(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UImageComponent* Image = Cast<UImageComponent>(Obj))
	{
		float FloatValue = 0;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
		{
			Image->SetFillAmount(FloatValue);
		}
	}
}

void SetThickness(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UImageComponent* Image = Cast<UImageComponent>(Obj))
	{
		float FloatValue = 0;
		if (SequenceComp->EvaluateFloat(SequenceComp->GetFloatCurveData(DataIndex), Time, FloatValue))
		{
			Image->SetThickness(FloatValue);
		}
	}
}

void UUISequenceComponent::InitFloatSetterFunctions()
{
	RectTransformFloatSetterFunctions.Emplace(TEXT("RenderOpacity"), SetRenderOpacity);
	RectTransformFloatSetterFunctions.Emplace(TEXT("LocalPositionZ"), SetLocalPositionZ);
	
	ImageFloatSetterFunctions.Emplace(TEXT("FillAmount"), SetFillAmount);
	ImageFloatSetterFunctions.Emplace(TEXT("Thickness"), SetThickness);
}

void UUISequenceComponent::FindFloatSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<UImageComponent>(Object))
	{
		if (const auto FunctionPtr = ImageFloatSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	else if (Cast<URectTransformComponent>(Object))
	{
		if (const auto FunctionPtr = RectTransformFloatSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &FloatDefaultFunction;
}
