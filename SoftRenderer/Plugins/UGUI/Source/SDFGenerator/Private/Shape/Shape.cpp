#include "Shape/Shape.h"
#include "Arithmetics.h"

namespace SDFGenerator
{
	FShape::FShape() : bInverseYAxis(false)
	{
	}

	void FShape::AddContour(const FContour& Contour)
	{
		Contours.Add(Contour);
	}

	void FShape::AddContour(FContour&& Contour)
	{
		Contours.Emplace(Contour);
	}

	FContour& FShape::AddContour()
	{
		Contours.SetNum(Contours.Num() + 1);
		return Contours.Last();
	}

	bool FShape::Validate() const
	{
		for (const auto& Contour : Contours)
		{
			if (Contour.Edges.Num() != 0)
			{
				FPoint2D Corner = Contour.Edges.Last()->Point(1);
				for (const auto& Edge : Contour.Edges)
				{
					if (!Edge)
						return false;

					if (Edge->Point(0) != Corner)
						return false;

					Corner = Edge->Point(1);
				}
			}
		}
		
		return true;
	}

	static void DeconvergeEdge(FEdgeHolder& EdgeHolder, int32 Param)
	{
		const auto EdgeSegment = &*EdgeHolder;

		if (EdgeSegment->GetType() == EEdgeSegmentType::QuadraticSegment)
		{
			const FQuadraticSegment* quadraticSegment = static_cast<const FQuadraticSegment*>(&*EdgeHolder);
			if (quadraticSegment)
				EdgeHolder = quadraticSegment->ConvertToCubic();
		}

		if (EdgeSegment->GetType() == EEdgeSegmentType::CubicSegment)
		{
			FCubicSegment* cubicSegment = static_cast<FCubicSegment*>(&*EdgeHolder);
			if (cubicSegment)
				cubicSegment->Deconverge(Param, SDF_GENERATOR_DECONVERGENCE_FACTOR);
		}
	}

	void FShape::Normalize()
	{
		for (auto& Contour : Contours)
		{
			if (Contour.Edges.Num() == 1)
			{
				FEdgeSegment* Parts[3] = {};
				Contour.Edges[0]->SplitInThirds(Parts[0], Parts[1], Parts[2]);
				Contour.Edges.Empty();
				Contour.Edges.Emplace(FEdgeHolder(Parts[0]));
				Contour.Edges.Emplace(FEdgeHolder(Parts[1]));
				Contour.Edges.Emplace(FEdgeHolder(Parts[2]));
			}
			else
			{
				FEdgeHolder* PrevEdge = &Contour.Edges.Last();
				for (auto& Edge : Contour.Edges)
				{
					FVector2 PrevDir = (*PrevEdge)->Direction(1).Normalize();
					FVector2 CurDir = Edge->Direction(0).Normalize();
					if (DotProduct(PrevDir, CurDir) < SDF_GENERATOR_CORNER_DOT_EPSILON - 1)
					{
						DeconvergeEdge(*PrevEdge, 1);
						DeconvergeEdge(Edge, 0);
					}
					PrevEdge = &Edge;
				}
			}
		}
	}

	void FShape::Bound(double& Left, double& Bottom, double& Right, double& Top) const
	{
		for (const auto& Contour : Contours)
		{
			Contour.Bound(Left, Bottom, Right, Top);
		}
	}

	void FShape::BoundMiters(double& Left, double& Bottom, double& Right, double& Top, double Border, double MiterLimit, int32 Polarity) const
	{
		for (const auto& Contour : Contours)
		{
			Contour.BoundMiters(Left, Bottom, Right, Top, Border, MiterLimit, Polarity);
		}
	}

	FShape::FBounds FShape::GetBounds(double Border, double MiterLimit, int32 Polarity) const
	{
		static constexpr double LARGE_VALUE = 1e240;
		FShape::FBounds Bounds = {+LARGE_VALUE, +LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE};
		Bound(Bounds.Left, Bounds.Bottom, Bounds.Right, Bounds.Top);
		if (Border > 0)
		{
			Bounds.Left -= Border, Bounds.Bottom -= Border;
			Bounds.Right += Border, Bounds.Top += Border;
			if (MiterLimit > 0)
				BoundMiters(Bounds.Left, Bounds.Bottom, Bounds.Right, Bounds.Top, Border, MiterLimit, Polarity);
		}
		return Bounds;
	}

	void FShape::Scanline(FScanline& Line, double Y) const
	{
		TArray<FScanline::FIntersection> Intersections;
		for (const auto& Contour : Contours)
		{
			for (const auto& Edge : Contour.Edges)
			{
				int32 Dy[3];
				double X[3];
				const int32 N = Edge->ScanlineIntersections(X, Dy, Y);
				for (int32 Index = 0; Index < N; ++Index)
				{
					FScanline::FIntersection Intersection = {X[Index], Dy[Index]};
					Intersections.Emplace(Intersection);
				}
			}
		}

		Line.SetIntersections(MoveTemp(Intersections));
	}

	int32 FShape::EdgeCount() const
	{
		int32 Total = 0;
		for (const auto& Contour : Contours)
		{
			Total += Contour.Edges.Num();
		}
		return Total;
	}
	
	void FShape::OrientContours()
	{
		struct FIntersection
		{
			double X;
			int32 Direction;
			int32 ContourIndex;
		};

		struct FCompareIntersections
		{
			FORCEINLINE bool operator()(const FIntersection& A, const FIntersection& B) const
			{
				return Sign(A.X - B.X) < 0;
			}
		};

		const double Ratio = .5 * (sqrt(5) - 1);
		
		// an irrational number to minimize chance of intersecting a corner or other point of interest
		TArray<int32> Orientations;
		Orientations.SetNumZeroed(Contours.Num());
		TArray<FIntersection> Intersections;
		
		for (int32 Index = 0, Count =  Contours.Num(); Index < Count; ++Index)
		{
			if (!Orientations[Index] && Contours[Index].Edges.Num() != 0)
			{
				// Find an Y that crosses the contour
				const double Y0 = Contours[Index].Edges[0]->Point(0).Y;
				double Y1 = Y0;

				for (const auto& Edge : Contours[Index].Edges)
				{
					if (Y0 != Y1)
						break;
					Y1 = Edge->Point(1).Y;
				}
			 
				for (const auto& Edge : Contours[Index].Edges)
				{
					if (Y0 != Y1)
						break;
					Y1 = Edge->Point(Ratio).Y; // in case all endpoints are in a horizontal line
				}

				const double Y = Mix(Y0, Y1, Ratio);

				// Scanline through whole shape at Y
				for (int32 J = 0, JCount = Contours.Num(); J < JCount; ++J)
				{
					for (const auto& Edge : Contours[J].Edges)
					{
						int32 Dy[3];
						double X[3];
						const int32 N = Edge->ScanlineIntersections(X, Dy, Y);
						for (int32 K = 0; K < N; ++K)
						{
							FIntersection Intersection = {X[K], Dy[K], J};
							Intersections.Emplace(Intersection);
						}
					}
				}

				Intersections.Sort(FCompareIntersections());
				
				// Disqualify multiple intersections
				for (int32 j = 1, JCount = Intersections.Num(); j < JCount; ++j)
				{
					if (Intersections[j].X == Intersections[j - 1].X)
					{
						Intersections[j].Direction = Intersections[j - 1].Direction = 0;
					}
				}
				
				// Inspect scanline and deduce orientations of intersected contours
				for (int32 j = 0, JCount = Intersections.Num(); j < JCount; ++j)
				{
					if (Intersections[j].Direction)
					{
						Orientations[Intersections[j].ContourIndex] += 2 * ((j & 1) ^ (Intersections[j].Direction > 0)) - 1;
					}
				}
				
				Intersections.Empty();
			}
		}
		
		// Reverse contours that have the opposite orientation
		for (int32 Index = 0, Count = Contours.Num(); Index < Count; ++Index)
		{
			if (Orientations[Index] < 0)
			{
				Contours[Index].Reverse();
			}
		}
	}
}
