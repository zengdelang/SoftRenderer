#include "Core/Culling/Clipping.h"

/////////////////////////////////////////////////////
// FClipping

FRect FClipping::FindCullAndClipRect(const TArray<URectMask2DSubComponent*>& RectMaskParents, bool& bValidRect)
{
	if (RectMaskParents.Num() == 0)
	{
		return FRect();
	}

	FRect Rect = RectMaskParents[0]->GetCanvasRect();
	float XMin = Rect.XMin;
	float XMax = Rect.GetXMax();
	float YMin = Rect.YMin;
	float YMax = Rect.GetYMax();

	for (int32 Index = 1, Count = RectMaskParents.Num(); Index < Count; ++Index)
	{
		Rect = RectMaskParents[Index]->GetCanvasRect();
		if (XMin < Rect.XMin)
		{
			XMin = Rect.XMin;
		}

		if (YMin < Rect.YMin)
		{
			YMin = Rect.YMin;
		}

		const float NewXMax = Rect.XMin + Rect.Width;
		if (XMax > NewXMax)
		{
			XMax = NewXMax;
		}

		const float NewYMax = Rect.YMin + Rect.Height;
		if (YMax > NewYMax)
		{
			YMax = NewYMax;
		}
	}

	bValidRect = XMax > XMin && YMax > YMin;
	if (bValidRect)
	{
		return FRect(XMin, YMin, XMax - XMin, YMax - YMin);
	}
	
	return FRect();
}

/////////////////////////////////////////////////////
