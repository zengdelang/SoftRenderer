#include "AssetTypeAction_SDFFont.h"
#include "SDFFontEditor.h"
#include "Core/Widgets/Text/SDFFont.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeAction_SDFFont::FAssetTypeAction_SDFFont(EAssetTypeCategories::Type InAssetCategory)
{
	MyAssetCategory = InAssetCategory;
}

FText FAssetTypeAction_SDFFont::GetName() const
{
	return FText::FromName(TEXT("SDF Font"));
}

UClass* FAssetTypeAction_SDFFont::GetSupportedClass() const
{
	return USDFFont::StaticClass();
}

uint32 FAssetTypeAction_SDFFont::GetCategories()
{
	return MyAssetCategory;
}

void FAssetTypeAction_SDFFont::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

	for (UObject* Object : InObjects)
	{
		const auto SDFFont = Cast<USDFFont>(Object);
		if (SDFFont)
		{
			const TSharedRef<FSDFFontEditor> NewSDFFontEditor(new FSDFFontEditor());
			NewSDFFontEditor->InitSDFFontEditor(Mode, EditWithinLevelEditor, SDFFont);
		}
		else
		{
			FMessageDialog::Open( EAppMsgType::Ok, LOCTEXT("FailedToLoadSDFFont", "SDF font could not be loaded."));
		}
	}
}

#undef LOCTEXT_NAMESPACE
