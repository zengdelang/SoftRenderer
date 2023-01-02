#include "Rasterization/Scanline.h"
#include "Arithmetics.h"

namespace SDFGenerator
{
	struct FCompareIntersections
	{
		FORCEINLINE bool operator()(const FScanline::FIntersection& A, const FScanline::FIntersection& B) const
		{
			return Sign(A.X - B.X) < 0;
		}
	};

	bool InterpretFillRule(int32 Intersections, EFillRule FillRule)
	{
		switch (FillRule)
		{
		case EFillRule::FILL_NONZERO:
			return Intersections != 0;
		case EFillRule::FILL_ODD:
			return Intersections & 1;
		case EFillRule::FILL_POSITIVE:
			return Intersections > 0;
		case EFillRule::FILL_NEGATIVE:
			return Intersections < 0;
		}
		return false;
	}

	double FScanline::Overlap(const FScanline& A, const FScanline& B, double xFrom, double xTo, EFillRule FillRule)
	{
		double Total = 0;
		bool aInside = false, bInside = false;
		int32 Ai = 0, Bi = 0;
		double Ax = A.Intersections.Num() != 0 ? A.Intersections[Ai].X : xTo;
		double Bx = B.Intersections.Num() != 0 ? B.Intersections[Bi].X : xTo;
		
		while (Ax < xFrom || Bx < xFrom)
		{
			const double xNext = Min(Ax, Bx);
			
			if (Ax == xNext && Ai < A.Intersections.Num())
			{
				aInside = InterpretFillRule(A.Intersections[Ai].Direction, FillRule);
				Ax = ++Ai < A.Intersections.Num() ? A.Intersections[Ai].X : xTo;
			}
			
			if (Bx == xNext && Bi < B.Intersections.Num())
			{
				bInside = InterpretFillRule(B.Intersections[Bi].Direction, FillRule);
				Bx = ++Bi < B.Intersections.Num() ? B.Intersections[Bi].X : xTo;
			}
		}
		
		double X = xFrom;
		while (Ax < xTo || Bx < xTo)
		{
			const double xNext = Min(Ax, Bx);
			if (aInside == bInside)
				Total += xNext - X;
			
			if (Ax == xNext && Ai < A.Intersections.Num())
			{
				aInside = InterpretFillRule(A.Intersections[Ai].Direction, FillRule);
				Ax = ++Ai < A.Intersections.Num() ? A.Intersections[Ai].X : xTo;
			}
			
			if (Bx == xNext && Bi < B.Intersections.Num())
			{
				bInside = InterpretFillRule(B.Intersections[Bi].Direction, FillRule);
				Bx = ++Bi < B.Intersections.Num() ? B.Intersections[Bi].X : xTo;
			}
			
			X = xNext;
		}
		
		if (aInside == bInside)
			Total += xTo - X;
		
		return Total;
	}

	FScanline::FScanline() : LastIndex(0)
	{
	}

	void FScanline::Preprocess()
	{
		LastIndex = 0;
		if (Intersections.Num() != 0)
		{
			Intersections.Sort(FCompareIntersections());
			
			int32 TotalDirection = 0;
			for (auto& Intersection : Intersections)
			{
				TotalDirection += Intersection.Direction;
				Intersection.Direction = TotalDirection;
			}
		}
	}

	void FScanline::SetIntersections(const TArray<FIntersection>& InIntersections)
	{
		this->Intersections = InIntersections;
		Preprocess();
	}

	void FScanline::SetIntersections(TArray<FIntersection>&& InIntersections)
	{
		this->Intersections = MoveTemp(InIntersections);
		Preprocess();
	}
	
	int32 FScanline::MoveTo(double X) const
	{
		if (Intersections.Num() == 0)
			return -1;
		
		int32 Index = LastIndex;
		if (X < Intersections[Index].X)
		{
			do
			{
				if (Index == 0)
				{
					LastIndex = 0;
					return -1;
				}
				--Index;
			}
			while (X < Intersections[Index].X);
		}
		else
		{
			const int32 IntersectionNum = Intersections.Num();
			while (Index < IntersectionNum - 1 && X >= Intersections[Index + 1].X)
			{
				++Index;
			}
		}
		LastIndex = Index;
		return Index;
	}

	int32 FScanline::CountIntersections(double X) const
	{
		return MoveTo(X) + 1;
	}

	int32 FScanline::SumIntersections(double X) const
	{
		const int32 Index = MoveTo(X);
		if (Index >= 0)
			return Intersections[Index].Direction;
		return 0;
	}

	bool FScanline::Filled(double X, EFillRule FillRule) const
	{
		return InterpretFillRule(SumIntersections(X), FillRule);
	}
}
