#include "Core/UIBlueprint.h"

/////////////////////////////////////////////////////
// UUIBlueprint

UUIBlueprint::UUIBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

#if WITH_EDITORONLY_DATA

FOnBlueprintPreSave UUIBlueprint::OnBlueprintPreSave;

void UUIBlueprint::PreSave(const ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);

	OnBlueprintPreSave.Broadcast(this);
}

#endif

/////////////////////////////////////////////////////
