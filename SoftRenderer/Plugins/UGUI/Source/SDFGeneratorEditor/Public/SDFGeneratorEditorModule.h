#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSDFGeneratorEditor, Log, All);

class FSDFGeneratorEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	void OnPostEngineInit();


private:
	/** Asset type actions */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> ItemDataAssetTypeActions;
	
};
