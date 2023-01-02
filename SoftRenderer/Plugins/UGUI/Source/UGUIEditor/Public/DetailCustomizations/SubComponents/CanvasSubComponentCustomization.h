#pragma once

#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class FCanvasSubComponentCustomization : public ISubComponentDetailCustomization
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;

private:
	TWeakObjectPtr<class UCanvasSubComponent> TargetScriptPtr;
	
};
