#pragma once

#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class FLayoutElementSubComponentCustomization : public ISubComponentDetailCustomization
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;

private:
	void CreateFloatPropertyWidget(FName PropertyName, IDetailCategoryBuilder& CategoryBuilder, FText FilterString);
	
private:
	TWeakObjectPtr<class ULayoutElementSubComponent> TargetScriptPtr;
	
};
