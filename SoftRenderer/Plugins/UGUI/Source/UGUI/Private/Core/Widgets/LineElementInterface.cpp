#include "Core/Widgets/LineElementInterface.h"
#include "Core/Layout/RectTransformComponent.h"

/////////////////////////////////////////////////////
// ILineElementInterface

TArray<FVector2D> ILineElementInterface::RectPoints;

void ILineElementInterface::GenerateRectPoints(ELineType LineType, bool bClosedLine)
{
	RectPoints.Reset();

	const FRect TransformRect = GetTransformRect();
	const float XMax = TransformRect.GetXMax();
	const float YMax = TransformRect.GetYMax();
	
	switch (LineType)
	{
	case ELineType::RectBox:
		RectPoints.Emplace(FVector2D(TransformRect.XMin, TransformRect.YMin));
		RectPoints.Emplace(FVector2D(TransformRect.XMin, YMax));
		RectPoints.Emplace(FVector2D(XMax, YMax));
		RectPoints.Emplace(FVector2D(XMax, TransformRect.YMin));
		
		if (!bClosedLine)
		{
			RectPoints.Emplace(FVector2D(TransformRect.XMin, TransformRect.YMin));
		}
		break;
	case ELineType::RectLeft:
		RectPoints.Emplace(FVector2D(TransformRect.XMin, TransformRect.YMin));
		RectPoints.Emplace(FVector2D(TransformRect.XMin, YMax));
		break;
	case ELineType::RectTop:
		RectPoints.Emplace(FVector2D(TransformRect.XMin, YMax));
		RectPoints.Emplace(FVector2D(XMax, YMax));
		break;
	case ELineType::RectRight:
		RectPoints.Emplace(FVector2D(XMax, YMax));
		RectPoints.Emplace(FVector2D(XMax, TransformRect.YMin));
		break;
	case ELineType::RectBottom:
		RectPoints.Emplace(FVector2D(TransformRect.XMin, TransformRect.YMin));
		RectPoints.Emplace(FVector2D(XMax, TransformRect.YMin));
		break;
	default:
		break;
	}
}

/////////////////////////////////////////////////////

