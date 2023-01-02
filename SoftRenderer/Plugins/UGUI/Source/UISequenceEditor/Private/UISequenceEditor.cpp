#include "UISequence.h"
#include "UISequenceComponent.h"
#include "BlueprintEditorModule.h"
#include "BlueprintEditorTabs.h"
#include "UISequenceComponentCustomization.h"
#include "MovieSceneSequenceEditor_UISequence.h"
#include "UISequenceEditorStyle.h"
#include "UISequenceEditorTabSummoner.h"
#include "Framework/Docking/LayoutExtender.h"
#include "LevelEditor.h"
#include "MovieSceneToolsProjectSettings.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ISettingsModule.h"
#include "SequencerSettings.h"
#include "ISequencerModule.h"

#define LOCTEXT_NAMESPACE "UISequenceEditor"

/** Shared class type that ensures safe binding to RegisterBlueprintEditorTab through an SP binding without interfering with module ownership semantics */
class FUISequenceEditorTabBinding
	: public TSharedFromThis<FUISequenceEditorTabBinding>
{
public:
	FUISequenceEditorTabBinding()
	{
		FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
		BlueprintEditorTabSpawnerHandle = BlueprintEditorModule.OnRegisterTabsForEditor().AddRaw(this, &FUISequenceEditorTabBinding::RegisterBlueprintEditorTab);
		BlueprintEditorLayoutExtensionHandle = BlueprintEditorModule.OnRegisterLayoutExtensions().AddRaw(this, &FUISequenceEditorTabBinding::RegisterBlueprintEditorLayout);

		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorTabSpawnerHandle = LevelEditor.OnRegisterTabs().AddRaw(this, &FUISequenceEditorTabBinding::RegisterLevelEditorTab);
		LevelEditorLayoutExtensionHandle = LevelEditor.OnRegisterLayoutExtensions().AddRaw(this, &FUISequenceEditorTabBinding::RegisterLevelEditorLayout);
	}

	void RegisterLevelEditorLayout(FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(FTabId("ContentBrowserTab1"), ELayoutExtensionPosition::Before, FTabManager::FTab(FName("EmbeddedUISequenceID"), ETabState::ClosedTab));
	}

	void RegisterBlueprintEditorLayout(FLayoutExtender& Extender)
	{
		Extender.ExtendLayout(FBlueprintEditorTabs::CompilerResultsID, ELayoutExtensionPosition::Before, FTabManager::FTab(FName("EmbeddedUISequenceID"), ETabState::ClosedTab));
	}

	void RegisterBlueprintEditorTab(FWorkflowAllowedTabSet& TabFactories, FName InModeName, TSharedPtr<FBlueprintEditor> BlueprintEditor)
	{
		TabFactories.RegisterFactory(MakeShared<FUISequenceEditorSummoner>(BlueprintEditor));
	}

	void RegisterLevelEditorTab(TSharedPtr<FTabManager> InTabManager)
	{
		auto SpawnTab = [](const FSpawnTabArgs&) -> TSharedRef<SDockTab>
			{
				TSharedRef<SUISequenceEditorWidget> Widget = SNew(SUISequenceEditorWidget, nullptr);

				return SNew(SDockTab)
					.Label(&Widget.Get(), &SUISequenceEditorWidget::GetDisplayLabel)
					.Icon(FUISequenceEditorStyle::Get().GetBrush("ClassIcon.UISequence"))
					[
						Widget
					];
			};

		InTabManager->RegisterTabSpawner("EmbeddedUISequenceID", FOnSpawnTab::CreateStatic(SpawnTab))
			.SetMenuType(ETabSpawnerMenuType::Hidden)
			.SetAutoGenerateMenuEntry(false);
	}

	~FUISequenceEditorTabBinding()
	{
		FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet");
		if (BlueprintEditorModule)
		{
			BlueprintEditorModule->OnRegisterTabsForEditor().Remove(BlueprintEditorTabSpawnerHandle);
			BlueprintEditorModule->OnRegisterLayoutExtensions().Remove(BlueprintEditorLayoutExtensionHandle);
		}

		FLevelEditorModule* LevelEditor = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditor)
		{
			LevelEditor->OnRegisterTabs().Remove(LevelEditorTabSpawnerHandle);
			LevelEditor->OnRegisterLayoutExtensions().Remove(LevelEditorLayoutExtensionHandle);
		}
	}

private:
	/** Delegate binding handle for FBlueprintEditorModule::OnRegisterTabsForEditor */
	FDelegateHandle BlueprintEditorTabSpawnerHandle, BlueprintEditorLayoutExtensionHandle;

	FDelegateHandle LevelEditorTabSpawnerHandle, LevelEditorLayoutExtensionHandle;
};

/**
 * Implements the UISequenceEditor module.
 */
class FUISequenceEditorModule : public IModuleInterface, public FGCObject
{
public:
	FUISequenceEditorModule() : Settings(nullptr)
	{
		
	}

	virtual void StartupModule() override
	{
		// Register styles
		FUISequenceEditorStyle::Get();

		BlueprintEditorTabBinding = MakeShared<FUISequenceEditorTabBinding>();
		RegisterCustomizations();
		RegisterSettings();
		OnInitializeSequenceHandle = UUISequence::OnInitializeSequence().AddStatic(FUISequenceEditorModule::OnInitializeSequence);

		ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
		SequenceEditorHandle = SequencerModule.RegisterSequenceEditor(UUISequence::StaticClass(), MakeUnique<FMovieSceneSequenceEditor_UISequence>());
	}
	
	virtual void ShutdownModule() override
	{
		UUISequence::OnInitializeSequence().Remove(OnInitializeSequenceHandle);
		UnregisterCustomizations();
		UnregisterSettings();

		ISequencerModule* SequencerModule = FModuleManager::Get().GetModulePtr<ISequencerModule>("Sequencer");
		if (SequencerModule)
		{
			SequencerModule->UnregisterSequenceEditor(SequenceEditorHandle);
		}

		BlueprintEditorTabBinding = nullptr;
	}

	static void OnInitializeSequence(UUISequence* Sequence)
	{
		auto* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
		UMovieScene* MovieScene = Sequence->GetMovieScene();
		
		FFrameNumber StartFrame = (ProjectSettings->DefaultStartTime * MovieScene->GetTickResolution()).RoundToFrame();
		int32        Duration   = (ProjectSettings->DefaultDuration * MovieScene->GetTickResolution()).RoundToFrame().Value;

		MovieScene->SetPlaybackRange(StartFrame, Duration);
	}

	/** Register details view customizations. */
	void RegisterCustomizations()
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		UISequenceComponentName = UUISequenceComponent::StaticClass()->GetFName();
		PropertyModule.RegisterCustomClassLayout("UISequenceComponent", FOnGetDetailCustomizationInstance::CreateStatic(&FUISequenceComponentCustomization::MakeInstance));
	}

	/** Unregister details view customizations. */
	void UnregisterCustomizations()
	{
		FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor");
		if (PropertyModule)
		{
			PropertyModule->UnregisterCustomPropertyTypeLayout(UISequenceComponentName);
		}
	}

	/** Register settings objects. */
	void RegisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			Settings = USequencerSettingsContainer::GetOrCreate<USequencerSettings>(TEXT("EmbeddedUISequenceEditor"));
			Settings->SetKeyInterpPropertiesOnly(true);
			Settings->SetShowRangeSlider(true);
			Settings->SetKeepPlayRangeInSectionBounds(false);
			Settings->SetZeroPadFrames(4);
			Settings->SetInfiniteKeyAreas(true);
			Settings->SetAutoSetTrackDefaults(true);
			Settings->SetCompileDirectorOnEvaluate(false);
			Settings->SetKeyInterpolation(EMovieSceneKeyInterpolation::Linear);
			
			SettingsModule->RegisterSettings("Editor", "ContentEditors", "EmbeddedUISequenceEditor",
				LOCTEXT("EmbeddedUISequenceEditorSettingsName", "Embedded UI Sequence Editor"),
				LOCTEXT("EmbeddedUISequenceEditorSettingsDescription", "Configure the look and feel of the Embedded UI Sequence Editor."),
				Settings);	
		}
	}

	/** Unregister settings objects. */
	void UnregisterSettings()
	{
		ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		if (SettingsModule != nullptr)
		{
			SettingsModule->UnregisterSettings("Editor", "ContentEditors", "EmbeddedUISequenceEditor");
		}
	}

	/** FGCObject interface */
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) override
	{
		if (Settings)
		{
			Collector.AddReferencedObject(Settings);
		}
	}

	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	TSharedPtr<FUISequenceEditorTabBinding> BlueprintEditorTabBinding;
	FName UISequenceComponentName;
	USequencerSettings* Settings;
};

IMPLEMENT_MODULE(FUISequenceEditorModule, UISequenceEditor);

#undef LOCTEXT_NAMESPACE
