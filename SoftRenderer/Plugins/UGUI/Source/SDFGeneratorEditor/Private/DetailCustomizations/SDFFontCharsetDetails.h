#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

class IDetailCategoryBuilder;
class IDetailLayoutBuilder;

class FSDFFontCharsetDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	TSharedRef<SWidget> MakeWidgetForOption(TSharedPtr<FString> InOption);
	void OnSelectionChanged(TSharedPtr<FString> NewValue, ESelectInfo::Type);
	FText GetCurrentItemLabel() const;
	
private:
	TWeakObjectPtr<class USDFFontCharset> TargetScriptPtr;
	TArray<TSharedPtr<FString>> FontFaceNames;

	TSharedPtr<FString> CurrentItem;
	TSharedPtr<IPropertyHandle> FaceIndexProperty;
	
};
