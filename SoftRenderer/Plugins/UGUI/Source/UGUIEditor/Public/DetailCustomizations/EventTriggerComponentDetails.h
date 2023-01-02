#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FEventTriggerComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

};
