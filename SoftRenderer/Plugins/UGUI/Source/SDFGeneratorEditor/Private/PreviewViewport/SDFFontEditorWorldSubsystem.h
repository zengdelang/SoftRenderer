#pragma once

#include "CoreMinimal.h"
#include "SDFFontEditorWidgetActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SDFFontEditorWorldSubsystem.generated.h"

UCLASS()
class USDFFontEditorWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ASDFFontEditorWidgetActor* DesignerWidgetActor;
	
public:
	USDFFontEditorWorldSubsystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void SpawnDesignerWidgetActor();
	
};


