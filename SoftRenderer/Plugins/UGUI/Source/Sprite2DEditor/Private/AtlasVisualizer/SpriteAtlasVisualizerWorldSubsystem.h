#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasVisualizerWidgetActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpriteAtlasVisualizerWorldSubsystem.generated.h"

UCLASS()
class USpriteAtlasVisualizerWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ASpriteAtlasVisualizerWidgetActor* DesignerWidgetActor;
	
public:
	USpriteAtlasVisualizerWorldSubsystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void SpawnDesignerWidgetActor();
	
};


