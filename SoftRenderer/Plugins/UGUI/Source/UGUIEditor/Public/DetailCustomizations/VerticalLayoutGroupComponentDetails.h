#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FVerticalLayoutGroupComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<class UVerticalLayoutGroupComponent> TargetScriptPtr;

};
