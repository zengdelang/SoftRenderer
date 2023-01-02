#include "Shape/EdgeHolder.h"

namespace SDFGenerator
{
	void FEdgeHolder::Swap(FEdgeHolder& A, FEdgeHolder& B)
	{
		FEdgeSegment* Temp = A.EdgeSegment;
		A.EdgeSegment = B.EdgeSegment;
		B.EdgeSegment = Temp;
	}

	FEdgeHolder::FEdgeHolder() : EdgeSegment(nullptr)
	{
	}

	FEdgeHolder::FEdgeHolder(FEdgeSegment* segment) : EdgeSegment(segment)
	{
	}

	FEdgeHolder::FEdgeHolder(FPoint2D P0, FPoint2D P1, EEdgeColor EdgeColor) : EdgeSegment(new FLinearSegment(P0, P1, EdgeColor))
	{
	}

	FEdgeHolder::FEdgeHolder(FPoint2D P0, FPoint2D P1, FPoint2D P2, EEdgeColor EdgeColor) : EdgeSegment(new FQuadraticSegment(P0, P1, P2, EdgeColor))
	{
	}

	FEdgeHolder::FEdgeHolder(FPoint2D P0, FPoint2D P1, FPoint2D P2, FPoint2D P3, EEdgeColor EdgeColor) : EdgeSegment(new FCubicSegment(P0, P1, P2, P3, EdgeColor))
	{
	}

	FEdgeHolder::FEdgeHolder(const FEdgeHolder& Orig) : EdgeSegment(Orig.EdgeSegment ? Orig.EdgeSegment->Clone() : nullptr)
	{
	}
	
	FEdgeHolder::FEdgeHolder(FEdgeHolder&& Orig) noexcept : EdgeSegment(Orig.EdgeSegment)
	{
		Orig.EdgeSegment = nullptr;
	}
	
	FEdgeHolder::~FEdgeHolder()
	{
		delete EdgeSegment;
	}

	FEdgeHolder& FEdgeHolder::operator=(const FEdgeHolder& Orig)
	{
		if (this != &Orig)
		{
			delete EdgeSegment;
			EdgeSegment = Orig.EdgeSegment ? Orig.EdgeSegment->Clone() : nullptr;
		}
		return *this;
	}


	FEdgeHolder& FEdgeHolder::operator=(FEdgeHolder&& Orig) noexcept
	{
		if (this != &Orig)
		{
			delete EdgeSegment;
			EdgeSegment = Orig.EdgeSegment;
			Orig.EdgeSegment = nullptr;
		}
		return *this;
	}


	FEdgeSegment& FEdgeHolder::operator*()
	{
		return *EdgeSegment;
	}

	const FEdgeSegment& FEdgeHolder::operator*() const
	{
		return *EdgeSegment;
	}

	FEdgeSegment* FEdgeHolder::operator->()
	{
		return EdgeSegment;
	}

	const FEdgeSegment* FEdgeHolder::operator->() const
	{
		return EdgeSegment;
	}

	FEdgeHolder::operator FEdgeSegment*()
	{
		return EdgeSegment;
	}

	FEdgeHolder::operator const FEdgeSegment*() const
	{
		return EdgeSegment;
	}
}
