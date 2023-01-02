#include "AssetTypeAction/AssetTypeAction_TextEmojiSheet.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FAssetTypeAction_TextEmojiSheet::FAssetTypeAction_TextEmojiSheet(EAssetTypeCategories::Type InAssetCategory)
{
	MyAssetCategory = InAssetCategory;
}

FText FAssetTypeAction_TextEmojiSheet::GetName() const
{
	return FText::FromName(TEXT("Text Emoji Sheet"));
}

UClass* FAssetTypeAction_TextEmojiSheet::GetSupportedClass() const
{
	return UTextEmojiSheet::StaticClass();
}

uint32 FAssetTypeAction_TextEmojiSheet::GetCategories()
{
	return MyAssetCategory;
}

#undef LOCTEXT_NAMESPACE
