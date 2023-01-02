#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

class FAssetTypeAction_Sprite2D : public FAssetTypeActions_Base
{
public:
	FAssetTypeAction_Sprite2D(EAssetTypeCategories::Type InAssetCategory);

public:
	// IAssetTypeActions Implementation
	virtual FColor GetTypeColor() const override { return FColor::Yellow; }
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	
private:
	EAssetTypeCategories::Type MyAssetCategory;

};
