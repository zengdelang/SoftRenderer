#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "UIEditorPerProjectUserSettings.generated.h"

UCLASS(minimalapi, config=UIEditorPerProjectUserSettings)
class UUIEditorPerProjectUserSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(config)
	uint8 bRawEditMode : 1;

	UPROPERTY(config)
	uint8 bShowRaycastRegion : 1;

	UPROPERTY(config)
	uint8 bTrackSelectedComponent : 1;

	UPROPERTY(config)
	uint8 bShowBackgroundImage : 1;

	UPROPERTY(config)
	uint8 bRespectLock : 1;

	UPROPERTY(config)
	uint8 bShowStats : 1;

public:
	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty( FPropertyChangedEvent& PropertyChangedEvent ) override;
#endif
	//~ End UObject Interface

};

