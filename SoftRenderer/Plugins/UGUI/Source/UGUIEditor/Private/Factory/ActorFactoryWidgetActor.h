#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactoryBlueprint.h"
#include "ActorFactoryWidgetActor.generated.h"

class AActor;
struct FAssetData;

UCLASS()
class UActorFactoryWidgetActor : public UActorFactoryBlueprint
{
	GENERATED_UCLASS_BODY()

public:
	//~ Begin UActorFactory Interface
	virtual void PostInitProperties() override;
	//~ End UActorFactory Interface
	
protected:
	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	//~ End UActorFactory Interface

	
};
