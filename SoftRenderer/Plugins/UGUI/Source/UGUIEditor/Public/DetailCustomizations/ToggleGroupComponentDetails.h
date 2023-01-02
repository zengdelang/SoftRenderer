#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FToggleGroupComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<class UImageComponent> TargetScriptPtr;

};
