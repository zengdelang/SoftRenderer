#pragma once

#include "Vector2.h"
#include "Shape/ContourCombiners.h"

namespace SDFGenerator
{
	/**
	 * Finds the distance between a point and a Shape. ContourCombiner dictates the distance metric and its data type.
	 */
	template <class TContourCombiner>
	class FShapeDistanceFinder
	{
	public:
		typedef typename TContourCombiner::DistanceType DistanceType;

		/**
		 * Passed shape object must persist until the distance finder is destroyed!
		 */
		explicit FShapeDistanceFinder(const FShape& Shape)
			: Shape(Shape)
			, ContourCombiner(Shape)
		{
			ShapeEdgeCache.SetNum(Shape.EdgeCount());
		}
		
		/**
		 * Finds the distance from origin. Not thread-safe! Is fastest when subsequent queries are close together.
		 */
		DistanceType Distance(const FPoint2D& Origin)
		{
			ContourCombiner.Reset(Origin);
			typename TContourCombiner::EdgeSelectorType::FEdgeCache* EdgeCache = &ShapeEdgeCache[0];

			for (int32 Index = 0, Count = Shape.Contours.Num(); Index < Count; ++Index)
			{
				const auto& Contour = Shape.Contours[Index];
				if (Contour.Edges.Num() != 0)
				{
					typename TContourCombiner::EdgeSelectorType& EdgeSelector = ContourCombiner.EdgeSelector(Index);

					const FEdgeSegment* PrevEdge = Contour.Edges.Num() >= 2 ? Contour.Edges[Contour.Edges.Num() - 2] : Contour.Edges[0];
					const FEdgeSegment* CurEdge = Contour.Edges.Last();
					for (const auto& Edge : Contour.Edges)
					{
						const FEdgeSegment* NextEdge = Edge;
						EdgeSelector.AddEdge(*EdgeCache++, PrevEdge, CurEdge, NextEdge);
						PrevEdge = CurEdge;
						CurEdge = NextEdge;
					}
				}
			}

			return ContourCombiner.Distance();
		}

		/**
		 * Finds the distance between shape and origin. Does not allocate result cache used to optimize performance of multiple queries.
		 */
		static DistanceType OneShotDistance(const FShape& Shape, const FPoint2D& Origin)
		{
			TContourCombiner ContourCombiner(Shape);
			ContourCombiner.Reset(Origin);

			for (int32 Index = 0, Count = Shape.Contours.Num(); Index < Count; ++Index)
			{
				const auto& Contour = Shape.Contours[Index];
				if (Contour.Edges.Num() != 0)
				{
					typename TContourCombiner::EdgeSelectorType& EdgeSelector = ContourCombiner.EdgeSelector(Index);

					const FEdgeSegment* PrevEdge = Contour.Edges.Num() >= 2 ? Contour.Edges[Contour.Edges.Num() - 2] : Contour.Edges[0];
					const FEdgeSegment* CurEdge = Contour.Edges.Last();
					for (const auto& Edge : Contour.Edges)
					{
						const FEdgeSegment* NextEdge = Edge;
						typename TContourCombiner::EdgeSelectorType::FEdgeCache Dummy;
						EdgeSelector.AddEdge(Dummy, PrevEdge, CurEdge, NextEdge);
						PrevEdge = CurEdge;
						CurEdge = NextEdge;
					}
				}
			}

			return ContourCombiner.Distance();
		}
		
	private:
		const FShape& Shape;
		TContourCombiner ContourCombiner;
		TArray<typename TContourCombiner::EdgeSelectorType::FEdgeCache> ShapeEdgeCache;
	};

	typedef FShapeDistanceFinder<FSimpleContourCombiner<FTrueDistanceSelector>> FSimpleTrueShapeDistanceFinder;
}
