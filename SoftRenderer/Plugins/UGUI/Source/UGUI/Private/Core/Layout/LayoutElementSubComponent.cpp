#include "Core/Layout/LayoutElementSubComponent.h"

/////////////////////////////////////////////////////
// ULayoutElementSubComponent

ULayoutElementSubComponent::ULayoutElementSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    bIgnoreLayout = false;
    MinWidth = -1;
    MinHeight = -1;
    PreferredWidth = -1;
    PreferredHeight = -1;
    FlexibleWidth = -1;
    FlexibleHeight = -1;
    LayoutPriority = 1;
}

#if WITH_EDITORONLY_DATA

void ULayoutElementSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    SetDirty();
}

void ULayoutElementSubComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);
    SetDirty();
}

#endif

/////////////////////////////////////////////////////
