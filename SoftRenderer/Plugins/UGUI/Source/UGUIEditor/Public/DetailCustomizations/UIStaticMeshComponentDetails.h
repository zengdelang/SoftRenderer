#pragma once

#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "UIPrimitiveBaseDetails.h"

class FUIStaticMeshComponentDetails : public FUIPrimitiveBaseDetails
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<class UUIStaticMeshComponent> TargetScriptPtr;
	
};
