#pragma once

#include "EdgetSegments.h"
#include "Vector2.h"
#include "SignedDistance/SignedDistance.h"

namespace SDFGenerator
{
	/**
	 * Selects the nearest edge by its true distance.
	 */
	class FTrueDistanceSelector
	{
	public:
		typedef double DistanceType;

		struct FEdgeCache
		{
			FPoint2D Point;
			double AbsDistance;

			FEdgeCache();
		};

		void Reset(const FPoint2D& InPoint);
		void AddEdge(FEdgeCache& Cache, const FEdgeSegment* PrevEdge, const FEdgeSegment* Edge,const FEdgeSegment* NextEdge);
		void Merge(const FTrueDistanceSelector& Other);
		DistanceType Distance() const;

	private:
		FPoint2D Point;
		FSignedDistance MinDistance;
	};
}
