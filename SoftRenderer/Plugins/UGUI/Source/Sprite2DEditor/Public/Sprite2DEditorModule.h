#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSprite2DEditor, Log, All);

DECLARE_STATS_GROUP(TEXT("Sprite2DEditor"), STATGROUP_Sprite2DEditor, STATCAT_Advanced);

class FSprite2DEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	void OnPostEngineInit();

private:
	void RegisterMenus();

	void OpenSprite2DAtlasVisualizer();
	void OpenSprite2DAtlasPacker();
	void ClearSprite2DAtlas();
	
private:
	TSharedRef<SWidget> GetAtlasVisualizer();
	TSharedRef<SWidget> GetTextureAtlasVisualizer();
	TSharedRef<SDockTab> MakeTextureAtlasVisualizerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> MakeSpriteAtlasPackerTab(const FSpawnTabArgs& SpawnTabArgs) const;

private:
	TSharedRef<FExtender> OnExtendContentBrowserDirectoryExtensionMenu(const TArray<FString>& SelectedPaths);
	void DirectoryExtensionContentBrowserMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths) const;

private:
	/** Asset type actions */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> ItemDataAssetTypeActions;

	FDelegateHandle ContentBrowserDirectoryExtensionDelegateHandle;
	
};
