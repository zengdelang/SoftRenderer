#pragma once

#include "IDetailCustomization.h"
#include "UIPrimitiveBaseDetails.h"

class FUICascadeComponentDetails : public FUIPrimitiveBaseDetails
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	
private:
	TWeakObjectPtr<class UUICascadeComponent> TargetScriptPtr;

};
