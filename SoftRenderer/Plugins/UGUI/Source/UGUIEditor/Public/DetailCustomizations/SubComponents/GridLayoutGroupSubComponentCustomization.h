#pragma once

#include "IDetailCustomization.h"
#include "UIPrimitiveSubComponentBaseDetails.h"

class FGridLayoutGroupSubComponentCustomization : public FUIPrimitiveSubComponentBaseDetails
{
public:
	static TSharedRef<ISubComponentDetailCustomization> MakeInstance();
	
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder, IDetailCategoryBuilder& CategoryBuilder) override;

private:
	TWeakObjectPtr<class UGridLayoutGroupSubComponent> TargetScriptPtr;

};
