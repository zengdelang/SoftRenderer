#include "ActorFactoryWidgetActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Core/UIBlueprint.h"
#include "Core/WidgetActor.h"

//////////////////////////////////////////////////////////////////////////
// UActorFactoryWidgetActor

UActorFactoryWidgetActor::UActorFactoryWidgetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("UGUI", "WidgetActorFactoryDisplayName", "Add Widget Actor");
	NewActorClass = AWidgetActor::StaticClass();
}

void UActorFactoryWidgetActor::PostInitProperties()
{
	Super::PostInitProperties();
	MenuPriority = 10000;
}

bool UActorFactoryWidgetActor::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(UUIBlueprint::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoBlueprint", "No Blueprint was specified, or the specified Blueprint needs to be compiled.");
		return false;
	}

	return Super::CanCreateActorFrom(AssetData, OutErrorMsg);
}

void UActorFactoryWidgetActor::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	Super::PostSpawnActor(Asset, NewActor);

	if (IsValid(NewActor) && NewActor->GetRootComponent())
	{
		NewActor->GetRootComponent()->SetWorldRotation(FQuat(FRotator(0, 0, -90)));
	}
}

void UActorFactoryWidgetActor::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	Super::PostCreateBlueprint(Asset, CDO);
}
