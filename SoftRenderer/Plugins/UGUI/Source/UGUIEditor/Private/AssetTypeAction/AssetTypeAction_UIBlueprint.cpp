#include "AssetTypeAction_UIBlueprint.h"
#include "BlueprintEditorModule.h"
#include "Core/UIBlueprint.h"
#include "EditorStyle.h"
#include "UIBlueprintEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeAction_UIBlueprint::FAssetTypeAction_UIBlueprint(EAssetTypeCategories::Type InAssetCategory)
	: MyAssetCategory(InAssetCategory)
{

}

uint32 FAssetTypeAction_UIBlueprint::GetCategories()
{
	return MyAssetCategory;
}

void FAssetTypeAction_UIBlueprint::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		const auto Blueprint = Cast<UUIBlueprint>(Object);
		if (Blueprint && Blueprint->SkeletonGeneratedClass && Blueprint->GeneratedClass )
		{
			const TSharedRef<FUIBlueprintEditor> NewBlueprintEditor(new FUIBlueprintEditor());

			TArray<UBlueprint*> Blueprints;
			Blueprints.Add(Blueprint);
			NewBlueprintEditor->InitUIBlueprintEditor(Mode, EditWithinLevelEditor, Blueprints, false);
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadUIBlueprint", "UI Blueprint could not be loaded because it derives from an invalid class.\nCheck to make sure the parent class for this blueprint hasn't been removed!"));
		}
	}
}

UClass* FAssetTypeAction_UIBlueprint::GetSupportedClass() const
{ 
	return UUIBlueprint::StaticClass();
}

#undef LOCTEXT_NAMESPACE
