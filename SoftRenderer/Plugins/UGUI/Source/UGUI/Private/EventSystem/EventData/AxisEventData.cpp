#include "EventSystem/EventData/AxisEventData.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UAxisEventData

UAxisEventData::UAxisEventData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MoveVector = FVector2D::ZeroVector;
	MoveDir = EMoveDirection::MoveDirection_None;
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
