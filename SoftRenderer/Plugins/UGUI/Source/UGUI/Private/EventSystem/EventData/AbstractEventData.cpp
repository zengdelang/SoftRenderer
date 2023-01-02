#include "EventSystem/EventData/AbstractEventData.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UAbstractEventData

UAbstractEventData::UAbstractEventData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUsed = false;
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
