#pragma once

#include "Core/Layout/RectTransformComponent.h"
#include "SubComponentsDetailRow/IComponentDetailCustomization.h"

class FRectTransformComponentDetails : public IComponentDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	virtual FSlateBrush* GetIconBrush() override;

private:
	void SetAnchorSmart(float Value, int32 Axis, bool bIsMax, bool bSmart, TSharedRef<IPropertyHandle> AnchorMinHandle,
		TSharedRef<IPropertyHandle> AnchorMaxHandle, TSharedRef<IPropertyHandle> AnchoredPositionHandle, TSharedRef<IPropertyHandle> SizeDeltaHandle, bool bCommitted = false);
	
	URectTransformComponent* GetParentComponent() const;

	static FVector GetRectReferenceCorner(const URectTransformComponent* RectTransform, bool bWorldSpace);

	void SetPivotSmart(float Value, int32 Axis, bool bSmart, TSharedRef<IPropertyHandle> PivotHandle, TSharedRef<IPropertyHandle> AnchoredPositionHandle, bool bCommitted = false);

	void SetLocalRotation(float Value, int32 Axis, bool bCommitted = false);
	
	void SetLocalScale(float Value, int32 Axis, bool bCommitted = false);
	
	TSharedRef<SWidget> MakeNumericWidget(const TSharedRef<IPropertyHandle>& PropertyHandle, TAttribute<TOptional<float>> Value,
	const TDelegate<TDelegate<void(float)>::RetValType(float), FDefaultDelegateUserPolicy> OnValueChanged, const TDelegate<TDelegate<void(float, ETextCommit::Type)>::RetValType(float, ETextCommit::Type), FDefaultDelegateUserPolicy> OnValueChangedCommitted, bool bShowLabel = false, FText LabelText = FText(), float SliderExponent = 1) const;
	
private:
	TWeakObjectPtr<class URectTransformComponent> TargetScriptPtr;

	FName NodeName;

	bool bRecordSetAnchorSmart = false;
	bool bRecordSetPivotSmart = false;
	bool bRecordSetLocalRotation = false;
	bool bRecordSetLocalScale = false;
};
