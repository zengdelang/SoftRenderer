#pragma once

#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class FContentSizeFitterSubComponentCustomization : public ISubComponentDetailCustomization
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;
	
};
