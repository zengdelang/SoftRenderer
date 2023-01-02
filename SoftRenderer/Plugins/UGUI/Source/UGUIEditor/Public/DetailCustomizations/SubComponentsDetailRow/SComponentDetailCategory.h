#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SComponentDetailCategory : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SComponentDetailCategory)
		{}

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, class UObject* InOwnerObject);
	
protected:
	virtual FVector2D ComputeDesiredSize(float InValue) const override;

private:
	TWeakObjectPtr<class UObject> OwnerObject;
	bool bHideEnableCheckBox = true;
	
public:
	FSlateBrush* ComponentIcon;
	
};
