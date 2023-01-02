#include "Rasterization/Rasterization.h"
#include "Arithmetics.h"

namespace SDFGenerator
{
	void Rasterize(const FBitmapRef<float, 1>& Output, const FShape& Shape, const FProjection& Projection, EFillRule FillRule)
	{
		for (int32 Y = 0; Y < Output.Height; ++Y)
		{
			const int32 Row = Shape.bInverseYAxis ? Output.Height - Y - 1 : Y;
			
			FScanline Scanline;
			Shape.Scanline(Scanline, Projection.UnprojectY(Y + .5));
			
			for (int32 X = 0; X < Output.Width; ++X)
			{
				*Output(X, Row) = static_cast<float>(Scanline.Filled(Projection.UnprojectX(X + .5), FillRule));
			}
		}
	}

	void DistanceSignCorrection(const FBitmapRef<float, 1>& Sdf, const FShape& Shape, const FProjection& Projection, EFillRule FillRule)
	{
		for (int32 Y = 0; Y < Sdf.Height; ++Y)
		{
			const int32 Row = Shape.bInverseYAxis ? Sdf.Height - Y - 1 : Y;

			FScanline Scanline;
			Shape.Scanline(Scanline, Projection.UnprojectY(Y + .5));
			
			for (int32 X = 0; X < Sdf.Width; ++X)
			{
				const bool bFill = Scanline.Filled(Projection.UnprojectX(X+.5), FillRule);
				float &Sd = *Sdf(X, Row);
				if ((Sd > .5f) != bFill)
				{
					Sd = 1.f-Sd;
				}
			}
		}
	}

	template <int32 N>
	static void MultiDistanceSignCorrection(const FBitmapRef<float, N>& Sdf, const FShape& Shape, const FProjection& Projection, EFillRule FillRule)
	{
		const int32 W = Sdf.Width;
		const int32 H = Sdf.Height;
		if (!(W * H))
			return;
		
		FScanline Scanline;
		bool bAmbiguous = false;
		
		TArray<int8> MatchMap;
		MatchMap.Reserve(W * H);
		int8* Match = &MatchMap[0];
		
		for (int32 Y = 0; Y < H; ++Y)
		{
			int32 Row = Shape.bInverseYAxis ? H - Y - 1 : Y;
			Shape.Scanline(Scanline, Projection.UnprojectY(Y + .5));
			for (int32 X = 0; X < W; ++X)
			{
				const bool bFill = Scanline.Filled(Projection.UnprojectX(X + .5), FillRule);

				float* Msd = Sdf(X, Row);
				const float Sd = Median(Msd[0], Msd[1], Msd[2]);
				if (Sd == .5f)
				{
					bAmbiguous = true;
				}
				else if ((Sd > .5f) != bFill)
				{
					Msd[0] = 1.f - Msd[0];
					Msd[1] = 1.f - Msd[1];
					Msd[2] = 1.f - Msd[2];
					*Match = -1;
				}
				else
				{
					*Match = 1;
				}
				
				if (N >= 4 && (Msd[3] > .5f) != bFill)
				{
					Msd[3] = 1.f - Msd[3];
				}
				
				++Match;
			}
		}
		
		// This step is necessary to avoid artifacts when whole shape is inverted
		if (bAmbiguous)
		{
			Match = &MatchMap[0];
			for (int32 Y = 0; Y < H; ++Y)
			{
				int32 Row = Shape.bInverseYAxis ? H - Y - 1 : Y;
				for (int32 X = 0; X < W; ++X)
				{
					if (!*Match)
					{
						int32 NeighborMatch = 0;
						if (X > 0)
						{
							NeighborMatch += *(Match - 1);
						}
						
						if (X < W - 1)
						{
							NeighborMatch += *(Match + 1);
						}
						
						if (Y > 0)
						{
							NeighborMatch += *(Match - W);
						}
						
						if (Y < H - 1)
						{
							NeighborMatch += *(Match + W);
						}
						
						if (NeighborMatch < 0)
						{
							float* Msd = Sdf(X, Row);
							Msd[0] = 1.f - Msd[0];
							Msd[1] = 1.f - Msd[1];
							Msd[2] = 1.f - Msd[2];
						}
					}
					++Match;
				}
			}
		}
	}

	void DistanceSignCorrection(const FBitmapRef<float, 3>& Sdf, const FShape& Shape, const FProjection& Projection, EFillRule FillRule)
	{
		MultiDistanceSignCorrection(Sdf, Shape, Projection, FillRule);
	}

	void DistanceSignCorrection(const FBitmapRef<float, 4>& Sdf, const FShape& Shape, const FProjection& Projection, EFillRule FillRule)
	{
		MultiDistanceSignCorrection(Sdf, Shape, Projection, FillRule);
	}
}
