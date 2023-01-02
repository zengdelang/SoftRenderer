#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Toolkits/AssetEditorToolkit.h"

extern const FName UIBlueprintEditorAppIdentifier;

class FUIBlueprintEditor;
class FUIBlueprintCompiler;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUIBlueprintEditorBeginTransaction, UActorComponent* ActorComponent, const FText& Description);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUIBlueprintEditorEndTransaction, UActorComponent* ActorComponent);

/** The public interface of the UI blueprint editor module. */
class UIBLUEPRINTEDITOR_API IUIBlueprintEditorModule : public IModuleInterface, public IHasMenuExtensibility, public IHasToolBarExtensibility
{
public:
	virtual FUIBlueprintCompiler* GetRegisteredCompiler() = 0;

public:
	static FOnUIBlueprintEditorBeginTransaction OnUIBlueprintEditorBeginTransaction;
	static FOnUIBlueprintEditorEndTransaction OnUIBlueprintEditorEndTransaction;
	
};
