#pragma once

#include "UIPrimitiveSubComponentBaseDetails.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class FUICascadeSubComponentCustomization : public FUIPrimitiveSubComponentBaseDetails
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;
	
};
