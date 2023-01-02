#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerWidgetActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpriteAtlasPackerWorldSubsystem.generated.h"

UCLASS()
class USpriteAtlasPackerWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ASpriteAtlasPackerWidgetActor* DesignerWidgetActor;
	
public:
	USpriteAtlasPackerWorldSubsystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void SpawnDesignerWidgetActor();
	
};


