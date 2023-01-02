#include "SSpriteAtlasPacker.h"
#include "Sprite2DEditorStyle.h"
#include "AtlasPacker/SpriteAtlasPackerPrivate.h"
#include "SSpriteAtlasPackerToolbar.h"

#define LOCTEXT_NAMESPACE "SpriteAtlasPacker"

static const FName ToolbarTabId("Toolbar");
static const FName SpriteListTabId("Sprites");
static const FName ViewportViewTabId("Viewport");
static const FName SettingsViewTabId("Settings");

SSpriteAtlasPacker::SSpriteAtlasPacker() 
	: SCompoundWidget(), CommandList(MakeShareable(new FUICommandList))
{ 

}

SSpriteAtlasPacker::~SSpriteAtlasPacker()
{

}

void SSpriteAtlasPacker::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
	// Tab Spawners
	TabManager = FGlobalTabmanager::Get()->NewTabManager(ConstructUnderMajorTab);
	TSharedRef<FWorkspaceItem> AppMenuGroup = TabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("SpriteAtlasPackerGroupName", "Sprite Atlas Packer"));

	TabManager->RegisterTabSpawner(ToolbarTabId, FOnSpawnTab::CreateRaw(this, &SSpriteAtlasPacker::HandleTabManagerSpawnTab, ToolbarTabId))
		.SetDisplayName(LOCTEXT("ToolbarTabTitle", "Toolbar"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FSprite2DEditorStyle::Get().GetStyleSetName(), "ToolbarTabIcon"));

	TabManager->RegisterTabSpawner(SpriteListTabId, FOnSpawnTab::CreateRaw(this, &SSpriteAtlasPacker::HandleTabManagerSpawnTab, SpriteListTabId))
		.SetDisplayName(LOCTEXT("SpriteListTabTitle", "Sprites"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FontEditor.Tabs.PageProperties"));
	
	TabManager->RegisterTabSpawner(ViewportViewTabId, FOnSpawnTab::CreateRaw(this, &SSpriteAtlasPacker::HandleTabManagerSpawnTab, ViewportViewTabId))
		.SetDisplayName(LOCTEXT("ViewportViewTabTitle", "Preview Viewport"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	TabManager->RegisterTabSpawner(SettingsViewTabId, FOnSpawnTab::CreateRaw(this, &SSpriteAtlasPacker::HandleTabManagerSpawnTab, SettingsViewTabId))
		.SetDisplayName(LOCTEXT("SettingsViewTabTitle", "Settings"))
		.SetGroup(AppMenuGroup)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	// Default Layout
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("Sprite2DAtlasPacker_v1.0")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(ToolbarTabId, ETabState::OpenedTab)
				->SetHideTabWell(true)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.6f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(SpriteListTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.2f)
					)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(ViewportViewTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.6f)
					)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(SettingsViewTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.2f)
					)
			)
		);
	TabManager->SetOnPersistLayout(FTabManager::FOnPersistLayout::CreateRaw(this, &SSpriteAtlasPacker::HandleTabManagerPersistLayout));
	
	// Window Menu
	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(TSharedPtr<FUICommandList>());
	MenuBarBuilder.AddPullDownMenu(
		LOCTEXT("WindowMenuLabel", "Window"),
		FText::GetEmpty(),
		FNewMenuDelegate::CreateStatic(&SSpriteAtlasPacker::FillWindowMenu, TabManager),
		"Window"
		);

	/*MenuBarBuilder.AddMenuEntry(
		LOCTEXT("SettingsMenuLabel", "Settings"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda(
			[this](){
				ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
				if (SettingsModule != nullptr)
				{
					SettingsModule->ShowViewer("Editor", "General", "VisualLogger");
				}
			}
		)),
		"Settings"
		);*/
	
	ChildSlot
   [
	   SNew(SVerticalBox)
	   + SVerticalBox::Slot()
	   .AutoHeight()
	   [
		   MenuBarBuilder.MakeWidget()
	   ]
	   + SVerticalBox::Slot()
	   .FillHeight(1.0f)
	   [
		   TabManager->RestoreFrom(Layout, ConstructUnderWindow).ToSharedRef()
	   ]
   ];
}

void SSpriteAtlasPacker::HandleTabManagerPersistLayout(const TSharedRef<FTabManager::FLayout>& LayoutToSave)
{
	// save any layout here
}

TSharedRef<SDockTab> SSpriteAtlasPacker::HandleTabManagerSpawnTab(const FSpawnTabArgs& Args, FName TabIdentifier) const
{
	TSharedPtr<SWidget> TabWidget = SNullWidget::NullWidget;
	bool AutoSizeTab = false;

	if (TabIdentifier == ToolbarTabId)
	{ 
		TabWidget = SNew(SSpriteAtlasPackerToolbar, CommandList);
		AutoSizeTab = true;
	}
	else if (TabIdentifier == ViewportViewTabId)
	{
		TabWidget = SAssignNew(ViewportView, SSpriteAtlasPackerViewport, CommandList);
		AutoSizeTab = false;
	}
	else if (TabIdentifier == SpriteListTabId)
	{
		TabWidget = SAssignNew(SpriteListView, SSpriteAtlasPackerSpriteList, CommandList);
		AutoSizeTab = false;
	}
	else if (TabIdentifier == SettingsViewTabId)
	{
		TabWidget = SAssignNew(SettingsView, SSpriteAtlasPackerSettings, CommandList);
		AutoSizeTab = false;
	}

	check(TabWidget.IsValid());
	return SNew(SSpriteAtlasPackerTab)
		.ShouldAutosize(AutoSizeTab)
		.TabRole(ETabRole::DocumentTab)
		[
			TabWidget.ToSharedRef()
		];
}

void SSpriteAtlasPacker::FillWindowMenu(FMenuBuilder& MenuBuilder, const TSharedPtr<FTabManager> TabManager)
{
	if (!TabManager.IsValid())
	{
		return;
	}

	TabManager->PopulateLocalTabSpawnerMenu(MenuBuilder);
}

#undef LOCTEXT_NAMESPACE
