#include "Sprite2DEditorModule.h"
#include "ContentBrowserModule.h"
#include "ObjectTools.h"
#include "IContentBrowserSingleton.h"
#include "LevelEditor.h"
#include "PaperSprite.h"
#include "Sprite2D.h"
#include "Sprite2DEditorCommands.h"
#include "ToolMenuMisc.h"
#include "AtlasVisualizer/SSpriteAtlasVisualizer.h"
#include "AtlasPacker/SSpriteAtlasPacker.h"
#include "AssetTypeAction/AssetTypeAction_Sprite2D.h"
#include "ContentBrowserExtensions/ContentBrowserExtensions.h"
#include "ThumbnailRenderer/Sprite2DThumbnailRenderer.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AtlasPacker/SpriteAtlasPackerPrivate.h"
#include "AtlasPacker/SpriteAtlasPackerSettingsDetails.h"
#include "Sprite2D/Public/SpriteMergeSubsystem.h"
#include "Core/WidgetActor.h"
#include "Factories/TextureFactory.h"
#include "Factory/Sprite2DFactory.h"
#include "Misc/FileHelper.h"
#include "Misc/FeedbackContext.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

#define LOCTEXT_NAMESPACE "FSprite2DEditorModule"

DEFINE_LOG_CATEGORY(LogSprite2DEditor);

static const FName Sprite2DAtlasVisualizerTabName("Sprite2DAtlasVisualizer");
static const FName Sprite2DAtlasPackerTabName("Sprite2DAtlasPacker");

void FSprite2DEditorModule::StartupModule()
{
	FSpriteAtlasPacker::Initialize();
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSprite2DEditorModule::OnPostEngineInit);
	
	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	const auto AssetCategoryBit = AssetToolsModule.RegisterAdvancedAssetCategory(FName(TEXT("UGUI")),
		LOCTEXT("UGUICategory", "UGUI"));
	
	const TSharedPtr<FAssetTypeAction_Sprite2D> SpriteActionType = MakeShareable(new FAssetTypeAction_Sprite2D(AssetCategoryBit));
	ItemDataAssetTypeActions.Add(SpriteActionType);
	AssetToolsModule.RegisterAssetTypeActions(SpriteActionType.ToSharedRef());

	if (!IsRunningCommandlet())
	{
		FSprite2DContentBrowserExtensions::InstallHooks();
	}
	
	FSprite2DEditorCommands::Register();
	
	const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedRef<FUICommandList> CommandBindings = LevelEditorModule.GetGlobalLevelEditorActions();
	CommandBindings->MapAction(
	 	FSprite2DEditorCommands::Get().OpenSprite2DAtlasVisualizer,
	 	FExecuteAction::CreateRaw(this, &FSprite2DEditorModule::OpenSprite2DAtlasVisualizer),
	 	FCanExecuteAction());

	CommandBindings->MapAction(
		 FSprite2DEditorCommands::Get().OpenSprite2DAtlasPacker,
		 FExecuteAction::CreateRaw(this, &FSprite2DEditorModule::OpenSprite2DAtlasPacker),
		 FCanExecuteAction());

	CommandBindings->MapAction(
		 FSprite2DEditorCommands::Get().ClearSprite2DAtlas,
		 FExecuteAction::CreateRaw(this, &FSprite2DEditorModule::ClearSprite2DAtlas),
		 FCanExecuteAction());
	

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSprite2DEditorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Sprite2DAtlasVisualizerTabName, FOnSpawnTab::CreateRaw(this, &FSprite2DEditorModule::MakeTextureAtlasVisualizerTab))
			.SetDisplayName(LOCTEXT("FSprite2DAtlasVisualizerTitle", "Sprite2D Atlas Visualizer"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(Sprite2DAtlasPackerTabName, FOnSpawnTab::CreateRaw(this, &FSprite2DEditorModule::MakeSpriteAtlasPackerTab))
			.SetDisplayName(LOCTEXT("FSprite2DAtlasPackerTitle", "Sprite2D Atlas Packer"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

	if (!IsRunningCommandlet() && !IsRunningGame() && FSlateApplication::IsInitialized())
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBFolderMenuExtenderDelegates = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
		
		CBFolderMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSprite2DEditorModule::OnExtendContentBrowserDirectoryExtensionMenu));
		ContentBrowserDirectoryExtensionDelegateHandle = CBFolderMenuExtenderDelegates.Last().GetHandle();
	}

	//register custom editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		PropertyModule.RegisterCustomClassLayout(USpriteAtlasPackerSettings::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FSpriteAtlasPackerSettingsDetails::MakeInstance));
	}
}

void FSprite2DEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	if (UObjectInitialized())
	{
		UThumbnailManager::Get().UnregisterCustomRenderer(USprite2D::StaticClass());
	}

	if (!IsRunningCommandlet() && !IsRunningGame() && !IsRunningDedicatedServer())
	{
		FContentBrowserModule* ContentBrowserModule = FModuleManager::GetModulePtr<FContentBrowserModule>(TEXT("ContentBrowser"));
		if (ContentBrowserModule)
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			CBMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserDirectoryExtensionDelegateHandle; });
		}

		// remove menu extension
		UToolMenus::UnregisterOwner(this);
		FCoreDelegates::OnPostEngineInit.RemoveAll(this);

		UPackage::PackageSavedEvent.RemoveAll(this);
	}
	
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : ItemDataAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
	ItemDataAssetTypeActions.Empty();
	
	FSprite2DContentBrowserExtensions::RemoveHooks();

	FSprite2DEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Sprite2DAtlasVisualizerTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Sprite2DAtlasPackerTabName);

	FSpriteAtlasPacker::Shutdown();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout(USpriteAtlasPackerSettings::StaticClass()->GetFName());
}

void FSprite2DEditorModule::OnPostEngineInit()
{
	if (UObjectInitialized())
	{
		// Register the thumbnail renderers
		UThumbnailManager::Get().RegisterCustomRenderer(USprite2D::StaticClass(), USprite2DThumbnailRenderer::StaticClass());
	}
}

void FSprite2DEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	const FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedRef<FUICommandList> CommandBindings = LevelEditorModule.GetGlobalLevelEditorActions();
	
	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("MainFrame.MainMenu.Window");
		FToolMenuSection& Section = Menu->AddSection("Sprite2DTabSpawners", LOCTEXT("Sprite2DTabSpawnersHeading", "Sprite 2D"), FToolMenuInsert("WindowGlobalTabSpawners", EToolMenuInsertType::After));
		Section.AddMenuEntryWithCommandList(FSprite2DEditorCommands::Get().OpenSprite2DAtlasVisualizer, CommandBindings);
	}

	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("MainFrame.MainMenu.Window");
		FToolMenuSection& Section = Menu->AddSection("Sprite2DTabSpawners", LOCTEXT("Sprite2DTabSpawnersHeading", "Sprite 2D"), FToolMenuInsert("WindowGlobalTabSpawners", EToolMenuInsertType::After));
		Section.AddMenuEntryWithCommandList(FSprite2DEditorCommands::Get().OpenSprite2DAtlasPacker, CommandBindings);
	}

	{
		UToolMenu* Menu = UToolMenus::Get()->RegisterMenu("MainFrame.MainMenu.Window");
		FToolMenuSection& Section = Menu->AddSection("Sprite2DTabSpawners", LOCTEXT("Sprite2DTabSpawnersHeading", "Sprite 2D"), FToolMenuInsert("WindowGlobalTabSpawners", EToolMenuInsertType::After));
		Section.AddMenuEntryWithCommandList(FSprite2DEditorCommands::Get().ClearSprite2DAtlas, CommandBindings);
	}
}

void FSprite2DEditorModule::OpenSprite2DAtlasVisualizer()
{
	FGlobalTabmanager::Get()->TryInvokeTab(Sprite2DAtlasVisualizerTabName);
}

void FSprite2DEditorModule::OpenSprite2DAtlasPacker()
{
	FGlobalTabmanager::Get()->TryInvokeTab(Sprite2DAtlasPackerTabName);
}

void FSprite2DEditorModule::ClearSprite2DAtlas()
{
	USpriteMergeSubsystem* Subsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->ClearSprite2DAtlas();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////// AtlasVisualizer

TSharedRef<SWidget> FSprite2DEditorModule::GetAtlasVisualizer()
{
	return SNew(SSpriteAtlasVisualizer);
}

TSharedRef<SWidget> FSprite2DEditorModule::GetTextureAtlasVisualizer()
{
	return GetAtlasVisualizer();
}

TSharedRef<SDockTab> FSprite2DEditorModule::MakeTextureAtlasVisualizerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			GetTextureAtlasVisualizer()
		];
}

TSharedRef<SDockTab> FSprite2DEditorModule::MakeSpriteAtlasPackerTab(const FSpawnTabArgs& SpawnTabArgs) const
{
	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	const TSharedPtr<SWidget> TabContent = SNew(SSpriteAtlasPacker, MajorTab, SpawnTabArgs.GetOwnerWindow());
	MajorTab->SetContent(TabContent.ToSharedRef());

	return MajorTab;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Extend content browser menu for applying UI setting for textures
TSharedRef<FExtender> FSprite2DEditorModule::OnExtendContentBrowserDirectoryExtensionMenu(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"PathContextBulkOperations",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FSprite2DEditorModule::DirectoryExtensionContentBrowserMenu, SelectedPaths));

	return Extender;
}

void FSprite2DEditorModule::DirectoryExtensionContentBrowserMenu(FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths) const
{
	MenuBuilder.BeginSection(TEXT("Sprite2D"), NSLOCTEXT("Sprite2DContentBrowerMenu", "Sprite2D", "Sprite 2D"));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ApplyUISettingForTexturesTitle", "Apply UI setting for textures"),
		LOCTEXT("ApplyUISettingForTexturesTooltipText", "Apply UI setting on the textures in the selected folder."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this, SelectedPaths]
		{
			/*const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("", ""));
			if (Result != EAppReturnType::Yes)
				return;*/
			
			const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

			// Form a filter from the paths
			FARFilter Filter;
			Filter.ClassNames.Add(UTexture2D::StaticClass()->GetFName());
			Filter.bRecursivePaths = true;
			for (const FString& Folder : SelectedPaths)
			{
				Filter.PackagePaths.Emplace(*Folder);
			}

			// Query for a list of assets in the selected paths
			TArray<FAssetData> AssetList;
			AssetRegistryModule.Get().GetAssets(Filter, AssetList);

			constexpr bool bShowCancelButton = false;
			const bool bShowProgressDialog = AssetList.Num() > 30;
			GWarn->BeginSlowTask(LOCTEXT("ApplyUISettingForTexturesProgress", "Appling UI setting to Texture2D"), bShowProgressDialog, bShowCancelButton);

			int32 Index = 0;
			for (const FAssetData& AssetData: AssetList)
			{
				GWarn->UpdateProgress(Index, AssetList.Num());
				++Index;
				
				UTexture2D* Texture2D = Cast<UTexture2D>(AssetData.GetAsset());
				if (Texture2D)
				{
					Texture2D->Modify();

					Texture2D->CompressionQuality = ETextureCompressionQuality::TCQ_Highest;
					Texture2D->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Texture2D->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
					Texture2D->NeverStream = true;
					Texture2D->SRGB = true;
					Texture2D->CompressionSettings = TC_EditorIcon;

					Texture2D->PostEditChange();
				}
			}

			GWarn->EndSlowTask();
		}))
	);

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("CovertPaperSpriteToSprite2DTitle", "Covert PaperSprite to Sprite2D"),
		LOCTEXT("CovertPaperSpriteToSprite2DTooltipText", "Convert PaperSprite to Sprite2D in the selected folder."),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([this, SelectedPaths]
		{
			const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
			const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
			const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			
			// Form a filter from the paths
			FARFilter Filter;
			Filter.ClassNames.Add(UPaperSprite::StaticClass()->GetFName());
			Filter.bRecursivePaths = true;
			for (const FString& Folder : SelectedPaths)
			{
				Filter.PackagePaths.Emplace(*Folder);
			}

			// Query for a list of assets in the selected paths
			TArray<FAssetData> AssetList;
			AssetRegistryModule.Get().GetAssets(Filter, AssetList);

			TArray<UObject*> ObjectsToSync;

			constexpr bool bShowCancelButton = false;
			const bool bShowProgressDialog = AssetList.Num() > 5;
			GWarn->BeginSlowTask(LOCTEXT("CovertPaperSpriteToSprite2DProgress", "Covert PaperSprite to Sprite2D"), bShowProgressDialog, bShowCancelButton);
			
			int32 Index = 0;
			for (const FAssetData& AssetData: AssetList)
			{
				GWarn->UpdateProgress(Index, AssetList.Num());
				++Index;

				const UPaperSprite* PaperSprite = Cast<UPaperSprite>(AssetData.GetAsset());
				if (PaperSprite)
				{
					const auto PaperSpriteSource = PaperSprite->GetSourceTexture();
					if (PaperSpriteSource && PaperSpriteSource->PlatformData && PaperSpriteSource->GetPixelFormat() == PF_B8G8R8A8)
					{
						FString NewAssetName = AssetData.AssetName.ToString();
						NewAssetName = NewAssetName.Replace(TEXT("_png"), TEXT(""));
												
						const FString PackagePath = AssetData.PackagePath.ToString().Replace(TEXT("/Frames"), TEXT("")) + TEXT("/") + NewAssetName;
						FString TexturePackagePath = AssetData.PackagePath.ToString().Replace(TEXT("/Frames"), TEXT("")) + TEXT("/Textures/") + (NewAssetName.StartsWith(TEXT("T_")) ? TEXT("") : TEXT("T_")) + NewAssetName;
												
						const FString DefaultSuffix;
						FString AssetName;
						FString PackageName;
						AssetToolsModule.Get().CreateUniqueAssetName(TexturePackagePath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

						// Create a unique package name and asset name
						UObject* OuterForTexture = CreatePackage(*PackageName);

						// Create the asset
						UTexture2D* Texture = NewObject<UTexture2D>(OuterForTexture, *AssetName, RF_Public | RF_Standalone);
						FAssetRegistryModule::AssetCreated(Texture);

						Texture->Modify();
 
						ObjectsToSync.Add(Texture);

						TArray64<uint8> RawData;
						RawData.Reserve(PaperSprite->GetSourceSize().X * PaperSprite->GetSourceSize().Y * sizeof(uint32));

						uint8* TextureRawData = (uint8*)PaperSpriteSource->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
						
						const int32 Width = PaperSprite->GetSourceSize().X;
						const int32 Height = PaperSprite->GetSourceSize().Y;
						const FVector2D SourceUV = PaperSprite->GetSourceUV();
						for (int32 Y = 0; Y < Height; ++Y)
						{
							const uint32 Offset = sizeof(uint32) * ((SourceUV.Y + Y) * PaperSpriteSource->GetSizeX() + SourceUV.X);
							FMemory::Memcpy(RawData.GetData() + Y * (sizeof(uint32) * Width), TextureRawData + Offset, sizeof(uint32) * Width);
						}
 
						PaperSpriteSource->PlatformData->Mips[0].BulkData.Unlock();
						
						if (Texture)
						{
							Texture->Source.Init(
								Width,
								Height,
								/*NumSlices=*/ 1,
								1,
								ETextureSourceFormat::TSF_BGRA8,
								RawData.GetData()
							);
						}
		
						Texture->LODGroup = PaperSpriteSource->LODGroup;
						Texture->MipGenSettings = PaperSpriteSource->MipGenSettings;
						Texture->CompressionSettings = PaperSpriteSource->CompressionSettings;
						Texture->CompressionQuality = PaperSpriteSource->CompressionQuality;
						Texture->NeverStream = PaperSpriteSource->NeverStream;;
						Texture->SRGB = PaperSpriteSource->SRGB;;

#if USE_DYNAMIC_UI_ALTAS
						Texture->Sprite2DTextureType = CPU;
#endif
						
						Texture->UpdateResource();

						{
							// Create the factory used to generate the sprite
							USprite2DFactory* SpriteFactory = NewObject<USprite2DFactory>();
							SpriteFactory->InitialTexture = Texture;

							// Create the sprite
							FString Sprite2DName;
							FString Sprite2DPackageName;

							// Get a unique name for the sprite
							const FString Sprite2DDefaultSuffix = TEXT("");
							AssetToolsModule.Get().CreateUniqueAssetName(PackagePath, Sprite2DDefaultSuffix, /*out*/ Sprite2DPackageName, /*out*/ Sprite2DName);
							const FString Sprite2DPackagePath = FPackageName::GetLongPackagePath(Sprite2DPackageName);

							if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Sprite2DName, Sprite2DPackagePath, USprite2D::StaticClass(), SpriteFactory))
							{
								ObjectsToSync.Add(NewAsset);

								TArray<FName> Referencers;
								AssetRegistryModule.Get().GetReferencers(AssetData.PackageName, Referencers);
								
								{
									// Form a filter from the paths
									FARFilter ReferencersFilter;
									Filter.ClassNames.Add(AWidgetActor::StaticClass()->GetFName());
									Filter.bRecursiveClasses = true;
									for (const auto& Referencer : Referencers)
									{
										ReferencersFilter.PackageNames.Add(Referencer);
									}

									// Query for a list of assets in the selected paths
									TArray<FAssetData> ReferencersAssetList;
									AssetRegistryModule.Get().GetAssets(ReferencersFilter, ReferencersAssetList);

									for (const FAssetData& ReferencersAssetData: ReferencersAssetList)
									{
										ReferencersAssetData.GetAsset();
									}
								}
	 
								TArray<UObject*> ObjectsToReplace;
								ObjectsToReplace.Add(AssetData.GetAsset());
 
								ObjectTools::ForceReplaceReferences(NewAsset, ObjectsToReplace);

								//UEditorAssetLibrary::DeleteLoadedAsset(AssetData.GetAsset());
								
								//NewAsset->Rename(*NewAssetName);
								AssetRegistryModule.Get().AssetRenamed(NewAsset, AssetData.PackageName.ToString());
							}
						}
					}
				}
			}
			
			if (ObjectsToSync.Num() > 0)
			{
				ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
			}

			GWarn->EndSlowTask();
		}))
	);
	
 	MenuBuilder.EndSection();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSprite2DEditorModule, Sprite2DEditor)
