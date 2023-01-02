#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class UGUIEDITOR_API FTextComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	TWeakObjectPtr<class UTextComponent> TargetScriptPtr;
};
