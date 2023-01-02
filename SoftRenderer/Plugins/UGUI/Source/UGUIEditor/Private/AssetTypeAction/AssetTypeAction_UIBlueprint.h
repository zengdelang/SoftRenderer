#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Toolkits/IToolkitHost.h"
#include "AssetTypeActions/AssetTypeActions_Blueprint.h"

class FAssetTypeAction_UIBlueprint : public FAssetTypeActions_Blueprint
{
public:
	FAssetTypeAction_UIBlueprint(EAssetTypeCategories::Type InAssetCategory);

public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions_UGUI", "AssetTypeActions_UIBlueprint", "UI Blueprint"); }
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;
	
private:
	EAssetTypeCategories::Type MyAssetCategory;

};
