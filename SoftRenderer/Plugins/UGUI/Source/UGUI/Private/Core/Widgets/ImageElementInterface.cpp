#include "Core/Widgets/ImageElementInterface.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/VertexHelper.h"

/////////////////////////////////////////////////////
// UImageElementInterface
 
UImageElementInterface::UImageElementInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void IImageElementInterface::AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color, FVector2D InQuadUVs[], FVector2D UV1,
	EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect)
{
	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

	if (MaskType == EImageMaskMode::MaskMode_Circle || MaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			const auto& QuadPos = InQuadPositions[Index];
			VertexHelper.AddVert(FVector(InQuadPositions[Index], 0), Color, InQuadUVs[Index], UV1,FVector2D::ZeroVector,
			FVector2D((QuadPos.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (QuadPos.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		}
	}
	else
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			VertexHelper.AddVert(FVector(InQuadPositions[Index], 0), Color, InQuadUVs[Index], UV1);
		}
	}

	VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
	VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex);
}

void IImageElementInterface::AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color,
	FVector2D InQuadUVs[], FVector2D UV1, FVector2D InQuadUV2s[], EImageMaskMode MaskType, FVector2D UV5,
	const FRect& ViewRect)
{
	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

	if (MaskType == EImageMaskMode::MaskMode_Circle || MaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			const auto& QuadPos = InQuadPositions[Index];
			VertexHelper.AddVert(FVector(InQuadPositions[Index], 0), Color, InQuadUVs[Index], UV1,InQuadUV2s[Index],
			FVector2D((QuadPos.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (QuadPos.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		}
	}
	else
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			VertexHelper.AddVert(FVector(InQuadPositions[Index], 0), Color, InQuadUVs[Index], UV1, InQuadUV2s[Index]);
		}
	}

	VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
	VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex);
}

void IImageElementInterface::AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color,
	FVector2D InQuadUVs[], FVector2D UV1, FVector2D InQuadUV2s[], EImageMaskMode MaskType, FVector2D UV5,
	const FRect& ViewRect, const int32 StartPointIndex)
{
	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

	if (MaskType == EImageMaskMode::MaskMode_Circle || MaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			int32 PointIndex = (StartPointIndex + Index) % 4;
			const auto& QuadPos = InQuadPositions[PointIndex];
			VertexHelper.AddVert(FVector(InQuadPositions[PointIndex], 0), Color, InQuadUVs[PointIndex], UV1,InQuadUV2s[PointIndex],
			FVector2D((QuadPos.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (QuadPos.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		}
	}
	else
	{
		for (int32 Index = 0; Index < 4; ++Index)
		{
			int32 PointIndex = (StartPointIndex + Index) % 4;
			VertexHelper.AddVert(FVector(InQuadPositions[PointIndex], 0), Color, InQuadUVs[PointIndex], UV1, InQuadUV2s[PointIndex]);
		}
	}

	VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
	VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex);
}

void IImageElementInterface::AddQuad(FVertexHelper& VertexHelper, FVector2D PosMin, FVector2D PosMax, FLinearColor Color, FVector2D UVMin, FVector2D UVMax, FVector2D UV1,
                                     EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect)
{
	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

	if (MaskType == EImageMaskMode::MaskMode_Circle || MaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		VertexHelper.AddVert(FVector(PosMin.X, PosMin.Y, 0), Color, FVector2D(UVMin.X, UVMin.Y), UV1, FVector2D::ZeroVector,
			FVector2D((PosMin.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMin.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMin.X, PosMax.Y, 0), Color, FVector2D(UVMin.X, UVMax.Y), UV1, FVector2D::ZeroVector,
			FVector2D((PosMin.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMax.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMax.X, PosMax.Y, 0), Color, FVector2D(UVMax.X, UVMax.Y), UV1, FVector2D::ZeroVector,
			FVector2D((PosMax.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMax.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMax.X, PosMin.Y, 0), Color, FVector2D(UVMax.X, UVMin.Y), UV1, FVector2D::ZeroVector,
			FVector2D((PosMax.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMin.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
	}
	else
	{
		VertexHelper.AddVert(FVector(PosMin.X, PosMin.Y, 0), Color, FVector2D(UVMin.X, UVMin.Y), UV1);
		VertexHelper.AddVert(FVector(PosMin.X, PosMax.Y, 0), Color, FVector2D(UVMin.X, UVMax.Y), UV1);
		VertexHelper.AddVert(FVector(PosMax.X, PosMax.Y, 0), Color, FVector2D(UVMax.X, UVMax.Y), UV1);
		VertexHelper.AddVert(FVector(PosMax.X, PosMin.Y, 0), Color, FVector2D(UVMax.X, UVMin.Y), UV1);
	}

	VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
	VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex);
}

void IImageElementInterface::AddQuad(FVertexHelper& VertexHelper, FVector2D PosMin, FVector2D PosMax,
	FLinearColor Color, FVector2D UVMin, FVector2D UVMax, FVector2D UV1, FVector2D UV2Min, FVector2D UV2Max,
	EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect)
{
	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

	if (MaskType == EImageMaskMode::MaskMode_Circle || MaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		VertexHelper.AddVert(FVector(PosMin.X, PosMin.Y, 0), Color, FVector2D(UVMin.X, UVMin.Y), UV1, FVector2D(UV2Min.X, UV2Min.Y),
			FVector2D((PosMin.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMin.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMin.X, PosMax.Y, 0), Color, FVector2D(UVMin.X, UVMax.Y), UV1, FVector2D(UV2Min.X, UV2Max.Y),
			FVector2D((PosMin.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMax.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMax.X, PosMax.Y, 0), Color, FVector2D(UVMax.X, UVMax.Y), UV1, FVector2D(UV2Max.X, UV2Max.Y),
			FVector2D((PosMax.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMax.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
		VertexHelper.AddVert(FVector(PosMax.X, PosMin.Y, 0), Color, FVector2D(UVMax.X, UVMin.Y), UV1, FVector2D(UV2Max.X, UV2Min.Y),
			FVector2D((PosMax.X - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Width), (PosMin.Y - ViewRect.XMin) / FMath::Max(0.0001f, ViewRect.Height)), UV5);
	}
	else
	{
		VertexHelper.AddVert(FVector(PosMin.X, PosMin.Y, 0), Color, FVector2D(UVMin.X, UVMin.Y), UV1, FVector2D(UV2Min.X, UV2Min.Y));
		VertexHelper.AddVert(FVector(PosMin.X, PosMax.Y, 0), Color, FVector2D(UVMin.X, UVMax.Y), UV1, FVector2D(UV2Min.X, UV2Max.Y));
		VertexHelper.AddVert(FVector(PosMax.X, PosMax.Y, 0), Color, FVector2D(UVMax.X, UVMax.Y), UV1, FVector2D(UV2Max.X, UV2Max.Y));
		VertexHelper.AddVert(FVector(PosMax.X, PosMin.Y, 0), Color, FVector2D(UVMax.X, UVMin.Y), UV1, FVector2D(UV2Max.X, UV2Min.Y));
	}

	VertexHelper.AddTriangle(StartIndex, StartIndex + 1, StartIndex + 2);
	VertexHelper.AddTriangle(StartIndex + 2, StartIndex + 3, StartIndex);
}

bool IImageElementInterface::RadialCut(FVector2D InQuadPositions[], FVector2D InQuadUVs[], float Fill, bool bInvert, int32 Corner)
{
	// Nothing to fill
	if (Fill < 0.001f)
		return false;

	// Even corners invert the fill direction
	if ((Corner & 1) == 1)
		bInvert = !bInvert;

	// Nothing to adjust
	if (!bInvert && Fill > 0.999f)
		return true;

	// Convert 0-1 value into 0 to 90 degrees angle in radians
	float Angle = FMath::Clamp(Fill, 0.0f, 1.0f);
	if (bInvert)
		Angle = 1 - Angle;
	Angle *= FMath::DegreesToRadians(90);

	// Calculate the effective X and Y factors
	float Cos = FMath::Cos(Angle);
	float Sin = FMath::Sin(Angle);

	RadialCut(InQuadPositions, Cos, Sin, bInvert, Corner);
	RadialCut(InQuadUVs, Cos, Sin, bInvert, Corner);
	return true;
}

bool IImageElementInterface::RadialCut(FVector2D InQuadPositions[], FVector2D InQuadUVs[], FVector2D InQuadUV2s[],
	float Fill, bool bInvert, int32 Corner)
{
	// Nothing to fill
	if (Fill < 0.001f)
		return false;

	// Even corners invert the fill direction
	if ((Corner & 1) == 1)
		bInvert = !bInvert;

	// Nothing to adjust
	if (!bInvert && Fill > 0.999f)
		return true;

	// Convert 0-1 value into 0 to 90 degrees angle in radians
	float Angle = FMath::Clamp(Fill, 0.0f, 1.0f);
	if (bInvert)
		Angle = 1 - Angle;
	Angle *= FMath::DegreesToRadians(90);

	// Calculate the effective X and Y factors
	float Cos = FMath::Cos(Angle);
	float Sin = FMath::Sin(Angle);

	RadialCut(InQuadPositions, Cos, Sin, bInvert, Corner);
	RadialCut(InQuadUVs, Cos, Sin, bInvert, Corner);
	RadialCutUV2(InQuadUV2s, Cos, Sin, bInvert, Corner);
	return true;
}

void IImageElementInterface::RadialCut(FVector2D InQuadXY[], float Cos, float Sin, bool bInvert, int32 Corner)
{
	const int32 Index0 = Corner;
	const int32 Index1 = ((Corner + 1) % 4);
	const int32 Index2 = ((Corner + 2) % 4);
	const int32 Index3 = ((Corner + 3) % 4);

	if ((Corner & 1) == 1)
	{
		if (Sin > Cos)
		{
			Cos /= Sin;
			Sin = 1;

			if (bInvert)
			{
				InQuadXY[Index1].X = FMath::Lerp(InQuadXY[Index0].X, InQuadXY[Index2].X, Cos);
				InQuadXY[Index2].X = InQuadXY[Index1].X;
			}
		}
		else if (Cos > Sin)
		{
			Sin /= Cos;
			Cos = 1;

			if (!bInvert)
			{
				InQuadXY[Index2].Y = FMath::Lerp(InQuadXY[Index0].Y, InQuadXY[Index2].Y, Sin);
				InQuadXY[Index3].Y = InQuadXY[Index2].Y;
			}
		}
		else
		{
			Cos = 1;
			Sin = 1;
		}

		if (!bInvert)
		{
			InQuadXY[Index3].X = FMath::Lerp(InQuadXY[Index0].X, InQuadXY[Index2].X, Cos);
		}
		else
		{
			InQuadXY[Index1].Y = FMath::Lerp(InQuadXY[Index0].Y, InQuadXY[Index2].Y, Sin);
		}
	}
	else
	{
		if (Cos > Sin)
		{
			Sin /= Cos;
			Cos = 1;

			if (!bInvert)
			{
				InQuadXY[Index1].Y = FMath::Lerp(InQuadXY[Index0].Y, InQuadXY[Index2].Y, Sin);
				InQuadXY[Index2].Y = InQuadXY[Index1].Y;
			}
		}
		else if (Sin > Cos)
		{
			Cos /= Sin;
			Sin = 1;

			if (bInvert)
			{
				InQuadXY[Index2].X = FMath::Lerp(InQuadXY[Index0].X, InQuadXY[Index2].X, Cos);
				InQuadXY[Index3].X = InQuadXY[Index2].X;
			}
		}
		else
		{
			Cos = 1;
			Sin = 1;
		}

		if (bInvert)
		{
			InQuadXY[Index3].Y = FMath::Lerp(InQuadXY[Index0].Y, InQuadXY[Index2].Y, Sin);
		}
		else
		{
			InQuadXY[Index1].X = FMath::Lerp(InQuadXY[Index0].X, InQuadXY[Index2].X, Cos);
		}
	}
}

void IImageElementInterface::RadialCutUV2(FVector2D InQuadXY[], float Cos, float Sin, bool bInvert, int32 Corner)
{
	const int32 Index0 = Corner;
	const int32 Index1 = ((Corner + 1) % 4);
	const int32 Index2 = ((Corner + 2) % 4);
	const int32 Index3 = ((Corner + 3) % 4);

	if (FMath::IsNearlyZero(Cos - 1) || FMath::IsNearlyZero(Sin - 1))
	{
		return;
	}
	
	if (Corner == 0)
	{
		if (!bInvert)
		{
			if (Cos > Sin)
			{
				InQuadXY[Index0] = FVector2D(1, 0);
				InQuadXY[Index1] = FVector2D(1, 1);
				InQuadXY[Index2] = FVector2D(1, 1);
				InQuadXY[Index3] = FVector2D(0, 1);
			}
			else
			{
				InQuadXY[Index0] = FVector2D(1, 0);
				InQuadXY[Index1] = FVector2D(1, 1);
				InQuadXY[Index2] = FVector2D(0, 1);
				InQuadXY[Index3] = FVector2D(0, 1);
			}
		}
		else
		{
			if (Cos > Sin)
			{
				InQuadXY[Index0] = FVector2D(1, 0);
				InQuadXY[Index1] = FVector2D(0, 1);
				InQuadXY[Index2] = FVector2D(0, 1);
				InQuadXY[Index3] = FVector2D(1, 1);
			}
			else
			{
				InQuadXY[Index0] = FVector2D(1, 0);
				InQuadXY[Index1] = FVector2D(0, 1);
				InQuadXY[Index2] = FVector2D(1, 1);
				InQuadXY[Index3] = FVector2D(1, 1);
			}
		}
	}
	else if (Corner == 1)
	{
		if (!bInvert)
		{
			InQuadXY[Index0] = FVector2D(-1, 0);
			InQuadXY[Index1] = FVector2D(1, 0);
			InQuadXY[Index2] = FVector2D(1, 1);
			InQuadXY[Index3] = FVector2D(-1, -1);
		}
		else
		{
			InQuadXY[Index0] = FVector2D(0, 1);
			InQuadXY[Index1] = FVector2D(1, 1);
			InQuadXY[Index2] = FVector2D(1, -1);
			InQuadXY[Index3] = FVector2D(0, -1);
		}
	}
	else if (Corner == 2)
	{
		if (!bInvert)
		{
			if (Cos > Sin)
			{
				InQuadXY[Index0] = FVector2D(-1, 0);
				InQuadXY[Index1] = FVector2D(-1, -1);
				InQuadXY[Index2] = FVector2D(-1, -1);
				InQuadXY[Index3] = FVector2D(0, -1);
			}
			else
			{
				InQuadXY[Index0] = FVector2D(-1, 0);
				InQuadXY[Index1] = FVector2D(-1, -1);
				InQuadXY[Index2] = FVector2D(0, -1);
				InQuadXY[Index3] = FVector2D(0, -1);
			}
		}
		else
		{
			if (Cos > Sin)
			{
				InQuadXY[Index0] = FVector2D(-1, 0);
				InQuadXY[Index1] = FVector2D(0, -1);
				InQuadXY[Index2] = FVector2D(0, -1);
				InQuadXY[Index3] = FVector2D(-1, -1);
			}
			else
			{
				InQuadXY[Index0] = FVector2D(-1, 0);
				InQuadXY[Index1] = FVector2D(0, -1);
				InQuadXY[Index2] = FVector2D(-1, -1);
				InQuadXY[Index3] = FVector2D(-1, -1);
			}
		}
	}
	else
	{
		if (!bInvert)
		{
			InQuadXY[Index0] = FVector2D(-1, 0);
			InQuadXY[Index1] = FVector2D(1, 0);
			InQuadXY[Index2] = FVector2D(1, 1);
			InQuadXY[Index3] = FVector2D(-1, 1);
		}
		else
		{
			InQuadXY[Index0] = FVector2D(1, 0);
			InQuadXY[Index1] = FVector2D(1, 0);
			InQuadXY[Index2] = FVector2D(1, 1);
			InQuadXY[Index3] = FVector2D(0, 1);
		}
	}
}

void IImageElementInterface::RadialCutUV2(FVector2D InQuadUV2XY[], float Fill, bool bClockwise, int32 StartQuadIndex,
	int32 QuadIndex, int32 CornerIndex)
{
	bool bIsStart = StartQuadIndex == QuadIndex;
	int32 IndexInvertVal = bClockwise ? 1 : -1;

	//trapezoid
	if (bIsStart)
	{
		InQuadUV2XY[(IndexInvertVal * 0 + CornerIndex + 4) % 4] = FVector2D(-1, -1);
		InQuadUV2XY[(IndexInvertVal * 1 + CornerIndex + 4) % 4] = FVector2D(-1, 1);
		InQuadUV2XY[(IndexInvertVal * 2 + CornerIndex + 4) % 4] = FVector2D(1, 1);
		InQuadUV2XY[(IndexInvertVal * 3 + CornerIndex + 4) % 4] = FVector2D(1, -1);
	}
	else
	{
		InQuadUV2XY[(IndexInvertVal * 0 + CornerIndex + 4) % 4] = FVector2D(1, 0);
		InQuadUV2XY[(IndexInvertVal * 1 + CornerIndex + 4) % 4] = FVector2D(0, 1);
		InQuadUV2XY[(IndexInvertVal * 2 + CornerIndex + 4) % 4] = FVector2D(0, 1);
		InQuadUV2XY[(IndexInvertVal * 3 + CornerIndex + 4) % 4] = FVector2D(1, 1);
	}
	
	
	if (Fill < 0.001)
	{
		return;
	}
	else if (Fill <= 0.5)
	{
		//triangle
		if (bIsStart)
		{
			InQuadUV2XY[(IndexInvertVal * 2 + CornerIndex + 4) % 4] = FVector2D(1, -1);
		}
		else
		{
			InQuadUV2XY[(IndexInvertVal * 2 + CornerIndex + 4) % 4] = FVector2D(1, 1);
		}
	}
	else if (Fill <= 1)
	{
		//trapezoid
	}
	else
	{
		//rect
		if (bIsStart)
		{
			InQuadUV2XY[(IndexInvertVal * 0 + CornerIndex + 4) % 4] = FVector2D(-1, 0);
			InQuadUV2XY[(IndexInvertVal * 3 + CornerIndex + 4) % 4] = FVector2D(1, 0);
		}
		else
		{
			InQuadUV2XY[(IndexInvertVal * 3 + CornerIndex + 4) % 4] = FVector2D(0, 1);
		}
	}
}

/////////////////////////////////////////////////////
