#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Core/Widgets/Text/TextEmojiSheet.h"

class FAssetTypeAction_TextEmojiSheet : public FAssetTypeActions_Base
{
public:
	FAssetTypeAction_TextEmojiSheet(EAssetTypeCategories::Type InAssetCategory);

	virtual FColor GetTypeColor() const override { return FColor(97, 85, 212); }
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

private:
	EAssetTypeCategories::Type MyAssetCategory;
	
};
