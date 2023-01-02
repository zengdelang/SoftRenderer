#pragma once

#include "CoreMinimal.h"
#include "SUIAnchorPreset.h"

class URectTransformComponent;

class SUIAnchorPresetPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUIAnchorPresetPanel)
	{}

	SLATE_ATTRIBUTE(FVector2D,AnchorMax)

	SLATE_ATTRIBUTE(FVector2D,AnchorMin)

	SLATE_ATTRIBUTE(FVector2D,Pivot)

	SLATE_ATTRIBUTE(TSharedPtr<IPropertyHandle>, SizeDeltaHandle)

	SLATE_END_ARGS()
	
	SUIAnchorPresetPanel();

	void Construct(const FArguments& InArgs, TSharedPtr<IPropertyHandle> InAnchorMax, TSharedPtr<IPropertyHandle> InAnchorMin, TSharedPtr<IPropertyHandle> InPivot,
		TSharedPtr<IPropertyHandle> InAnchoredPositionHandle, TWeakObjectPtr<class URectTransformComponent> InTargetScriptPtr);

protected:
	FReply OnClick(const FGeometry&, const FPointerEvent& ,FVector2D InAnchorMin,FVector2D InAnchorMax,FVector2D InPivot, FAnchorType InAnchorType);
	FReply OnEdgeClick(const FGeometry&, const FPointerEvent&, FVector2D InAnchorMin, FVector2D InAnchorMax, FAnchorType InAnchorType);

	void SetAnchorSmart(float Value, int32 Axis, bool bIsMax, bool bSmart) const;
	void SetPivotSmart(float Value, int32 Axis, bool bSmart) const;
	
	URectTransformComponent* GetParentComponent() const;
	static FVector GetRectReferenceCorner(const URectTransformComponent* RectTransform, bool bWorldSpace);
	
	static EVerticalAnchorType GetVerticalAnchorType( FVector2D InAnchorMin,  FVector2D InAnchorMax);
	static EHorizontalAnchorType GetHorizontalAnchorType( FVector2D AnchorMin,  FVector2D InAnchorMax);

	static FVector2D GetPivot(FAnchorType AnchorType);

protected:
	TSharedRef<SWidget> MakeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D Pivot, EHorizontalAnchorType InHorizontalAnchorType, EVerticalAnchorType InVerticalAnchorType,
		TAttribute<bool> InShowPivotLambda, TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda, TAttribute<EVerticalAnchorType> SelectVerticalLambda, TAttribute<bool> bShowSelectedBound);
	TSharedRef<SWidget> MakeHorizontalEdgeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax, EHorizontalAnchorType InHorizontalAnchorType, EVerticalAnchorType InVerticalAnchorType,
		TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda, TAttribute<EVerticalAnchorType> SelectVerticalLambda, FText TextLabel, TAttribute<bool> bShowSelectedBound);
	TSharedRef<SWidget> MakeVerticalEdgeAnchorItem(FVector2D InAnchorMin, FVector2D InAnchorMax, EHorizontalAnchorType InHorizontalAnchorType, EVerticalAnchorType InVerticalAnchorType,
		TAttribute<bool> InShowPositionLambda, TAttribute<EHorizontalAnchorType> SelectHorizontalLambda, TAttribute<EVerticalAnchorType> SelectVerticalLambda, FText TextLabel, TAttribute<bool> bShowSelectedBound);
	
protected:
	TSharedPtr<IPropertyHandle> AnchorMaxHandle;
	TSharedPtr<IPropertyHandle> AnchorMinHandle;
	TSharedPtr<IPropertyHandle> PivotHandle;
	TSharedPtr<IPropertyHandle> AnchoredPositionHandle;
	TSharedPtr<IPropertyHandle> SizeDeltaHandle;

	TWeakObjectPtr<class URectTransformComponent> TargetScriptPtr;

	TAttribute<FVector2D> AnchorMin;
	TAttribute<FVector2D> AnchorMax;
	TAttribute<FVector2D> Pivot;

	EHorizontalAnchorType SelectHorizontalAnchorType;
	EVerticalAnchorType SelectVerticalAnchorType;

	FVector2D DefaultAnchorMin;
	FVector2D DefaultanchorMax;
	FVector2D DefaultanchoredPosition;
	FVector2D DefaultSizeDelta;
	
};
