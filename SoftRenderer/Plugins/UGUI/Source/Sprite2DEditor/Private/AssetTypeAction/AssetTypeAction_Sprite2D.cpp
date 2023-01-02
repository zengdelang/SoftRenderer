#include "AssetTypeAction_Sprite2D.h"
#include "Sprite2D.h"
#include "Sprite2DEditor/Sprite2DEditor.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeAction_Sprite2D::FAssetTypeAction_Sprite2D(EAssetTypeCategories::Type InAssetCategory)
{
	MyAssetCategory = InAssetCategory;
}

FText FAssetTypeAction_Sprite2D::GetName() const
{
	return FText::FromName(TEXT("Sprite 2D"));
}

UClass* FAssetTypeAction_Sprite2D::GetSupportedClass() const
{
	return USprite2D::StaticClass();
}

uint32 FAssetTypeAction_Sprite2D::GetCategories()
{
	return MyAssetCategory;
}

void FAssetTypeAction_Sprite2D::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		const auto Sprite = Cast<USprite2D>(Object);
		if (Sprite)
		{
			const TSharedRef<FSprite2DEditor> NewSprite2DEditor(new FSprite2DEditor());
			NewSprite2DEditor->InitSprite2DEditor(Mode, EditWithinLevelEditor, Sprite);
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadSprite", "Sprite could not be loaded."));
		}
	}
}

#undef LOCTEXT_NAMESPACE
