#pragma once

#include "CoreMinimal.h"
#include "SSpriteAtlasPackerSettings.h"
#include "SSpriteAtlasPackerSpriteList.h"
#include "SSpriteAtlasPackerViewport.h"
#include "UIBlueprintEditor/Public/UIEditorViewport/UGUIEditorViewportClient.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SSpriteAtlasPacker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SSpriteAtlasPacker ) {}
	SLATE_END_ARGS()

public:
	/**
	* Default constructor.
	*/
	SSpriteAtlasPacker();
	virtual ~SSpriteAtlasPacker();
	
	void Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

protected:
	void HandleTabManagerPersistLayout(const TSharedRef<FTabManager::FLayout>& LayoutToSave);

protected:
	TSharedRef<SDockTab> HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const;
	static void FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager);
	
protected:
	// Holds the tab manager that manages the front-end's tabs.
	TSharedPtr<FTabManager> TabManager;

	// Holds the list of UI commands.
	TSharedRef<FUICommandList> CommandList;

	mutable TSharedPtr<SSpriteAtlasPackerViewport> ViewportView;
	mutable TSharedPtr<SSpriteAtlasPackerSpriteList> SpriteListView;
	mutable TSharedPtr<SSpriteAtlasPackerSettings> SettingsView;
};
