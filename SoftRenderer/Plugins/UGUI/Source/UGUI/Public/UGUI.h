#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUGUI, Log, All);

DECLARE_STATS_GROUP(TEXT("UnrealGUI"), STATGROUP_UnrealGUI, STATCAT_Advanced);

class FUGUIModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
