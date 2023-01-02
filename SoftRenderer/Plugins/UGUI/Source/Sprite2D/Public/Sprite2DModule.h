#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSprite2D, Log, All);

DECLARE_STATS_GROUP(TEXT("Sprite2D"), STATGROUP_Sprite2D, STATCAT_Advanced);

class FSprite2DModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
