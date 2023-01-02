#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FInputFieldComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	void MakeColorMultiplierRow(IDetailGroup& Category, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const;
	
private:
	TWeakObjectPtr<class UInputFieldComponent> TargetScriptPtr;

};
