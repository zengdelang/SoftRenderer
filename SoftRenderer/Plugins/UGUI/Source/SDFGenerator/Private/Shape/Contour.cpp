#include "Shape/Contour.h"
#include "Arithmetics.h"

namespace SDFGenerator
{
	static double Shoelace(const FPoint2D& A, const FPoint2D& B)
	{
		return (B.X - A.X) * (A.Y + B.Y);
	}

	void FContour::AddEdge(const FEdgeHolder& Edge)
	{
		Edges.Add(Edge);
	}

	void FContour::AddEdge(FEdgeHolder&& Edge)
	{
		Edges.Emplace(Edge);
	}
	
	FEdgeHolder& FContour::AddEdge()
	{
		Edges.SetNum(Edges.Num() + 1);
		return Edges.Last();
	}

	static void BoundPoint(double& Left, double& Bottom, double& Right, double& Top, FPoint2D Point)
	{
		if (Point.X < Left) Left = Point.X;
		if (Point.Y < Bottom) Bottom = Point.Y;
		if (Point.X > Right) Right = Point.X;
		if (Point.Y > Top) Top = Point.Y;
	}

	void FContour::Bound(double& Left, double& Bottom, double& Right, double& Top) const
	{
		for (const auto& Edge : Edges)
		{
			Edge->Bound(Left, Bottom, Right, Top);
		}
	}

	void FContour::BoundMiters(double& Left, double& Bottom, double& Right, double& Top, double Border, double MiterLimit, int32 Polarity) const
	{
		if (Edges.Num() == 0)
			return;
		
		FVector2 PrevDir = Edges.Last()->Direction(1).Normalize(true);
		for (const auto& Edge : Edges)
		{
			FVector2 Dir = -Edge->Direction(0).Normalize(true);
			if (Polarity * CrossProduct(PrevDir, Dir) >= 0)
			{
				double MiterLength = MiterLimit;

				const double Q = .5 * (1 - DotProduct(PrevDir, Dir));
				if (Q > 0)
				{
					MiterLength = Min(1 / sqrt(Q), MiterLimit);
				}

				const FPoint2D Miter = Edge->Point(0) + Border * MiterLength * (PrevDir + Dir).Normalize(true);
				BoundPoint(Left, Bottom, Right, Top, Miter);
			}
			
			PrevDir = Edge->Direction(1).Normalize(true);
		}
	}

	int32 FContour::Winding() const
	{
		if (Edges.Num() == 0)
			return 0;
		
		double Total = 0;
		if (Edges.Num() == 1)
		{
			const FPoint2D A = Edges[0]->Point(0), B = Edges[0]->Point(1 / 3.), C = Edges[0]->Point(2 / 3.);
			Total += Shoelace(A, B);
			Total += Shoelace(B, C);
			Total += Shoelace(C, A);
		}
		else if (Edges.Num() == 2)
		{
			const FPoint2D A = Edges[0]->Point(0), B = Edges[0]->Point(.5), C = Edges[1]->Point(0), D = Edges[1]->Point(.5);
			Total += Shoelace(A, B);
			Total += Shoelace(B, C);
			Total += Shoelace(C, D);
			Total += Shoelace(D, A);
		}
		else
		{
			FPoint2D Prev = Edges.Last()->Point(0);
			for (const auto& Edge : Edges)
			{
				FPoint2D Cur = Edge->Point(0);
				Total += Shoelace(Prev, Cur);
				Prev = Cur;
			}
		}
		return Sign(Total);
	}

	void FContour::Reverse()
	{
		for (int32 Index = Edges.Num() / 2; Index > 0; --Index)
		{
			FEdgeHolder::Swap(Edges[Index - 1], Edges[Edges.Num() - Index]);
		}
		
		for (auto& Edge : Edges)
		{
			Edge->Reverse();
		}
	}
}
