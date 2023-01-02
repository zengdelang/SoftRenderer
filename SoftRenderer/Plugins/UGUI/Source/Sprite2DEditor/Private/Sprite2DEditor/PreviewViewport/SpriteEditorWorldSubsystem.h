#pragma once

#include "CoreMinimal.h"
#include "SpriteEditorWidgetActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SpriteEditorWorldSubsystem.generated.h"

UCLASS()
class USpriteEditorWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ASpriteEditorWidgetActor* DesignerWidgetActor;
	
public:
	USpriteEditorWorldSubsystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void SpawnDesignerWidgetActor();
	
};


