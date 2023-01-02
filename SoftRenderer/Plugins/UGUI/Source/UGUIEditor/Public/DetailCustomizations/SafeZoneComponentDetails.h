#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FSafeZoneComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<class USafeZoneComponent> TargetScriptPtr;

};
