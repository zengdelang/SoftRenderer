#include "Shape/EdgeSelectors.h"
#include "Arithmetics.h"

namespace SDFGenerator
{
	#define DISTANCE_DELTA_FACTOR 1.001

	FTrueDistanceSelector::FEdgeCache::FEdgeCache() : AbsDistance(0)
	{
	}

	void FTrueDistanceSelector::Reset(const FPoint2D& InPoint)
	{
		const double Delta = DISTANCE_DELTA_FACTOR * (InPoint - this->Point).Length();
		MinDistance.Distance += NonZeroSign(MinDistance.Distance) * Delta;
		this->Point = InPoint;
	}

	void FTrueDistanceSelector::AddEdge(FEdgeCache& Cache, const FEdgeSegment* PrevEdge, const FEdgeSegment* Edge, const FEdgeSegment* NextEdge)
	{
		const double Delta = DISTANCE_DELTA_FACTOR * (Point - Cache.Point).Length();
		if (Cache.AbsDistance - Delta <= fabs(MinDistance.Distance))
		{
			double Dummy;
			const FSignedDistance Distance = Edge->SignedDistance(Point, Dummy);
			if (Distance < MinDistance)
				MinDistance = Distance;
			
			Cache.Point = Point;
			Cache.AbsDistance = fabs(Distance.Distance);
		}
	}

	void FTrueDistanceSelector::Merge(const FTrueDistanceSelector& Other)
	{
		if (Other.MinDistance < MinDistance)
			MinDistance = Other.MinDistance;
	}

	FTrueDistanceSelector::DistanceType FTrueDistanceSelector::Distance() const
	{
		return MinDistance.Distance;
	}
}
