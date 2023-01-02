#include "Sprite2DEditor.h"
#include "SSprite2DEditorViewport.h"

#define LOCTEXT_NAMESPACE "Sprite2DEditor"

const FName SpriteEditorAppIdentifier = FName(TEXT("Sprite2DEditorApp"));
static const FName SpriteEditor_ViewportViewTabId("Viewport");
static const FName Sprite2DEditor_DetailTabId("Details");

FSprite2DEditor::FSprite2DEditor()
	: Sprite(nullptr), CommandList(MakeShareable(new FUICommandList))
{
	
}

FSprite2DEditor::~FSprite2DEditor()
{
	
}

void FSprite2DEditor::InitSprite2DEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UObject* ObjectToEdit)
{
	Sprite = CastChecked<USprite2D>(ObjectToEdit);
	CreateDetailView();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_Sprite2DEditor_Layout")
	->AddArea
	(
		FTabManager::NewPrimaryArea()
		->SetOrientation( Orient_Vertical )
		->Split
		(
			FTabManager::NewStack()
			->SetSizeCoefficient(0.3f)
			->AddTab( GetToolbarTabId(), ETabState::OpenedTab )
			->SetHideTabWell( false )
		)
		->Split
		(
			FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->Split(
				FTabManager::NewStack()
						->AddTab(SpriteEditor_ViewportViewTabId, ETabState::OpenedTab)
						->SetSizeCoefficient(.7f)
				)
				->Split
				(
				FTabManager::NewStack()
					->AddTab(Sprite2DEditor_DetailTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(.3f)
				)
		)
	);

	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;

	Sprite->OnSpriteChange.AddRaw(this, &FSprite2DEditor::OnSpriteChange);
	
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, SpriteEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit);
}

void FSprite2DEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_Sprite2DEditor", "Sprite 2D Editor"));
	const auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(SpriteEditor_ViewportViewTabId,
		FOnSpawnTab::CreateSP(this, &FSprite2DEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("Sprite2DEditoTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
	
	InTabManager->RegisterTabSpawner(Sprite2DEditor_DetailTabId,
		FOnSpawnTab::CreateSP(this, &FSprite2DEditor::SpawnTab_Details))
		.SetDisplayName( LOCTEXT("DetailsSprite2DTab", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FSprite2DEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	InTabManager->UnregisterTabSpawner(SpriteEditor_ViewportViewTabId);
	InTabManager->UnregisterTabSpawner(Sprite2DEditor_DetailTabId);
}

void FSprite2DEditor::CreateDetailView()
{
	FDetailsViewArgs Args;
    Args.bHideSelectionTip = true;
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    DetailsView = PropertyModule.CreateDetailView(Args);
    DetailsView->SetObject(Sprite);
}

void FSprite2DEditor::OnSpriteChange() const
{
	ViewportPtr->UpdatePreviewSceneActor();
}

TSharedRef<SDockTab> FSprite2DEditor::SpawnTab_Details(const FSpawnTabArgs& Args) const
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSprite2DEditorTab)
		.Label(LOCTEXT("DetailsTabTitle", "Details"))
		[
			DetailsView.ToSharedRef()
		];

	return SpawnedTab; 
}

TSharedRef<SDockTab> FSprite2DEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSprite2DEditorTab)
		.Label(LOCTEXT("ViewportTabTitle", "Viewport"))
		[
			SAssignNew(ViewportPtr, SSprite2DEditorViewport, CommandList, SharedThis(this))
		];

	return SpawnedTab; 
}

FName FSprite2DEditor::GetToolkitFName() const
{
	return FName("Sprite2DEditor");
}

FText FSprite2DEditor::GetBaseToolkitName() const
{
	return LOCTEXT( "AppLabel", "Sprite 2D Editor" );
}

FString FSprite2DEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Sprite2DEditor ").ToString();
}

FLinearColor FSprite2DEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

#undef LOCTEXT_NAMESPACE
