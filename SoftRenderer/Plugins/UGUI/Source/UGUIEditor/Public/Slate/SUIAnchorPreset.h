#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

enum class EVerticalAnchorType : uint8
{
	Top,
	Middle,
	Bottom,
	Stretch,
	Custom,
};

enum class EHorizontalAnchorType : uint8
{
	Left,
	Center,
	Right,
	Stretch,
	Custom,
};

struct FAnchorType
{
	EHorizontalAnchorType HorizontalAnchorType;
	EVerticalAnchorType VerticalAnchorType;

	FAnchorType(EHorizontalAnchorType InHorizontalAnchorType, EVerticalAnchorType InVerticalAnchorType)
	    :HorizontalAnchorType(InHorizontalAnchorType), VerticalAnchorType(InVerticalAnchorType) {}
};

class SUIAnchorPreset : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SUIAnchorPreset)
		: _bShowSelectedPosBound(true)
		, _SelectedBoundColor(FLinearColor::White)
		, _CenterBoundColor(FLinearColor(102 / 255.f, 102 / 255.f, 102 / 255.f))
		, _bShowCenterPosBound(true)
		, _PosBoundColor(FLinearColor(153 / 255.f, 153 / 255.f, 153 / 255.f))
		, _bShowPivot(false)
		, _PivotPosition(FVector2D(0.0f, 0.0f))
		, _PivotColor(FLinearColor(0, 116 / 255.f, 232 / 255.f))
		, _bShowAnchor(true)
		, _AnchorMin(FVector2D(0.5f, 0.5f))
		, _AnchorMax(FVector2D(0.5f, 0.5f))
	    , _CurrentHorizontalAnchorType(EHorizontalAnchorType::Custom)
	    ,_CurrentVerticalAnchorType(EVerticalAnchorType::Custom)
	{
	
	}

	SLATE_ATTRIBUTE(bool, bShowSelectedPosBound)
	
	SLATE_ATTRIBUTE(FLinearColor, SelectedBoundColor)
	
	SLATE_ATTRIBUTE(FLinearColor, CenterBoundColor)

	SLATE_ATTRIBUTE(bool, bShowCenterPosBound)

	SLATE_ATTRIBUTE(FLinearColor, PosBoundColor)
	
	SLATE_ATTRIBUTE(bool, bShowPivot)
	
	SLATE_ATTRIBUTE(FVector2D, PivotPosition)

	SLATE_ATTRIBUTE(FLinearColor, PivotColor)

	SLATE_ATTRIBUTE(bool, bShowAnchor)

	SLATE_ATTRIBUTE(FVector2D, AnchorMin)
	
	SLATE_ATTRIBUTE(FVector2D, AnchorMax)

    SLATE_ATTRIBUTE(EHorizontalAnchorType, CurrentHorizontalAnchorType)

	SLATE_ATTRIBUTE(EVerticalAnchorType, CurrentVerticalAnchorType)

	SLATE_END_ARGS()

	SUIAnchorPreset();

	void Construct(const FArguments& InArgs);

public:
	// SWidget overrides
	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override;

protected:
	// Begin SWidget overrides.
	virtual FVector2D ComputeDesiredSize(float) const override;
	// End SWidget overrides.

protected:
	static void MakeBoxLines(FSlateWindowElementList& OutDrawElements,const FGeometry& AllottedGeometry, const int32& LayerId, FVector2D Center, FVector2D Size, TAttribute<FLinearColor> Color);

	static void MakeVerticalRedLine(const EVerticalAnchorType& VerticalAnchorType);
	static void MakeHorizontalRedLine(const EHorizontalAnchorType& HorizontalAnchorType);

	static void MakeVerticalBlueLine(const EVerticalAnchorType& VerticalAnchorType);
    static void MakeHorizontalBlueLine(const EHorizontalAnchorType& HorizontalAnchorType);

	EVerticalAnchorType GetVerticalAnchorType() const;
	EHorizontalAnchorType GetHorizontalAnchorType() const;

	static FVector2D GetLocation(FVector2D Position);

	static FVector2D GetOffset(EVerticalAnchorType VerticalAnchorType, EHorizontalAnchorType HorizontalAnchorType, float Delta);

	static bool CheckSelected(EVerticalAnchorType OriginalVerticalAnchorType, EHorizontalAnchorType OriginalHorizontalAnchorType, EVerticalAnchorType VerticalAnchorType, EHorizontalAnchorType HorizontalAnchorType);

protected:
	TAttribute<bool> bShowSelectedPosBound;
	TAttribute<FLinearColor> SelectedBoundColor;
	TAttribute<bool> bShowSelectedBound;
	
	TAttribute<FLinearColor> CenterBoundColor;

	TAttribute<bool> bShowCenterPosBound;
	TAttribute<FLinearColor> PosBoundColor;
	
	TAttribute<bool> bShowPivot;
	TAttribute<FVector2D> PivotPosition;
	TAttribute<FLinearColor> PivotColor;

	TAttribute<bool> bShowAnchor;
	TAttribute<FVector2D> AnchorMin;
	TAttribute<FVector2D> AnchorMax;

	TAttribute<EHorizontalAnchorType> CurrentHorizontalAnchorType;
	TAttribute<EVerticalAnchorType> CurrentVerticalAnchorType;
};
