#include "UISequenceComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Widgets/Text/TextComponent.h"

bool EvaluateByte(const FUISequenceIntCurve& IntCurve, float InTime, int32& OutValue)
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

void SetByteDefaultFunction(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (IsValid(Obj))
	{
		if (IsValid(Func))
		{
			int32 IntValue = 0;
			if (EvaluateByte(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
			{
				InvokeSetterFunction(Obj, Func, static_cast<uint8>(IntValue));
			}
		}
		else if (const FByteProperty* ByteProperty = CastField<FByteProperty>(Prop))
		{
			int32 IntValue = 0;
			if (EvaluateByte(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
			{
				ByteProperty->SetPropertyValue_InContainer(Obj, static_cast<uint8>(IntValue));
			}
		}
	}
}

static UISequenceInterpFunc ByteDefaultFunction = SetByteDefaultFunction;
TMap<FName, UISequenceInterpFunc> TextComponentByteSetterFunctions;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetVerticalOverflow(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UTextComponent* TextComponent = Cast<UTextComponent>(Obj))
	{
		int32 IntValue = 0;
		if (EvaluateByte(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
		{
			TextComponent->SetVerticalOverflow(static_cast<EVerticalWrapMode>(IntValue));
		}
	}
}

void SetHorizontalOverflow(UObject* Obj, UFunction* Func, FProperty* Prop, int32 DataIndex, UUISequenceComponent* SequenceComp, float Time)
{
	if (UTextComponent* TextComponent = Cast<UTextComponent>(Obj))
	{
		int32 IntValue = 0;
		if (EvaluateByte(SequenceComp->GetIntCurveData(DataIndex), Time, IntValue))
		{
			TextComponent->SetHorizontalOverflow(static_cast<EHorizontalWrapMode>(IntValue));
		}
	}
}

void UUISequenceComponent::InitByteSetterFunctions()
{
	TextComponentByteSetterFunctions.Emplace(TEXT("VerticalOverflow"), SetVerticalOverflow);
	TextComponentByteSetterFunctions.Emplace(TEXT("HorizontalOverflow"), SetHorizontalOverflow);
}

void UUISequenceComponent::FindByteSetterFunction(UObject* Object, FUISequenceTrackObject& TrackObject, const FUISequenceTrack& Track)
{
	if (Cast<UTextComponent>(Object))
	{
		if (const auto FunctionPtr = TextComponentByteSetterFunctions.Find(Track.TrackName))
		{
			TrackObject.FunctionPtr = FunctionPtr;
			return;
		}
	}
	
	TrackObject.FunctionPtr = &ByteDefaultFunction;
}
