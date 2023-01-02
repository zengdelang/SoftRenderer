#include "SSpriteAtlasPackerSettings.h"
#include "Misc/FeedbackContext.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Sprite2D.h"
#include "SSpriteAtlasPackerSpriteListItem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ObjectTools.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

#define LOCTEXT_NAMESPACE "SSpriteAtlasPackerSettings"

void SSpriteAtlasPackerSettings::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList)
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::ENameAreaSettings::HideNameArea, false, nullptr, false, NAME_None);
	TSharedRef<IDetailsView> DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	
	DetailsView->SetObject(FSpriteAtlasPacker::Get().GetPackerSettings(), true);
	//DetailsView->SetIsPropertyEditingEnabledDelegate(IsPropertyEditingEnabled);

	ChildSlot
	[
		SNew( SVerticalBox )
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			DetailsView
		]
		+ SVerticalBox::Slot()
		.Padding(4.0f)
		.AutoHeight()
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.Text(LOCTEXT("ImportSprites", "Import Sprites"))
			.OnClicked(this, &SSpriteAtlasPackerSettings::OnImportSprites)
		]
		+ SVerticalBox::Slot()
        .Padding(4.0f)
        .AutoHeight()
        [
        	SNew(SButton)
        	.HAlign(HAlign_Center)
        	.ToolTipText(LOCTEXT("RemoveRedundantSprite2DAssetsTip", "Remove redundant Sprite2D assets under the import path that are not in the import sprite list."))
        	.Text(LOCTEXT("RemoveRedundantSprite2DAssets", "Remove Redundant Assets"))
        	.OnClicked(this, &SSpriteAtlasPackerSettings::OnRemoveRedundantSprites)
        ]
	];
}

FReply SSpriteAtlasPackerSettings::OnImportSprites() const
{
	if (FSpriteAtlasPacker::Get().GetRootSprites().Num() == 0 || FSpriteAtlasPacker::Get().GetPackerSettings() == nullptr)
		return FReply::Handled();

	FString ImportPath = FSpriteAtlasPacker::Get().GetPackerSettings()->ImportPath;

	FText Reason;
	if (!FPaths::ValidatePath(ImportPath, &Reason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return FReply::Handled();
	}

	if (!ImportPath.StartsWith(TEXT("/Game/")))
	{
		Reason = LOCTEXT("PathStartsWithError", "Path does not start with /Game/");
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return FReply::Handled();
	}
	
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	FPaths::RemoveDuplicateSlashes(ImportPath);
	if (ImportPath.EndsWith(TEXT("/")))
	{
		ImportPath.RemoveAt(ImportPath.Len() - 1);
	}

	FARFilter Filter;
	Filter.ClassNames.Add(USprite2D::StaticClass()->GetFName());
	Filter.PackagePaths.Emplace(*ImportPath);

	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	TMap<FName, FAssetData> AssetMap;
	for (const auto& Asset : AssetList)
	{
		AssetMap.Add(Asset.PackageName, Asset);
	}

	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	constexpr bool bShowCancelButton = false;
	constexpr bool bShowProgressDialog = true;
	GWarn->BeginSlowTask(LOCTEXT("GeneratingSprite2DProgress", "Generating Sprite2D ..."), bShowProgressDialog, bShowCancelButton);
	
	TArray<UObject*> ObjectsToSync;

	int32 Index = 0;
	const auto Settings = FSpriteAtlasPacker::Get().GetPackerSettings();
	for (const auto& Elem : FSpriteAtlasPacker::Get().GetRootSprites())
	{
		GWarn->UpdateProgress(Index, AssetList.Num());
		++Index;
		
		const FString RealSpriteName = GetRealSpriteName(Elem->Name);
		const FString Sprite2DPackagePath = ImportPath + TEXT("/") + RealSpriteName;
		USprite2D* Sprite2D = nullptr;
		
		const auto AssetPtr = AssetMap.Find(FName(Sprite2DPackagePath));
		if (AssetPtr)
		{
			Sprite2D = Cast<USprite2D>(AssetPtr->GetAsset());
		}

		if (!IsValid(Sprite2D))
		{
			Sprite2D = GenerateSprite2D(ImportPath, Elem, ObjectsToSync);
		}

		UTexture2D* SpriteTexture = Sprite2D->GetSourceTexture();
		if (!IsValid(SpriteTexture))
		{
			SpriteTexture = GenerateTexture2D(ImportPath, Elem, ObjectsToSync);
		}

		{
			SpriteTexture->Modify();
			if (SpriteTexture)
			{
				SpriteTexture->Source.Init(
					Elem->Width,
					Elem->Height,
					/*NumSlices=*/ 1,
					1,
					ETextureSourceFormat::TSF_BGRA8,
					Elem->RawData.GetData()
				);
			}
		
			SpriteTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
			SpriteTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
			SpriteTexture->CompressionSettings = TC_EditorIcon;
			SpriteTexture->CompressionQuality = ETextureCompressionQuality::TCQ_Highest;
			SpriteTexture->NeverStream = true;
			SpriteTexture->SRGB = true;

#if USE_DYNAMIC_UI_ALTAS
			SpriteTexture->Sprite2DTextureType = CPU;
#endif
						
			SpriteTexture->UpdateResource();
			SpriteTexture->MarkPackageDirty();
		}

		{
			Sprite2D->Modify();

			Sprite2D->SetImportSettings(Settings->bUseDynamicAtlas, FVector2D(Elem->Width, Elem->Height),
				SpriteTexture, Settings->AtlasName, Settings->PixelsPerUnrealUnit);
		
			Sprite2D->MarkPackageDirty();
		}

		if (Sprite2D->GetIsMerged())
		{
			Sprite2D->RemergeSprite2D();
		}
	}

	if (ObjectsToSync.Num() > 0)
	{
		ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
	}

	GWarn->EndSlowTask();
	
	return FReply::Handled();
}

FReply SSpriteAtlasPackerSettings::OnRemoveRedundantSprites() const
{
	const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo,
		LOCTEXT("RemoveRedundantSpritesAction", "Remove redundant Sprite2D assets under the import path ?"));
	if (Result != EAppReturnType::Yes)
		return FReply::Handled();

	FString ImportPath = FSpriteAtlasPacker::Get().GetPackerSettings()->ImportPath;

	FText Reason;
	if (!FPaths::ValidatePath(ImportPath, &Reason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return FReply::Handled();
	}

	if (!ImportPath.StartsWith(TEXT("/Game/")))
	{
		Reason = LOCTEXT("PathStartsWithError", "Path does not start with /Game/");
		FMessageDialog::Open(EAppMsgType::Ok, Reason);
		return FReply::Handled();
	}

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	FPaths::RemoveDuplicateSlashes(ImportPath);
	if (ImportPath.EndsWith(TEXT("/")))
	{
		ImportPath.RemoveAt(ImportPath.Len() - 1);
	}

	FARFilter Filter;
	Filter.ClassNames.Add(USprite2D::StaticClass()->GetFName());
	Filter.PackagePaths.Emplace(*ImportPath);

	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetList);

	TMap<FName, FAssetData> AssetMap;
	for (const auto& Asset : AssetList)
	{
		AssetMap.Add(Asset.PackageName, Asset);
	}

	for (const auto& Elem : FSpriteAtlasPacker::Get().GetRootSprites())
	{
		const FString RealSpriteName = GetRealSpriteName(Elem->Name);
		const FString Sprite2DPackagePath = ImportPath + TEXT("/") + RealSpriteName;

		AssetMap.Remove(FName(Sprite2DPackagePath));
	}

	TArray<UObject*> AssetsToDelete;
	for (const auto& AssetElem : AssetMap)
	{
		USprite2D* Sprite2D = Cast<USprite2D>(AssetElem.Value.GetAsset());
		if (Sprite2D)
		{
			AssetsToDelete.Add(Sprite2D);
			if (Sprite2D->GetSourceTexture())
			{
				AssetsToDelete.Add(Sprite2D->GetSourceTexture());
			}
		}
	}

	if (AssetsToDelete.Num() > 0)
	{
		ObjectTools::DeleteObjects(AssetsToDelete);
	}

	return FReply::Handled();
}

FString SSpriteAtlasPackerSettings::GetRealSpriteName(FString InName)
{
	int32 CharIndex = InName.Len() - 1;
	while (CharIndex >= 0 && InName[CharIndex] >= TEXT('0') && InName[CharIndex] <= TEXT('9'))
	{
		--CharIndex;
	}
 
	if (InName.Len() > 0 && CharIndex == -1)
	{
		return FString::Printf(TEXT("%s%d"), *(InName + TEXT("_")), 2);
	}

	return InName;
}

UTexture2D* SSpriteAtlasPackerSettings::GenerateTexture2D(const FString& ImportPath, const TSharedPtr<class FSpriteItem>& SpriteItem, TArray<UObject*>& ObjectsToSync) const
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	
	UTexture2D* Texture = nullptr;
	
	const FString TexturePackagePath = ImportPath + TEXT("/Textures/T_") + SpriteItem->Name;
			
	const FString DefaultSuffix;
	FString AssetName;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(TexturePackagePath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);
			
	// Create a unique package name and asset name
	UObject* OuterForTexture = CreatePackage(*PackageName);

	// Create the asset
	Texture = NewObject<UTexture2D>(OuterForTexture, *AssetName, RF_Public | RF_Standalone);
	FAssetRegistryModule::AssetCreated(Texture);
	
	ObjectsToSync.Add(Texture);
	return Texture;
}

USprite2D* SSpriteAtlasPackerSettings::GenerateSprite2D(const FString& ImportPath, const TSharedPtr<class FSpriteItem>& SpriteItem, TArray<UObject*>& ObjectsToSync) const
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	
	const FString Sprite2DPackagePath = ImportPath + TEXT("/") + SpriteItem->Name;
			
	const FString DefaultSuffix;
	FString AssetName;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(Sprite2DPackagePath, DefaultSuffix, /*out*/ PackageName, /*out*/ AssetName);

	// Create a unique package name and asset name
	UObject* OuterForSprite2D = CreatePackage(*PackageName);

	// Create the asset
	USprite2D* Sprite2D = NewObject<USprite2D>(OuterForSprite2D, *AssetName, RF_Public | RF_Standalone);
	FAssetRegistryModule::AssetCreated(Sprite2D);
			
	ObjectsToSync.Add(Sprite2D);
	return Sprite2D;
}

#undef LOCTEXT_NAMESPACE
