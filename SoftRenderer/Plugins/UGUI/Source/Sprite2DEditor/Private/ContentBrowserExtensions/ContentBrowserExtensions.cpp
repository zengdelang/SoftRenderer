#include "ContentBrowserExtensions.h"
#include "Modules/ModuleManager.h"
#include "Misc/PackageName.h"
#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "EditorStyleSet.h"
#include "Engine/Texture2D.h"
#include "AssetToolsModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "Sprite2D.h"
#include "Factory/Sprite2DFactory.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

#define LOCTEXT_NAMESPACE "Sprite2D"

DECLARE_LOG_CATEGORY_EXTERN(LogSprite2DExtensions, Log, All);
DEFINE_LOG_CATEGORY(LogSprite2DExtensions);

static FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
static FDelegateHandle ContentBrowserExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////
// FContentBrowserSelectedAssetExtensionBase

struct FContentBrowserSelectedAssetExtensionBase
{
public:
	TArray<struct FAssetData> SelectedAssets;

public:
	virtual void Execute() {}
	virtual ~FContentBrowserSelectedAssetExtensionBase() {}
};

//////////////////////////////////////////////////////////////////////////
// FCreateSpriteFromTextureExtension

struct FCreateSpriteFromTextureExtension : public FContentBrowserSelectedAssetExtensionBase
{
	FCreateSpriteFromTextureExtension()
	{
	}

	void CreateSpritesFromTextures(TArray<UTexture2D*>& Textures)
	{
		const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		TArray<UObject*> ObjectsToSync;

		for (auto TextureIt = Textures.CreateConstIterator(); TextureIt; ++TextureIt)
		{
			UTexture2D* Texture = *TextureIt;

			// Create the factory used to generate the sprite
			USprite2DFactory* SpriteFactory = NewObject<USprite2DFactory>();
			SpriteFactory->InitialTexture = Texture;

			// Create the sprite
			FString Name;
			FString PackageName;

			// Get a unique name for the sprite
			const FString DefaultSuffix = TEXT("_Sprite2D");
			AssetToolsModule.Get().CreateUniqueAssetName(Texture->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);
			const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

			if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, PackagePath, USprite2D::StaticClass(), SpriteFactory))
			{
				ObjectsToSync.Add(NewAsset);
			}
		}

		if (ObjectsToSync.Num() > 0)
		{
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
		}
	}

	virtual void Execute() override
	{
		// Create sprites for any selected textures
		TArray<UTexture2D*> Textures;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset()))
			{
				Textures.Add(Texture);
			}
		}

		CreateSpritesFromTextures(Textures);
	}
};

struct FConfigureTexturesForSpriteUsageExtension : public FContentBrowserSelectedAssetExtensionBase
{
	virtual void Execute() override
	{
		// Change the compression settings and trigger a recompress
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset()))
			{
				if (Texture->IsNormalMap())
				{
					// Leave normal maps alone
				}
				else
				{
					Texture->Modify();

					Texture->CompressionQuality = ETextureCompressionQuality::TCQ_Highest;
					Texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
					Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
					Texture->NeverStream = true;
					Texture->SRGB = true;
					Texture->CompressionSettings = TC_EditorIcon;

#if USE_DYNAMIC_UI_ALTAS
					Texture->Sprite2DTextureType = CPU;
#endif

					Texture->PostEditChange();
				}
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// FSprite2DContentBrowserExtensions_Impl

class FSprite2DContentBrowserExtensions_Impl
{
public:
	static void ExecuteSelectedContentFunctor(TSharedPtr<FContentBrowserSelectedAssetExtensionBase> SelectedAssetFunctor)
	{
		SelectedAssetFunctor->Execute();
	}

	static void CreateSpriteActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("Sprite2DActionsSubMenuLabel", "Sprite2D Actions"),
			LOCTEXT("Sprite2DActionsSubMenuToolTip", "Sprite2D-related actions for this texture."),
			FNewMenuDelegate::CreateStatic(&FSprite2DContentBrowserExtensions_Impl::PopulateSpriteActionsMenu, SelectedAssets),
			false,
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.PaperSprite")
		);
	}

	static void PopulateSpriteActionsMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		// Create Sprite2D
		TSharedPtr<FCreateSpriteFromTextureExtension> SpriteCreatorFunctor = MakeShareable(new FCreateSpriteFromTextureExtension());
		SpriteCreatorFunctor->SelectedAssets = SelectedAssets;

		FUIAction Action_CreateSpritesFromTextures(
			FExecuteAction::CreateStatic(&FSprite2DContentBrowserExtensions_Impl::ExecuteSelectedContentFunctor, StaticCastSharedPtr<FContentBrowserSelectedAssetExtensionBase>(SpriteCreatorFunctor)));
		
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CB_Extension_Texture_CreateSprite2D", "Create Sprite2D"),
			LOCTEXT("CB_Extension_Texture_CreateSprite2D_Tooltip", "Create Sprite2D from selected textures"),
			FSlateIcon(FEditorStyle::Get().GetStyleSetName(), "AssetActions.CreateSprite2D"),
			Action_CreateSpritesFromTextures,
			NAME_None,
			EUserInterfaceActionType::Button);

		TSharedPtr<FConfigureTexturesForSpriteUsageExtension> TextureConfigFunctor = MakeShareable(new FConfigureTexturesForSpriteUsageExtension());
		TextureConfigFunctor->SelectedAssets = SelectedAssets;
		
		FUIAction Action_ConfigureTexturesForSprites(
			FExecuteAction::CreateStatic(&FSprite2DContentBrowserExtensions_Impl::ExecuteSelectedContentFunctor, StaticCastSharedPtr<FContentBrowserSelectedAssetExtensionBase>(TextureConfigFunctor)));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CB_Extension_Texture_ConfigureTextureForUI", "Apply UI Texture Settings"),
			LOCTEXT("CB_Extension_Texture_ConfigureTextureForUI_Tooltip", "Sets compression settings and texture group to the defaults UI settings"),
			FSlateIcon(FEditorStyle::Get().GetStyleSetName(), "AssetActions.ConfigureForRetroSprites"),
			Action_ConfigureTexturesForSprites,
			NAME_None,
			EUserInterfaceActionType::Button);

	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		// Run thru the assets to determine if any meet our criteria
		bool bAnyTextures = false;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			bAnyTextures = bAnyTextures || (Asset.AssetClass == UTexture2D::StaticClass()->GetFName());
		}

		if (bAnyTextures)
		{
			// Add the sprite actions sub-menu extender
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FSprite2DContentBrowserExtensions_Impl::CreateSpriteActionsSubMenu, SelectedAssets));
		}

		return Extender;
	}

	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}
};

//////////////////////////////////////////////////////////////////////////
// FPaperContentBrowserExtensions

void FSprite2DContentBrowserExtensions::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FSprite2DContentBrowserExtensions_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FSprite2DContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FSprite2DContentBrowserExtensions::RemoveHooks()
{
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FSprite2DContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate){ return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE