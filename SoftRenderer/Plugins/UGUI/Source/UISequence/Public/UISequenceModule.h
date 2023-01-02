#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUISequence, Log, All);

DECLARE_STATS_GROUP(TEXT("UISequence"), STATGROUP_UISequence, STATCAT_Advanced);

class FUISequenceModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
};
