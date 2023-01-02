#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SSubComponentOverlay : public SOverlay
{
public:
	SLATE_BEGIN_ARGS(SSubComponentOverlay)
		{}

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedPtr< class IPropertyHandle > InPropertyHandle, 
		const TSharedPtr< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, int32 InIndex, const TAttribute<bool>& InIsParentEnabled);

public:
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	void GetRequireSubClasses(UClass* InClass, TArray<UClass*>& RequireSubClasses) const;
	
	void OnRemoveComponentClicked() const;
	bool CanRemoveComponent() const;

	void OnMoveUpClicked() const;
	bool CanMoveUp() const;

	void OnMoveDownClicked() const;
	bool CanMoveDown() const;
	
private:
	TSharedPtr<class IPropertyHandle> PropertyHandle;
	TSharedPtr<class IPropertyHandle> SubComponentsPropertyHandle;
	TWeakObjectPtr<class UObject> OwnerObject;
	
	int32 Index = 0;

	/** Whether or not our parent is enabled */
	TAttribute<bool> IsParentEnabled;
	
};
