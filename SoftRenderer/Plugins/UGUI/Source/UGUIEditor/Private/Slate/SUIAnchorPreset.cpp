#include "Slate/SUIAnchorPreset.h"
#include "UGUIEditorStyle.h"

TArray<FVector2D> BoundPoints;
TArray<FVector2D> CachedPoints;

TArray<FVector2D> VerticalRedLine;
TArray<FVector2D> HorizontalRedLine;

TArray<FVector2D> VerticalBlueLine;
TArray<FVector2D> HorizontalBlueLine;

FVector2D SlatePivot;

SUIAnchorPreset::SUIAnchorPreset()
{

}

void SUIAnchorPreset::Construct( const FArguments& InArgs )
{
	bShowSelectedPosBound = InArgs._bShowSelectedPosBound;
	SelectedBoundColor = InArgs._SelectedBoundColor;
	
	CenterBoundColor = InArgs._CenterBoundColor;

	bShowCenterPosBound = InArgs._bShowCenterPosBound;
	PosBoundColor = InArgs._PosBoundColor;
	
	bShowPivot = InArgs._bShowPivot;
	PivotPosition = InArgs._PivotPosition;
	PivotColor = InArgs._PivotColor;

	bShowAnchor = InArgs._bShowAnchor;
	AnchorMin = InArgs._AnchorMin;
	AnchorMax = InArgs._AnchorMax;

	CurrentHorizontalAnchorType = InArgs._CurrentHorizontalAnchorType;
	CurrentVerticalAnchorType = InArgs._CurrentVerticalAnchorType;
}

int32 SUIAnchorPreset::OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const
{
	const FVector2D Center = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5, 0.5));
	const FVector2D LocalSize = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(1, 1)) - Center;

	const EVerticalAnchorType VerticalAnchorType = GetVerticalAnchorType();
	const EHorizontalAnchorType HorizontalAnchorType = GetHorizontalAnchorType();
		
	SlatePivot = Center;

	const bool bRealShowSelectedPosBound = bShowSelectedPosBound.Get() && CheckSelected(VerticalAnchorType, HorizontalAnchorType, CurrentVerticalAnchorType.Get(), CurrentHorizontalAnchorType.Get());
	
	if (bRealShowSelectedPosBound)
	{
		MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId,Center, LocalSize,SelectedBoundColor);
	}
	
	const FVector2D CenterLocalSize = LocalSize * (42 - 8) / 42.f;
	const float Delta = (CenterLocalSize.X / 2.0f) / 42.0f;

	MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId,Center, CenterLocalSize,CenterBoundColor);

	if (!(VerticalAnchorType == EVerticalAnchorType::Custom && HorizontalAnchorType == EHorizontalAnchorType::Custom))
	{
		if (VerticalAnchorType != EVerticalAnchorType::Custom && VerticalAnchorType != EVerticalAnchorType::Stretch)
		{
			MakeVerticalRedLine(VerticalAnchorType);
 		    FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), VerticalRedLine, ESlateDrawEffect::None, FLinearColor(185 / 255.0f, 19 / 255.0f, 19 / 255.0f), true, 1);
		}

		if (HorizontalAnchorType != EHorizontalAnchorType::Custom && HorizontalAnchorType!= EHorizontalAnchorType::Stretch)
		{
			MakeHorizontalRedLine(HorizontalAnchorType);
			FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), HorizontalRedLine, ESlateDrawEffect::None, FLinearColor(185 / 255.0f, 19 / 255.0f, 19 / 255.0f), true, 1);
		}

		FVector2D Anchor = AnchorMin.Get();
		FVector2D SlateAnchor = FVector2D(Anchor.X, 1 - Anchor.Y);

		if (VerticalAnchorType != EVerticalAnchorType::Custom && HorizontalAnchorType != EHorizontalAnchorType::Custom)
		{
			if (VerticalAnchorType != EVerticalAnchorType::Stretch && HorizontalAnchorType != EHorizontalAnchorType::Stretch)
			{
				FVector2D AnchorPosition = GetLocation(CachedPoints[0] + (CachedPoints[2] - CachedPoints[0]) * SlateAnchor);
				FSlateBrush ImageBrush;
				const FGeometry AnchorGeometry = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.1f, 0.1f), FVector2D(AnchorPosition)));
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AnchorGeometry.ToPaintGeometry(), &ImageBrush, ESlateDrawEffect::None, FLinearColor(0.6f, 0.32f, 0.0, 1.0f));
			}
			else if(VerticalAnchorType == EVerticalAnchorType::Stretch && HorizontalAnchorType == EHorizontalAnchorType::Stretch)
			{
				TArray<FVector2D> AnchorPositions;
				for(int32 Index = 0 ; Index < 4 ; Index++)
				{
					AnchorPositions.Emplace(CachedPoints[Index]);
				}

				FSlateBrush ImageBrush;
				for(int32 Index = 0 ; Index < 4 ; Index++)
				{
					const FGeometry AnchorGeometry = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.05f, 0.05f), GetLocation(AnchorPositions[Index])));
					FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AnchorGeometry.ToPaintGeometry(), &ImageBrush, ESlateDrawEffect::None, FLinearColor(0.6f, 0.32f, 0.0, 1.0f));
				}
			}
			else
			{
				TArray<FVector2D> AnchorPositions;

			    if (VerticalAnchorType == EVerticalAnchorType::Stretch)
			    {
			        if(HorizontalAnchorType== EHorizontalAnchorType::Left)
			        {
						AnchorPositions.Emplace(CachedPoints[0]);
						AnchorPositions.Emplace(CachedPoints[3]);
			        }
					else if(HorizontalAnchorType == EHorizontalAnchorType::Center)
					{
						AnchorPositions.Emplace((CachedPoints[0] + CachedPoints[1]) / 2.0f);
						AnchorPositions.Emplace((CachedPoints[3] + CachedPoints[2]) / 2.0f);
					}
					else
					{
						AnchorPositions.Emplace(CachedPoints[1]);
						AnchorPositions.Emplace(CachedPoints[2]);
					}
			    }

				if (HorizontalAnchorType == EHorizontalAnchorType::Stretch)
				{
				    if(VerticalAnchorType == EVerticalAnchorType::Top)
				    {
						AnchorPositions.Emplace(CachedPoints[0]);
						AnchorPositions.Emplace(CachedPoints[1]);
				    }
					else if(VerticalAnchorType == EVerticalAnchorType::Middle)
					{
						AnchorPositions.Emplace((CachedPoints[0] + CachedPoints[3]) / 2.0f);
						AnchorPositions.Emplace((CachedPoints[1] + CachedPoints[2]) / 2.0f);
					}
					else
					{
						AnchorPositions.Emplace(CachedPoints[3]);
						AnchorPositions.Emplace(CachedPoints[2]);
					}
				}

				if (AnchorPositions.Num() > 0)
				{
					FSlateBrush ImageBrush;
					for (int32 Index = 0; Index < AnchorPositions.Num(); Index++)
					{
						const FGeometry AnchorGeometry = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.05f, 0.05f), GetLocation(AnchorPositions[Index])));
						FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AnchorGeometry.ToPaintGeometry(), &ImageBrush, ESlateDrawEffect::None, FLinearColor(0.6f, 0.32f, 0.0, 1.0f));
					}

					AnchorPositions.Reset();
				}
			}
		}
	}

	if (bShowCenterPosBound.Get())
	{
		MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId, Center, CenterLocalSize * 0.5f, PosBoundColor);
	}
	else
	{
		FVector2D Offset = GetOffset(VerticalAnchorType, HorizontalAnchorType, Delta);
		FVector2D PosCenter = AllottedGeometry.GetLocalPositionAtCoordinates(FVector2D(0.5f, 0.5f) + Offset);
		
		if (VerticalAnchorType == EVerticalAnchorType::Stretch && HorizontalAnchorType == EHorizontalAnchorType::Stretch)
		{
			MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId, Center, CenterLocalSize, PosBoundColor);
		}
		else if(VerticalAnchorType == EVerticalAnchorType::Stretch)
		{
			MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId, PosCenter, FVector2D(CenterLocalSize.X * 0.5f, CenterLocalSize.Y), PosBoundColor);
		}
		else if(HorizontalAnchorType == EHorizontalAnchorType::Stretch)
		{
			MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId, PosCenter, FVector2D(CenterLocalSize.X, CenterLocalSize.Y * 0.5f), PosBoundColor);
		}
		else
		{
			MakeBoxLines(OutDrawElements, AllottedGeometry, LayerId, PosCenter, CenterLocalSize * 0.5f, PosBoundColor);
		}
	}

	VerticalBlueLine.Reset();
	MakeVerticalBlueLine(VerticalAnchorType);

	HorizontalBlueLine.Reset();
	MakeHorizontalBlueLine(HorizontalAnchorType);
	
	if (VerticalBlueLine.Num() > 0)
	{
		auto ImageBrush = FUGUIEditorStyle::Get().GetBrush("UGUI.Triangle");


		auto TopF = FVector2D(VerticalBlueLine[0].X, VerticalBlueLine[0].Y + 2.1f);
		auto BottomF = FVector2D(VerticalBlueLine[1].X, VerticalBlueLine[1].Y - 2.1f);

		const FGeometry Top = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.1f,0.1f), GetLocation(TopF)));
		const FGeometry Bottom = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.1f, -0.1f), GetLocation(BottomF)));

		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), VerticalBlueLine, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f), true, 1);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Top.ToPaintGeometry(), ImageBrush, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Bottom.ToPaintGeometry(), ImageBrush, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f));
	}

	if (HorizontalBlueLine.Num() > 0)
	{
		auto ImageBrush = FUGUIEditorStyle::Get().GetBrush("UGUI.TriangleLeft");

		const FGeometry Left = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.1f, 0.1f), GetLocation(FVector2D(HorizontalBlueLine[0].X + 2.1f , HorizontalBlueLine[0].Y ))));
		const FGeometry Right = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(-0.1f, 0.1f), GetLocation(FVector2D(HorizontalBlueLine[1].X - 2.1f, HorizontalBlueLine[1].Y))));

		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), HorizontalBlueLine, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f), true, 1);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Left.ToPaintGeometry(), ImageBrush, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Right.ToPaintGeometry(), ImageBrush, ESlateDrawEffect::None, FLinearColor(0,116 / 255.0f,232 / 255.0f));
	}

	if (bShowPivot.Get())
	{
		FVector2D Pivot = PivotPosition.Get();
		FVector2D LocalPivot = FVector2D(Pivot.X, 1 - Pivot.Y);
		FVector2D AnchorPosition = GetLocation(CachedPoints[0] + (CachedPoints[2] - CachedPoints[0]) * LocalPivot);

		FSlateBrush ImageBrush;
		const FGeometry AnchorGeometry = AllottedGeometry.MakeChild(FSlateRenderTransform(FScale2D(0.1f, 0.1f), FVector2D(AnchorPosition)));
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AnchorGeometry.ToPaintGeometry(), &ImageBrush, ESlateDrawEffect::None, PivotColor.Get());
	}
	
	return LayerId;
}

FVector2D SUIAnchorPreset::ComputeDesiredSize(float) const
{
	return FVector2D(0,0);
}

void SUIAnchorPreset::MakeBoxLines(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, const int32& LayerId, FVector2D Center, FVector2D Size, TAttribute<FLinearColor> Color)
{
	CachedPoints.Reset();

	BoundPoints.Reset();
	BoundPoints.Emplace(Center - Size); 
	BoundPoints.Emplace(Center + FVector2D(Size.X, -Size.Y));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BoundPoints, ESlateDrawEffect::None, Color.Get(), true, 1);
	CachedPoints.Emplace(BoundPoints[0]);
	
	BoundPoints.Reset();
	BoundPoints.Emplace(Center + FVector2D(Size.X, -Size.Y));
	BoundPoints.Emplace(Center + Size);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BoundPoints, ESlateDrawEffect::None, Color.Get(), true, 1);
	CachedPoints.Emplace(BoundPoints[0]);
	
	BoundPoints.Reset();
	BoundPoints.Emplace(Center + Size);
	BoundPoints.Emplace(Center + FVector2D(-Size.X, Size.Y));
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BoundPoints, ESlateDrawEffect::None, Color.Get(), true, 1);
	CachedPoints.Emplace(BoundPoints[0]);
	
	BoundPoints.Reset();
	BoundPoints.Emplace(Center + FVector2D(-Size.X, Size.Y));
	BoundPoints.Emplace(Center - Size);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), BoundPoints, ESlateDrawEffect::None, Color.Get(), true, 1);
	CachedPoints.Emplace(BoundPoints[0]);
}

void SUIAnchorPreset::MakeVerticalRedLine(const EVerticalAnchorType& VerticalAnchorType)
{
	if (VerticalAnchorType == EVerticalAnchorType::Stretch)
		return;

	if(VerticalAnchorType == EVerticalAnchorType::Top)
	{
		VerticalRedLine.Reset();
		VerticalRedLine.Emplace(CachedPoints[0]);
		VerticalRedLine.Emplace(CachedPoints[1]);
	}
	else if(VerticalAnchorType == EVerticalAnchorType::Middle)
	{
		VerticalRedLine.Reset();
		VerticalRedLine.Emplace(FVector2D(CachedPoints[3].X, (CachedPoints[0].Y + CachedPoints[3].Y) / 2.0f));
		VerticalRedLine.Emplace(FVector2D(CachedPoints[2].X, (CachedPoints[0].Y + CachedPoints[2].Y) / 2.0f));
	}
	else if(VerticalAnchorType == EVerticalAnchorType::Bottom)
	{
		VerticalRedLine.Reset();
		VerticalRedLine.Emplace(CachedPoints[3].X, CachedPoints[3].Y);
		VerticalRedLine.Emplace(CachedPoints[2].X, CachedPoints[2].Y);
	}
}

void SUIAnchorPreset::MakeHorizontalRedLine(const EHorizontalAnchorType& HorizontalAnchorType)
{
	if (HorizontalAnchorType == EHorizontalAnchorType::Stretch)
		return;

	if (HorizontalAnchorType == EHorizontalAnchorType::Left)
	{
		HorizontalRedLine.Reset();
		HorizontalRedLine.Emplace(CachedPoints[0]);
		HorizontalRedLine.Emplace(CachedPoints[3]);
	}
	else if (HorizontalAnchorType == EHorizontalAnchorType::Center)
	{
		HorizontalRedLine.Reset();
		HorizontalRedLine.Emplace(FVector2D((CachedPoints[0].X + CachedPoints[1].X) / 2.0f, CachedPoints[1].Y));
		HorizontalRedLine.Emplace(FVector2D((CachedPoints[0].X+ CachedPoints[2].X) / 2.0f, CachedPoints[2].Y));
	}
	else if (HorizontalAnchorType == EHorizontalAnchorType::Right)
	{
		HorizontalRedLine.Reset();
		HorizontalRedLine.Emplace(CachedPoints[1].X, CachedPoints[1].Y);
		HorizontalRedLine.Emplace(CachedPoints[2].X, CachedPoints[2].Y);
	}
}

void SUIAnchorPreset::MakeVerticalBlueLine(const EVerticalAnchorType& VerticalAnchorType)
{
	if (VerticalAnchorType != EVerticalAnchorType::Stretch)
		return;

	VerticalBlueLine.Reset();
	VerticalBlueLine.Emplace((CachedPoints[0] + CachedPoints[1]) / 2.0f);
	VerticalBlueLine.Emplace((CachedPoints[2] + CachedPoints[3]) / 2.0f);
}

void SUIAnchorPreset::MakeHorizontalBlueLine(const EHorizontalAnchorType& HorizontalAnchorType)
{
	if (HorizontalAnchorType != EHorizontalAnchorType::Stretch)
		return;

	HorizontalBlueLine.Reset();
	HorizontalBlueLine.Emplace((CachedPoints[0] + CachedPoints[3]) / 2.0f);
	HorizontalBlueLine.Emplace((CachedPoints[1] + CachedPoints[2]) / 2.0f);
}

EVerticalAnchorType SUIAnchorPreset::GetVerticalAnchorType() const
{
	const FVector2D AnchorMinValue = AnchorMin.Get();
	const FVector2D AnchorMaxValue = AnchorMax.Get();

	if (AnchorMinValue.Y == AnchorMaxValue.Y)
	{
		if (AnchorMinValue.Y == 0)
		{
			return EVerticalAnchorType::Bottom;
		}

		if (AnchorMinValue.Y == 0.5)
		{
			return EVerticalAnchorType::Middle;
		}

		if (AnchorMinValue.Y == 1)
		{
			return EVerticalAnchorType::Top;
		}
	}
	else
	{
		if (AnchorMinValue.Y == 0 && AnchorMaxValue.Y == 1)
		{
			return EVerticalAnchorType::Stretch;
		}
	}

	return EVerticalAnchorType::Custom;
}

EHorizontalAnchorType SUIAnchorPreset::GetHorizontalAnchorType() const
{
	const FVector2D AnchorMinValue = AnchorMin.Get();
	const FVector2D AnchorMaxValue = AnchorMax.Get();

	if (AnchorMinValue.X == AnchorMaxValue.X)
	{
		if (AnchorMinValue.X == 0)
		{
			return EHorizontalAnchorType::Left;
		}

		if (AnchorMinValue.X == 0.5)
		{
			return EHorizontalAnchorType::Center;
		}

		if (AnchorMinValue.X == 1)
		{
			return EHorizontalAnchorType::Right;
		}
	}
	else
	{
		if (AnchorMinValue.X == 0 && AnchorMaxValue.X == 1)
		{
			return EHorizontalAnchorType::Stretch;
		}
	}

	return EHorizontalAnchorType::Custom;
}

FVector2D SUIAnchorPreset::GetLocation(FVector2D Position)
{
	return  Position - SlatePivot;
}

FVector2D SUIAnchorPreset::GetOffset(EVerticalAnchorType VerticalAnchorType, EHorizontalAnchorType HorizontalAnchorType, float Delta)
{
	float XOffset = 0.0f;
	float YOffset = 0.0f;

	switch (VerticalAnchorType)
	{
	case EVerticalAnchorType::Top:
		YOffset -= Delta;
		break;
	case EVerticalAnchorType::Bottom:
		YOffset += Delta;
	default:break;
	}

	switch (HorizontalAnchorType)
	{
	case EHorizontalAnchorType::Left:
		XOffset -= Delta;
		break;
	case EHorizontalAnchorType::Right:
		XOffset += Delta;
	default:break;
	}

	return FVector2D(XOffset, YOffset);
}

bool SUIAnchorPreset::CheckSelected(EVerticalAnchorType OriginalVerticalAnchorType, EHorizontalAnchorType OriginalHorizontalAnchorType, EVerticalAnchorType VerticalAnchorType, EHorizontalAnchorType HorizontalAnchorType)
{
	if (OriginalHorizontalAnchorType == HorizontalAnchorType && OriginalVerticalAnchorType == VerticalAnchorType)
		return true;

	if(OriginalHorizontalAnchorType == EHorizontalAnchorType::Custom)
	{
		if (OriginalVerticalAnchorType == VerticalAnchorType)
			return true;
	}

	if(OriginalVerticalAnchorType == EVerticalAnchorType::Custom)
	{
		if (OriginalHorizontalAnchorType == HorizontalAnchorType)
			return true;
	}

	if (OriginalVerticalAnchorType == EVerticalAnchorType::Custom && OriginalHorizontalAnchorType == EHorizontalAnchorType::Custom)
		return false;

	return false;
}
