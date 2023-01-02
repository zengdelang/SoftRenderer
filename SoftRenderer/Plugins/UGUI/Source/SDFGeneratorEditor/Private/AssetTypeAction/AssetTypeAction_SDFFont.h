#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FAssetTypeAction_SDFFont : public FAssetTypeActions_Base
{
public:
	FAssetTypeAction_SDFFont(EAssetTypeCategories::Type InAssetCategory);

	virtual FColor GetTypeColor() const override { return FColor(212, 85, 97); }
	virtual FText GetName() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	
private:
	EAssetTypeCategories::Type MyAssetCategory;
	
};
