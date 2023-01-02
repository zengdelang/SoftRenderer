#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UGUIInputSubsystem.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UGUI_API UUGUIInputSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UUGUIInputSubsystem();

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
};
