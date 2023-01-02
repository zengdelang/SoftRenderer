#pragma once

#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FScrollbarComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

protected:
	void MakeColorMultiplierRow(IDetailGroup& ColorTintAndSpriteGroup, const TSharedRef<IPropertyHandle> ColorSpriteBlockProperty) const;
	
private:
	TWeakObjectPtr<class UScrollbarComponent> TargetScriptPtr;

};
