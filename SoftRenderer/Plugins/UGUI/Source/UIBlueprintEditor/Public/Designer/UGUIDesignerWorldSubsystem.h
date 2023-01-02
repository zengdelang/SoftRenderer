#pragma once

#include "CoreMinimal.h"
#include "BackgroundImageActor.h"
#include "Designer2DWidgetActor.h"
#include "DesignerEditorEventViewportClient.h"
#include "DesignerWidgetActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "UGUIDesignerWorldSubsystem.generated.h"

UCLASS()
class UIBLUEPRINTEDITOR_API UUGUIDesignerWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	ADesignerWidgetActor* DesignerWidgetActor;
	
	UPROPERTY(Transient)
	ADesigner2DWidgetActor* Designer2DWidgetActor;

	UPROPERTY(Transient)
    ABackgroundImageActor* BackgroundImageActor;
	
	UPROPERTY(Transient)
	UDesignerEditorEventViewportClient* EventViewportClient;
	
public:
	UUGUIDesignerWorldSubsystem();
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void SpawnDesignerWidgetActor();
	
};


