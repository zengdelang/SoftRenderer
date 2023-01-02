#include "SDFFontEditor.h"
#include "ObjectTools.h"
#include "PackageTools.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "IContentBrowserSingleton.h"
#include "IMessageLogListing.h"
#include "MessageLogModule.h"
#include "SDFGeneratorEditorCommands.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CharsetsTab/SCharsetsTabWidget.h"
#include "Core/Widgets/Text/SDFFontCharset.h"
#include "FontAtlasGenerator/FontAtlasGenerator.h"
#include "FontAtlasGenerator/FontAtlasPacker.h"
#include "PreviewViewport/SSDFFontEditorViewport.h"
#include "Misc/FeedbackContext.h"

#define LOCTEXT_NAMESPACE "SDFFontEditor"

const FName SDFFontEditorAppIdentifier = FName(TEXT("SDFFontEditorApp"));

static const FName SDFFontEditor_CharsetListTabId("CharsetList");
static const FName SDFFontEditor_ViewportViewTabId("Viewport");
static const FName SDFFontEditor_SettingsViewTabId("Settings");
static const FName SDFFontEditor_PreviewTabId("FontPreview");
static const FName SDFFontEditor_MessageLogTabId("MessageLog");
static const FName NAME_SDFFontEditorEvents(TEXT("SDFFontEditorEvents"));

FSDFFontEditor::FSDFFontEditor()
	: SDFFont(nullptr), CommandList(MakeShareable(new FUICommandList))
{
}

FSDFFontEditor::~FSDFFontEditor()
{
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	if (Editor != NULL)
	{
		Editor->UnregisterForUndo(this);
		Editor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.RemoveAll(this);
	}
}

void FSDFFontEditor::InitSDFFontEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, UObject* ObjectToEdit)
{
	SDFFont = CastChecked<USDFFont>(ObjectToEdit);

	// Support undo/redo
	SDFFont->SetFlags(RF_Transactional);

	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	if (Editor != nullptr)
	{
		Editor->RegisterForUndo(this);
	}

	BindCommands();

	CreateInternalWidgets();
	
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_SDFFontEditor_Layout_v2")
	->AddArea
	(
		FTabManager::NewPrimaryArea() ->SetOrientation( Orient_Vertical )
		->Split
		(
			FTabManager::NewStack()
			->AddTab( GetToolbarTabId(), ETabState::OpenedTab ) ->SetHideTabWell( true )
		)
		->Split
		(
		FTabManager::NewSplitter()
			->SetOrientation(Orient_Horizontal)
			->SetSizeCoefficient(0.6f)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(SDFFontEditor_CharsetListTabId, ETabState::OpenedTab)
				->SetSizeCoefficient(0.2f)
			)
			->Split
			(
				FTabManager::NewSplitter() ->SetOrientation(Orient_Vertical) ->SetSizeCoefficient(0.6f)
				->Split
				(
					FTabManager::NewStack()
					->AddTab(SDFFontEditor_ViewportViewTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.7f)
				)
				->Split
				(
					FTabManager::NewStack()
					->AddTab( SDFFontEditor_PreviewTabId, ETabState::OpenedTab )
					->AddTab( SDFFontEditor_MessageLogTabId, ETabState::ClosedTab )
					->SetSizeCoefficient(0.3f)
				)
			)
			->Split
			(
				FTabManager::NewStack()
				->AddTab(SDFFontEditor_SettingsViewTabId, ETabState::OpenedTab)
				->SetSizeCoefficient(0.2f)
			)
		)
	);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;
	
	FAssetEditorToolkit::InitAssetEditor(Mode, InitToolkitHost, SDFFontEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, ObjectToEdit);
	
	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

void FSDFFontEditor::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_FontEditor", "Font Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);
	
	InTabManager->RegisterTabSpawner( SDFFontEditor_CharsetListTabId, FOnSpawnTab::CreateSP(this, &FSDFFontEditor::SpawnTab_Charsets) )
		.SetDisplayName( LOCTEXT("CharsetsSDFFontEditorTab", "Charsets") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FontEditor.Tabs.PageProperties"));

	InTabManager->RegisterTabSpawner( SDFFontEditor_ViewportViewTabId, FOnSpawnTab::CreateSP(this, &FSDFFontEditor::SpawnTab_Viewport) )
		.SetDisplayName( LOCTEXT("TViewportSDFFontEditorTab", "Viewport") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));
	
	InTabManager->RegisterTabSpawner( SDFFontEditor_SettingsViewTabId,	FOnSpawnTab::CreateSP(this, &FSDFFontEditor::SpawnTab_Details) )
		.SetDisplayName( LOCTEXT("DetailsSDFFontTab", "Details") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner( SDFFontEditor_PreviewTabId,	FOnSpawnTab::CreateSP(this, &FSDFFontEditor::SpawnTab_Preview) )
		.SetDisplayName( LOCTEXT("PreviewSDFFontTab", "Preview") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "FontEditor.Tabs.Preview"));

	InTabManager->RegisterTabSpawner( SDFFontEditor_MessageLogTabId,	FOnSpawnTab::CreateSP(this, &FSDFFontEditor::SpawnTab_MessageLog) )
		.SetDisplayName( LOCTEXT("MessageLogSDFFontTab", "Results") )
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "Kismet.Tabs.CompilerResults"));
}

void FSDFFontEditor::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	
	InTabManager->UnregisterTabSpawner( SDFFontEditor_CharsetListTabId );
	InTabManager->UnregisterTabSpawner( SDFFontEditor_ViewportViewTabId );	
	InTabManager->UnregisterTabSpawner( SDFFontEditor_SettingsViewTabId );
	InTabManager->UnregisterTabSpawner( SDFFontEditor_PreviewTabId );
	InTabManager->UnregisterTabSpawner( SDFFontEditor_MessageLogTabId );
}

void FSDFFontEditor::SaveAsset_Execute()
{
	FAssetEditorToolkit::SaveAsset_Execute();

	if (SDFFont && SDFFont->FontTextureArray)
	{
		TArray<UObject*> Assets;
		Assets.Add(SDFFont->FontTextureArray);

		for (auto& Texture : SDFFont->FontTextureArray->SourceTextures)
		{
			if (Texture)
			{
				Assets.Add(Texture);
			}
		}
		
		UEditorAssetLibrary::SaveLoadedAssets(Assets);
	}
}

FName FSDFFontEditor::GetToolkitFName() const
{
	return FName("SDFFontEditor");
}

FText FSDFFontEditor::GetBaseToolkitName() const
{
	return LOCTEXT( "AppLabel", "SDF Font Editor" );
}

FString FSDFFontEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "SDFFont ").ToString();
}

FLinearColor FSDFFontEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

void FSDFFontEditor::PostUndo(bool bSuccess)
{
	FEditorUndoClient::PostUndo(bSuccess);

	if (CharsetsTabWidget.IsValid())
	{
		CharsetsTabWidget->PostUndo(bSuccess);
	}
}

TSharedRef<SDockTab> FSDFFontEditor::SpawnTab_Charsets(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSDFFontEditorTab)
		.Label(LOCTEXT("CharsetsTabTitle", "Charsets"))
		[
			SAssignNew(CharsetsTabWidget, SCharsetsTabWidget, CommandList, SharedThis(this))
		];

	return SpawnedTab; 
}

TSharedRef<SDockTab> FSDFFontEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSDFFontEditorTab)
		.Label(LOCTEXT("ViewportTabTitle", "Viewport"))
		[
			SNew(SSDFFontEditorViewport, CommandList, SharedThis(this))
		];

	return SpawnedTab; 
}

TSharedRef<SDockTab> FSDFFontEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSDFFontEditorTab)
		.Label(LOCTEXT("DetailsTabTitle", "Details"))
		[
			DetailsView.ToSharedRef()
		];

	return SpawnedTab; 
}

TSharedRef<SDockTab> FSDFFontEditor::SpawnTab_Preview(const FSpawnTabArgs& Args)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SSDFFontEditorTab)
		.Icon(FEditorStyle::GetBrush("FontEditor.Tabs.Preview"))
		.Label(LOCTEXT("FontPreviewTitle", "Preview"))
		[
			FontPreview.ToSharedRef()
		];
	
	return SpawnedTab;
}

TSharedRef<SDockTab> FSDFFontEditor::SpawnTab_MessageLog(const FSpawnTabArgs& Args)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>(TEXT("MessageLog"));
	if (!MessageLogModule.IsRegisteredLogListing(NAME_SDFFontEditorEvents))
	{
		MessageLogModule.RegisterLogListing(NAME_SDFFontEditorEvents, LOCTEXT("WidgetEventLog", "Widget Events"), FMessageLogInitializationOptions{});
	}
	MessageLogListing = MessageLogModule.GetLogListing(NAME_SDFFontEditorEvents);
	
	TSharedRef<SDockTab> SpawnedTab = SNew(SSDFFontEditorTab)
	.Icon(FEditorStyle::GetBrush("Kismet.Tabs.CompilerResults"))
	.Label(LOCTEXT("MessageLogTitle", "Results"))
	[
		MessageLogModule.CreateLogListingWidget(MessageLogListing.ToSharedRef())
	];
	
	return SpawnedTab;
}

void FSDFFontEditor::BindCommands()
{
	const FSDFGeneratorEditorCommands& Commands = FSDFGeneratorEditorCommands::Get();

	ToolkitCommands->MapAction(
		FSDFGeneratorEditorCommands::Get().GenerateSDFFont,
		FExecuteAction::CreateSP(this, &FSDFFontEditor::OnGenerateSDFFont),
		FCanExecuteAction::CreateSP(this, &FSDFFontEditor::OnGenerateSDFFontEnabled));
}

void FSDFFontEditor::CreateInternalWidgets()
{
	PreviewText = NSLOCTEXT("FontEditor", "DefaultPreviewText", "The quick brown fox jumps over the lazy dog");
	
	FontPreview =
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(SSpacer)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SEditableTextBox)
				.Text(NSLOCTEXT("FontEditor", "DefaultPreviewText", "The quick brown fox jumps over the lazy dog"))
				.SelectAllTextWhenFocused(true)
				.OnTextChanged(this, &FSDFFontEditor::OnPreviewTextChanged)
			]
		];
	
	// Details View
	{
		FDetailsViewArgs Args;
		Args.bHideSelectionTip = true;
		//Args.NotifyHook = this;
	
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		DetailsView = PropertyModule.CreateDetailView(Args);

		//DetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateRaw(this, &FSDFFontEditor::GetIsPropertyVisible));
		DetailsView->SetObject(SDFFont);
	}
}

void FSDFFontEditor::ExtendToolbar()
{
	// If the ToolbarExtender is valid, remove it before rebuilding it
	if(ToolbarExtender.IsValid())
	{
		RemoveToolbarExtender(ToolbarExtender);
		ToolbarExtender.Reset();
	}

	ToolbarExtender = MakeShareable(new FExtender);

	AddToolbarExtender(ToolbarExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateSP(this, &FSDFFontEditor::FillToolbar)
	);
}

void FSDFFontEditor::FillToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("Toolbar");
	{
		ToolbarBuilder.AddToolBarButton(
			FSDFGeneratorEditorCommands::Get().GenerateSDFFont,
			NAME_None,
			TAttribute<FText>(), TAttribute<FText>(),
			FSlateIcon(FEditorStyle::Get().GetStyleSetName(), TEXT("FontEditor.UpdateAll"))
		);

		/*ToolbarBuilder.AddToolBarButton(
			FSDFGeneratorEditorCommands::Get().GenerateSDFFont,
			NAME_None,
			TAttribute<FText>(), TAttribute<FText>(),
			FSlateIcon(FEditorStyle::Get().GetStyleSetName(), TEXT("FontEditor.ExportAllPages"))
		);*/
	}
	ToolbarBuilder.EndSection();
}

void FSDFFontEditor::OnPreviewTextChanged(const FText& Text)
{
	PreviewText = Text;
}

void FSDFFontEditor::OnGenerateSDFFont()
{
	TabManager->TryInvokeTab(SDFFontEditor_MessageLogTabId);

	if (MessageLogListing.IsValid())
	{
		MessageLogListing->ClearMessages();
	}
	
#if WITH_FREETYPE
	TArray<FontAtlasGenerator::FFontInput> FontInputs;

	for (const auto& Charset : SDFFont->Charsets)
	{
		if (auto CharsetObj = Cast<USDFFontCharset>(Charset))
		{
			if (!CharsetObj->bEnabled)
			{
				continue;	
			}
			
			FontAtlasGenerator::FFontInput FontInput;

			FString FinalFilename = CharsetObj->FontFilename;
			if (!FPaths::FileExists(FinalFilename))
			{
				FString ProjectDir = FPaths::ProjectDir();
				FinalFilename = FPaths::Combine(ProjectDir, CharsetObj->FontFilename);

				FString LastFinalFilename = FinalFilename;
				if (!FPaths::FileExists(FinalFilename))
				{
					ProjectDir = FPaths::GetPath(FPaths::GetPath(FPaths::EnginePluginsDir()));
					FinalFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(ProjectDir, CharsetObj->FontFilename));
					if (!FPaths::FileExists(FinalFilename))
					{
						FinalFilename = LastFinalFilename;
					}
				}
			}

			FontInput.FontFilename = FinalFilename;
			
			FontInput.Charset = CharsetObj->Charset;
			
			switch (CharsetObj->FontStyle)
			{
			case ESDFFontStyle::Normal:
				FontInput.FontStyle = FontAtlasGenerator::EFontStyle::Normal;
				break;
			case ESDFFontStyle::Bold:
				FontInput.FontStyle = FontAtlasGenerator::EFontStyle::Bold;
				break;
			case ESDFFontStyle::Italic:
				FontInput.FontStyle = FontAtlasGenerator::EFontStyle::Italic;
				break;
			case ESDFFontStyle::BoldAndItalic:
				FontInput.FontStyle = FontAtlasGenerator::EFontStyle::BoldAndItalic;
				break;
			default:
				FontInput.FontStyle = FontAtlasGenerator::EFontStyle::Normal;
				break;
			}

			FontInput.CharsetName = CharsetObj->Name;
			FontInput.FaceIndex = CharsetObj->FaceIndex;
			FontInput.PxRange = CharsetObj->PxRange;
			FontInput.FontScale = CharsetObj->FontScale;
			FontInputs.Emplace(FontInput);
		}
	}
	
	FontAtlasGenerator::FFontAtlasPacker AtlasPacker;
	FontAtlasGenerator::FFont FontInfo;
	bool bError = FontAtlasGenerator::GenerateFontAtlas(FontInputs, AtlasPacker, FontInfo, SDFFont->MaxAtlasSize, SDFFont->bOverlapSupport, SDFFont->bScanlinePass, this, SDFFont->CustomBaselineOffset);

	if (bError)
	{
		bRefreshTextComponent = true;
		return;
	}

	SDFFont->Modify();

	SDFFont->AtlasWidth = FontInfo.AtlasWidth;
	SDFFont->AtlasHeight = FontInfo.AtlasHeight;
	SDFFont->ImportFontSize = FontInfo.ImportFontSize;
	SDFFont->LineHeight = FontInfo.LineHeight;
	SDFFont->Baseline = FontInfo.Baseline;
	SDFFont->BaselineOffset = FontInfo.BaselineOffset;
	SDFFont->UnderlineY = FontInfo.UnderlineY;
	SDFFont->UnderlineThickness = FontInfo.UnderlineThickness;
	SDFFont->NotDefineCharIndex = FontInfo.NotDefineCharIndex;
	SDFFont->ScreenPxRanges = MoveTemp(FontInfo.ScreenPxRanges);

	SDFFont->Characters.Empty();
	SDFFont->NormalCharRemap.Empty();
	SDFFont->BoldCharRemap.Empty();
	SDFFont->ItalicCharRemap.Empty();
	SDFFont->BoldAndItalicCharRemap.Empty();
	
	for (auto& Character : FontInfo.Characters)
	{
		FSDFFontCharacter FontChar;
		FontChar.StartU = Character.StartU;
		FontChar.StartV = Character.StartV;
		FontChar.USize = Character.USize;
		FontChar.VSize = Character.VSize;
		FontChar.Advance = Character.Advance;
		FontChar.HorizontalOffset = Character.HorizontalOffset;
		FontChar.AscenderY = Character.AscenderY;
		FontChar.Scale = Character.Scale;
		FontChar.TextureIndex = Character.TextureIndex;
		FontChar.ScreenPxRangeIndex = Character.ScreenPxRangeIndex;
		SDFFont->Characters.Emplace(FontChar);
	}

	for (auto& Elem : FontInfo.NormalCharRemap)
	{
		SDFFont->NormalCharRemap.Emplace(Elem.Key, Elem.Value);
	}

	for (auto& Elem : FontInfo.BoldCharRemap)
	{
		SDFFont->BoldCharRemap.Emplace(Elem.Key, Elem.Value);
	}

	for (auto& Elem : FontInfo.ItalicCharRemap)
	{
		SDFFont->ItalicCharRemap.Emplace(Elem.Key, Elem.Value);
	}

	for (auto& Elem : FontInfo.BoldAndItalicCharRemap)
	{
		SDFFont->BoldAndItalicCharRemap.Emplace(Elem.Key, Elem.Value);
	}

	TArray<UObject*> ObjectsToSync;
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		const FString LongPackagePath = FPackageName::GetLongPackagePath(SDFFont->GetOutermost()->GetPathName());
		
		// Create a package for the frame
		const FString TargetSubPath = LongPackagePath + TEXT("/FontTextures");

		FString FontName = SDFFont->GetName();
		FontName.ReplaceInline(TEXT("UGUI"), TEXT(""));
		FontName.ReplaceInline(TEXT("_"), TEXT(""));
		
		UTexture2DArray* FontTextureArray = SDFFont->FontTextureArray;
		if (FontTextureArray == nullptr)
		{
			const FString SanitizedFrameName = ObjectTools::SanitizeObjectName(FontName);
			const FString TentativePackagePath = UPackageTools::SanitizePackageName(LongPackagePath + TEXT("/") + SanitizedFrameName);
			FString DefaultSuffix = TEXT("_FontTextureArray");
			FString AssetName;
			FString PackageName;
			AssetToolsModule.Get().CreateUniqueAssetName(TentativePackagePath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

			// Create a unique package name and asset name
			UObject*  OuterForFontTextures = CreatePackage(*PackageName);

			// Create the asset
			FontTextureArray = NewObject<UTexture2DArray>(OuterForFontTextures, *AssetName, RF_Public | RF_Standalone);
			FAssetRegistryModule::AssetCreated(FontTextureArray);
		}

		FontTextureArray->Modify();
		FontTextureArray->AddressX = TextureAddress::TA_Clamp;
		FontTextureArray->AddressY = TextureAddress::TA_Clamp;
		FontTextureArray->Filter = TextureFilter::TF_Bilinear;
		FontTextureArray->LODGroup = TextureGroup::TEXTUREGROUP_ColorLookupTable;
		FontTextureArray->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		FontTextureArray->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
		FontTextureArray->NeverStream = true;

		TArray<UTexture2D*> SourceTextures;
		for (int32 Index = 0, Count = AtlasPacker.AtlasDataList.Num(); Index < Count; ++Index)
		{
			if (FontTextureArray->SourceTextures.IsValidIndex(Index))
			{
				SourceTextures.Add(FontTextureArray->SourceTextures[Index]);
			}
			else
			{
				break;
			}
		}

		TArray<UTexture2D*> DeletedTextures;
		for (int32 Index = AtlasPacker.AtlasDataList.Num(), Count = FontTextureArray->SourceTextures.Num(); Index < Count; ++Index)
		{
			if (!SourceTextures.Contains(FontTextureArray->SourceTextures[Index]))
			{
				DeletedTextures.Add(FontTextureArray->SourceTextures[Index]);
			}
		}
		
		FontTextureArray->SourceTextures.Empty();

		for (int32 Index = 0, Count = AtlasPacker.AtlasDataList.Num(); Index < Count; ++Index)
		{
			UTexture2D* FontTexture = SourceTextures.IsValidIndex(Index) ? SourceTextures[Index] : nullptr;
			if (!FontTexture)
			{
				const FString SanitizedFrameName = ObjectTools::SanitizeObjectName(FontName);
				const FString TentativePackagePath = UPackageTools::SanitizePackageName(TargetSubPath + TEXT("/") + SanitizedFrameName);
				FString DefaultSuffix = TEXT("_FontTexture");
				FString AssetName;
				FString PackageName;
				AssetToolsModule.Get().CreateUniqueAssetName(TentativePackagePath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

				// Create a unique package name and asset name
				UObject* OuterForFontTexture = CreatePackage(*PackageName);

				// Create the asset
				FontTexture = NewObject<UTexture2D>(OuterForFontTexture, *AssetName, RF_Public | RF_Standalone);
				FAssetRegistryModule::AssetCreated(FontTexture);
			}

			FontTexture->Modify();

			FontTexture->Source.Init(
					AtlasPacker.GetAtlasWidth(),
					AtlasPacker.GetAtlasHeight(),
					/*NumSlices=*/ 1,
					1,
					ETextureSourceFormat::TSF_G8,
					AtlasPacker.AtlasDataList[Index].GetData()
				);

			FontTexture->AddressX = TextureAddress::TA_Clamp;
			FontTexture->AddressY = TextureAddress::TA_Clamp;
			FontTexture->Filter = TextureFilter::TF_Bilinear;
			FontTexture->LODGroup = TextureGroup::TEXTUREGROUP_ColorLookupTable;
			FontTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			FontTexture->CompressionSettings = TextureCompressionSettings::TC_Grayscale;
			FontTexture->NeverStream = true;
			FontTexture->SRGB = false;
		
			FontTexture->UpdateResource();
			
			FontTextureArray->SourceTextures.Add(FontTexture);

			ObjectsToSync.Add(FontTexture);
		}

		ObjectsToSync.Add(FontTextureArray);
		FontTextureArray->UpdateSourceFromSourceTextures();
		FontTextureArray->UpdateResource();
		
		SDFFont->FontTextureArray = FontTextureArray;

		for (auto& Texture : DeletedTextures)
		{
			UEditorAssetLibrary::DeleteLoadedAsset(Texture);
		}
	}

	if (ObjectsToSync.Num() > 0)
	{
		ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
	}

	if (MessageLogListing.IsValid())
	{
		MessageLogListing->AddMessage(FTokenizedMessage::Create(EMessageSeverity::Info, LOCTEXT("GenerateSDFFontResult", "Generate SDF font successfully")), false);
	}
#endif

	bRefreshTextComponent = true;
}

bool FSDFFontEditor::OnGenerateSDFFontEnabled() const
{
	return SDFFont && SDFFont->Charsets.Num() > 0;
}

void FSDFFontEditor::AddWarningMessage(FText Msg)
{
	MessageLogListing->AddMessage(FTokenizedMessage::Create(EMessageSeverity::Warning, Msg), false);
}

void FSDFFontEditor::AddErrorMessage(FText Msg)
{
	MessageLogListing->AddMessage(FTokenizedMessage::Create(EMessageSeverity::Error, Msg), false);
}

void FSDFFontEditor::BeginSlowTask(bool bShowCancelButton, bool bShowProgressDialog)
{
	GWarn->BeginSlowTask(LOCTEXT("GenerateSDFFontProgress", "Generating SDF font characters ..."), bShowProgressDialog, bShowCancelButton);
}

void FSDFFontEditor::UpdateProgress(int32 CurIndex, int32 Count)
{
	GWarn->UpdateProgress(CurIndex, Count);
}

void FSDFFontEditor::EndSlowTask()
{
	GWarn->EndSlowTask();
}

#undef LOCTEXT_NAMESPACE
