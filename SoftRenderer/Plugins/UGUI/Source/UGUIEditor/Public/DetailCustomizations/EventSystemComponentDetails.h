#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FEventSystemComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

};
