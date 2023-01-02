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
#include "RenderObject.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "SoftRendererEditor"

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
// FCreateRenderObjectFromTextureExtension

struct FCreateRenderObjectFromTextureExtension : public FContentBrowserSelectedAssetExtensionBase
{
	FCreateRenderObjectFromTextureExtension()
	{
	}
	
	virtual void Execute() override
	{
		const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& AssetData = *AssetIt;
			if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset()))
			{
				// Create the sprite
				FString Name;
				FString PackageName;
				
				const FString DefaultSuffix = TEXT("_RenderObject");
				AssetToolsModule.Get().CreateUniqueAssetName(StaticMesh->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);

				UPackage* Package = CreatePackage( *PackageName);
				if (ensure(Package))
				{
					UBlueprint* NewBP = CastChecked<UBlueprint>(FKismetEditorUtilities::CreateBlueprint(URenderObject::StaticClass(), Package, FName(Name), EBlueprintType::BPTYPE_Normal, 
			UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass(), NAME_None));

					if (TSubclassOf<UObject> GeneratedClass = NewBP->GeneratedClass)
					{
						if (URenderObject* RenderObject = GeneratedClass->GetDefaultObject<URenderObject>())
						{
							const FStaticMeshLODResources& StaticMeshLOD = StaticMesh->RenderData->LODResources[0];
							
							const int32 NumWedges = StaticMeshLOD.IndexBuffer.GetNumIndices();
							const int32 NumVertexPositions = StaticMeshLOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();

							//Create the vertex
							for (int32 VertexIndex = 0; VertexIndex < NumVertexPositions; ++VertexIndex)
							{
								FRenderObjectVertex Vertex;
								Vertex.Position = StaticMeshLOD.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
								RenderObject->Vertices.Emplace(Vertex);
							}

							for (int32 Index = 0; Index < NumWedges; ++Index)
							{
								RenderObject->Indices.Add(StaticMeshLOD.IndexBuffer.GetIndex(Index));
							}

							RenderObject->Material.VertexShaderClass = UVertexShader::StaticClass();
						}
					}

					FBlueprintEditorUtils::MarkBlueprintAsModified(NewBP);
					
					// Need to make sure we compile with the new source code
					// FKismetEditorUtilities::CompileBlueprint(NewBP);
				}

			}
		}
 
	}
};

//////////////////////////////////////////////////////////////////////////
// FSoftRendererContentBrowserExtensions_Impl

class FSoftRendererContentBrowserExtensions_Impl
{
public:
	static void ExecuteSelectedContentFunctor(TSharedPtr<FContentBrowserSelectedAssetExtensionBase> SelectedAssetFunctor)
	{
		SelectedAssetFunctor->Execute();
	}

	static void CreateSoftRendererActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("SoftRendererActionsSubMenuLabel", "SoftRenderer Actions"),
			LOCTEXT("SoftRendererActionsSubMenuToolTip", "SoftRenderer-related actions for this static mesh."),
			FNewMenuDelegate::CreateStatic(&FSoftRendererContentBrowserExtensions_Impl::PopulateSoftRendererActionsMenu, SelectedAssets),
			false
		);
	}

	static void PopulateSoftRendererActionsMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
	{
		const TSharedPtr<FCreateRenderObjectFromTextureExtension> RenderObjectCreatorFunctor = MakeShareable(new FCreateRenderObjectFromTextureExtension());
		RenderObjectCreatorFunctor->SelectedAssets = SelectedAssets;

		const FUIAction Action_CreateRenderObjectsFromTextures(
			FExecuteAction::CreateStatic(&FSoftRendererContentBrowserExtensions_Impl::ExecuteSelectedContentFunctor, StaticCastSharedPtr<FContentBrowserSelectedAssetExtensionBase>(RenderObjectCreatorFunctor)));
		
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CB_Extension_Texture_CreateSoftRenderer", "Create RenderObject"),
			LOCTEXT("CB_Extension_Texture_CreateSoftRenderer_Tooltip", "Create RenderObject from selected static meshes"),
			FSlateIcon(FEditorStyle::Get().GetStyleSetName(), "AssetActions.CreateSoftRenderer"),
			Action_CreateRenderObjectsFromTextures,
			NAME_None,
			EUserInterfaceActionType::Button);
		
	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		// Run thru the assets to determine if any meet our criteria
		bool bAnyStaticMeshes = false;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			bAnyStaticMeshes = bAnyStaticMeshes || (Asset.AssetClass == UStaticMesh::StaticClass()->GetFName());
		}

		if (bAnyStaticMeshes)
		{
			// Add the sprite actions sub-menu extender
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FSoftRendererContentBrowserExtensions_Impl::CreateSoftRendererActionsSubMenu, SelectedAssets));
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

void FSoftRendererContentBrowserExtensions::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FSoftRendererContentBrowserExtensions_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FSoftRendererContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FSoftRendererContentBrowserExtensions::RemoveHooks()
{
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FSoftRendererContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate){ return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE