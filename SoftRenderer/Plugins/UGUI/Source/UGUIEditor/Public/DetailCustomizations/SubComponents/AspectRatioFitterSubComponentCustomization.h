#pragma once

#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class FAspectRatioFitterSubComponentCustomization : public ISubComponentDetailCustomization
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;
	
};
