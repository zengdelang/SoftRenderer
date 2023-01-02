#pragma once

#include "IDetailCustomization.h"
#include "SubComponentsDetailRow/ISubComponentDetailCustomization.h"

class UGUIEDITOR_API FBehaviourComponentDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

	virtual void CustomizeDetails(const TSharedPtr<IDetailLayoutBuilder>& InDetailBuilder) override;
	
protected:
	bool IsParentEnabled() const;
	
protected:
	static FName GetSubComponentCategoryName(IDetailLayoutBuilder& DetailBuilder, FName CategoryName);
	
private:
	TWeakObjectPtr<class UBehaviourComponent> TargetScriptPtr;

	TWeakPtr<IDetailLayoutBuilder> DetailBuilder;

	TArray<TSharedRef<ISubComponentDetailCustomization>> SubComponentDetailCustomizationArray;
	
};
