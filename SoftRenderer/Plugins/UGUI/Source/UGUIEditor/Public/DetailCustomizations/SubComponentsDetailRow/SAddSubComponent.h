#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class UBehaviourComponent;

class SAddSubComponent : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAddSubComponent)
		{}

	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, const TSharedRef< class IPropertyHandle >& InRefreshPropertyHandle, 
		const TSharedRef< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, const TAttribute<bool>& InIsParentEnabled);

protected:
	virtual FVector2D ComputeDesiredSize(float InValue) const override;

private:
	TSharedRef<SWidget> GenerateClassPicker();

	void OnClassPicked(UClass* InClass) const;

	static bool IsDisallowMultipleComponent(UClass* InClass, UClass*& RootDisallowClass);

	UObject* CreateObjectByClass(const UClass* InClass, UObject* Object) const;

	void GetDisallowedSubClasses(UClass* InClass, TSet<UClass*>& DisallowedSubClasses) const;
	void GetRequireSubClasses(UClass* InClass, TArray<UClass*>& RequireSubClasses) const;

	static void StripInvalidRequireSubClasses(TArray<UClass*>& RequireSubClasses, UBehaviourComponent* Component);
	
protected:
	TSharedPtr<class IPropertyHandle> RefreshPropertyHandle;
	TSharedPtr<class IPropertyHandle> SubComponentsPropertyHandle;
	
	TWeakObjectPtr<class UObject> OwnerObject;

	/** Whether or not our parent is enabled */
	TAttribute<bool> IsParentEnabled;
	
	TSharedPtr<class SComboButton> ComboButton;

	bool bRefreshArrayHandle = true;
	
};
