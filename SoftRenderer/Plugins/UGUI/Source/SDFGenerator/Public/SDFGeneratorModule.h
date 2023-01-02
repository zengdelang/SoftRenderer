#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSDFGenerator, Log, All);

DECLARE_STATS_GROUP(TEXT("SDFGenerator"), STATGROUP_SDFGenerator, STATCAT_Advanced);

class FSDFGeneratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
