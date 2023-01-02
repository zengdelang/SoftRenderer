#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class FSoftRendererEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	void OnPostEngineInit();
	
};
