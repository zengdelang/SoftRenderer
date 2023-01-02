#include "UIBlueprintEditorModule.h"
#include "ChildWidgetActorCompoentBroker.h"
#include "ComponentAssetBroker.h"
#include "Modules/ModuleManager.h"
#include "KismetCompilerModule.h"
#include "UGUI.h"
#include "UIBlueprintCompiler.h"
#include "UIBlueprintEditorStyle.h"
#include "UISequenceCompiler.h"
#include "Core/UIBlueprint.h"
#include "Core/Layout/ChildWidgetActorComponent.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

const FName UIBlueprintEditorAppIdentifier = FName(TEXT("UIBlueprintEditorApp"));

FOnUIBlueprintEditorBeginTransaction IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction;
FOnUIBlueprintEditorEndTransaction IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction;

class FUIBlueprintEditorModule : public IUIBlueprintEditorModule
{
public:
	/** Constructor, set up console commands and variables **/
	FUIBlueprintEditorModule()
	{
		
	}

	/** Called right after the module DLL has been loaded and the module object has been created */
	virtual void StartupModule() override
	{
		FUIBlueprintEditorStyle::Get();
		
		FCoreDelegates::OnPostEngineInit.AddRaw(this, &FUIBlueprintEditorModule::OnPostEngineInit);
	}

	/** Called before the module is unloaded, right before the module object is destroyed. */
	virtual void ShutdownModule() override
	{
		FUIBlueprintEditorStyle::Destroy();
		
		MenuExtensibilityManager.Reset();
		ToolBarExtensibilityManager.Reset();

		if (UObjectInitialized())
		{
			IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
			KismetCompilerModule.GetCompilers().Remove(&UIBlueprintCompiler);

#if SUPPORT_UI_BLUEPRINT_EDITOR
			FComponentAssetBrokerage::UnregisterBroker(ChildWidgetActorBroker);
#endif
		}
	}

	/** Gets the extensibility managers for outside entities to extend gui page editor's menus and toolbars */
	virtual TSharedPtr<FExtensibilityManager> GetMenuExtensibilityManager() override { return MenuExtensibilityManager; }
	virtual TSharedPtr<FExtensibilityManager> GetToolBarExtensibilityManager() override { return ToolBarExtensibilityManager; }
	
	virtual FUIBlueprintCompiler* GetRegisteredCompiler() override
	{
		return &UIBlueprintCompiler;
	}

	void OnPostEngineInit()
	{
		FModuleManager::LoadModuleChecked<FUGUIModule>("UGUI");

		MenuExtensibilityManager = MakeShared<FExtensibilityManager>();
		ToolBarExtensibilityManager = MakeShared<FExtensibilityManager>();

		// Register widget blueprint compiler we do this no matter what.
		IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
		KismetCompilerModule.GetCompilers().Add(&UIBlueprintCompiler);
		
		FKismetCompilerContext::RegisterCompilerForBP(UUIBlueprint::StaticClass(), [](UBlueprint* InBlueprint, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompileOptions)
		{
			return MakeShared<FUIBlueprintCompilerContext>(CastChecked<UUIBlueprint>(InBlueprint), InMessageLog, InCompileOptions);
		});

#if SUPPORT_UI_BLUEPRINT_EDITOR
		ChildWidgetActorBroker = MakeShareable(new FChildWidgetActorCompoentBroker);
		FComponentAssetBrokerage::RegisterBroker(ChildWidgetActorBroker, UChildWidgetActorComponent::StaticClass(), true, true);
#endif

#if WITH_EDITORONLY_DATA
		UUIBlueprint::OnBlueprintPreSave.AddLambda([](UBlueprint* InBlueprint)
		{
			if (InBlueprint)
			{
				FUISequenceCompiler::CompileUISequenceData(InBlueprint, nullptr);	
			}
		});
#endif
	}

private:
	TSharedPtr<FExtensibilityManager> MenuExtensibilityManager;
	TSharedPtr<FExtensibilityManager> ToolBarExtensibilityManager;

	/** Compiler customization for Widgets */
	FUIBlueprintCompiler UIBlueprintCompiler;

#if SUPPORT_UI_BLUEPRINT_EDITOR
	TSharedPtr<IComponentAssetBroker> ChildWidgetActorBroker;
#endif
	
};

IMPLEMENT_MODULE(FUIBlueprintEditorModule, UIBlueprintEditor);

#undef LOCTEXT_NAMESPACE
