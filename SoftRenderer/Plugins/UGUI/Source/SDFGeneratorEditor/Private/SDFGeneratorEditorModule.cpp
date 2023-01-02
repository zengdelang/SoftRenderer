#include "SDFGeneratorEditorModule.h"
#include "AssetToolsModule.h"
#include "SDFGeneratorEditorCommands.h"
#include "AssetTypeAction/AssetTypeAction_SDFFont.h"
#include "Core/Widgets/Text/SDFFontCharset.h"
#include "DetailCustomizations/SDFFontCharsetDetails.h"

#define LOCTEXT_NAMESPACE "FSDFGeneratorEditorModule"

void FSDFGeneratorEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSDFGeneratorEditorModule::OnPostEngineInit);
	
	FSDFGeneratorEditorCommands::Register();
	
	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	const auto AssetCategoryBit = AssetToolsModule.RegisterAdvancedAssetCategory(FName(TEXT("UGUI")),
		LOCTEXT("UGUICategory", "UGUI"));
	
	const TSharedPtr<FAssetTypeAction_SDFFont> SDFFontActionType = MakeShareable(new FAssetTypeAction_SDFFont(AssetCategoryBit));
	ItemDataAssetTypeActions.Add(SDFFontActionType);
	AssetToolsModule.RegisterAssetTypeActions(SDFFontActionType.ToSharedRef());

	//register custom editor
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		
		PropertyModule.RegisterCustomClassLayout(USDFFontCharset::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FSDFFontCharsetDetails::MakeInstance));
		
		PropertyModule.NotifyCustomizationModuleChanged();
	}
}

void FSDFGeneratorEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FSDFGeneratorEditorCommands::Unregister();
	
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

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout(USDFFontCharset::StaticClass()->GetFName());
}

void FSDFGeneratorEditorModule::OnPostEngineInit()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSDFGeneratorEditorModule, SDFGeneratorEditor)
