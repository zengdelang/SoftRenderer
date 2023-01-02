#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_DELEGATE_RetVal(TSharedRef<class ISubComponentDetailCustomization>, FOnGetSubComponentDetailCustomizationInstance);

struct FSubComponentDetailLayoutCallback
{
	FOnGetSubComponentDetailCustomizationInstance DetailLayoutDelegate;
};

typedef TMap< FName, FSubComponentDetailLayoutCallback > FSubComponentCustomDetailLayoutNameMap;

class FUGUIEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:
	virtual void RegisterSubComponentClassLayout(FName ClassName, FOnGetSubComponentDetailCustomizationInstance DetailLayoutDelegate);

	virtual void UnregisterSubComponentClassLayout(FName ClassName);

	virtual bool GetSubComponentClassLayout(FName ClassName, FOnGetSubComponentDetailCustomizationInstance& DetailLayoutDelegate);

protected:
	void OnPostEngineInit() const;

private:
	FSubComponentCustomDetailLayoutNameMap ClassNameToDetailLayoutNameMap;

	/** Asset type actions */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> ItemDataAssetTypeActions;

	FDelegateHandle ComponentMaterialTrackCreateEditorHandle;
	FDelegateHandle RotatorPropertyTrackCreateEditorHandle;
	
};
