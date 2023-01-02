#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FBackgroundGlitchComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
 
};
