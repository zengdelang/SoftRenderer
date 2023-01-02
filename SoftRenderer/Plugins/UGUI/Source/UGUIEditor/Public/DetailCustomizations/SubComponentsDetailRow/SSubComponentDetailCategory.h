#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SSubComponentDetailCategory : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSubComponentDetailCategory)
		{}

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedPtr<IPropertyHandle> InEnabledHandle, const TSharedRef< class IPropertyHandle >& InPropertyHandle,
		const TSharedRef< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, const TAttribute<bool>& InIsParentEnabled);

	void SetIndex(int32 InIndex);
	
protected:
	virtual FVector2D ComputeDesiredSize(float InValue) const override;

private:
	TSharedPtr<IPropertyHandle> EnabledHandle;

	TSharedPtr<class IPropertyHandle> PropertyHandle;
	TSharedPtr<class IPropertyHandle> SubComponentsPropertyHandle;
	
	TWeakObjectPtr<class UObject> OwnerObject;
	int32 Index = 0;

	/** Whether or not our parent is enabled */
	TAttribute<bool> IsParentEnabled;

public:
	bool bHideEnableCheckBox;
	FSlateBrush* ComponentIcon;
	
};
