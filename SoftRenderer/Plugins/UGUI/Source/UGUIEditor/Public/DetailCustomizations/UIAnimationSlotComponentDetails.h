#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FUIAnimationSlotComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

};
