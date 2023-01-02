#pragma once

#include "Shape.h"
#include "EdgeSelectors.h"

namespace SDFGenerator
{
	static void InitDistance(double& Distance)
	{
		Distance = FSignedDistance::INFINITE.Distance;
	}

	static double ResolveDistance(double Distance)
	{
		return Distance;
	}
	
	/**
	 * Simply selects the nearest contour.
	 */
	template <class TEdgeSelector>
	class FSimpleContourCombiner
	{
	public:
		typedef TEdgeSelector EdgeSelectorType;
		typedef typename TEdgeSelector::DistanceType DistanceType;

		explicit FSimpleContourCombiner(const FShape& Shape)
		{
			
		}
		
		void Reset(const FPoint2D& InPoint)
		{
			ShapeEdgeSelector.Reset(InPoint);
		}
		
		EdgeSelectorType& EdgeSelector(int32 Index)
		{
			return ShapeEdgeSelector;
		}
		
		DistanceType Distance() const
		{
			return ShapeEdgeSelector.Distance();
		}

	private:
		EdgeSelectorType ShapeEdgeSelector;
	};

	/**
	 * Selects the nearest contour that actually forms a border between filled and unfilled area.
	 */
	template <class TEdgeSelector>
	class FOverlappingContourCombiner
	{
	public:
		typedef TEdgeSelector EdgeSelectorType;
		typedef typename TEdgeSelector::DistanceType DistanceType;

		explicit FOverlappingContourCombiner(const FShape& Shape)
		{
			Windings.Reserve(Shape.Contours.Num());
			for (const auto& Contour : Shape.Contours)
			{
				Windings.Add(Contour.Winding());
			}
			EdgeSelectors.SetNum(Shape.Contours.Num());
		}
		
		void Reset(const FPoint2D& InPoint)
		{
			this->Point = InPoint;
			for (auto& ContourEdgeSelector : EdgeSelectors)
			{
				ContourEdgeSelector.Reset(InPoint);
			}
		}
		
		EdgeSelectorType& EdgeSelector(int32 Index)
		{
			return EdgeSelectors[Index];
		}
		
		DistanceType Distance() const
		{
			const int32 ContourCount = EdgeSelectors.Num();
			
			EdgeSelectorType ShapeEdgeSelector;
			EdgeSelectorType InnerEdgeSelector;
			EdgeSelectorType OuterEdgeSelector;
			ShapeEdgeSelector.Reset(Point);
			InnerEdgeSelector.Reset(Point);
			OuterEdgeSelector.Reset(Point);
			
			for (int32 Index = 0; Index < ContourCount; ++Index)
			{
				DistanceType EdgeDistance = EdgeSelectors[Index].Distance();
				ShapeEdgeSelector.Merge(EdgeSelectors[Index]);
				
				if (Windings[Index] > 0 && ResolveDistance(EdgeDistance) >= 0)
				{
					InnerEdgeSelector.Merge(EdgeSelectors[Index]);
				}
				
				if (Windings[Index] < 0 && ResolveDistance(EdgeDistance) <= 0)
				{
					OuterEdgeSelector.Merge(EdgeSelectors[Index]);
				}
			}

			DistanceType ShapeDistance = ShapeEdgeSelector.Distance();
			DistanceType InnerDistance = InnerEdgeSelector.Distance();
			DistanceType OuterDistance = OuterEdgeSelector.Distance();

			const double InnerScalarDistance = ResolveDistance(InnerDistance);
			const double OuterScalarDistance = ResolveDistance(OuterDistance);
			
			DistanceType Distance;
			InitDistance(Distance);

			int32 Winding = 0;
			if (InnerScalarDistance >= 0 && fabs(InnerScalarDistance) <= fabs(OuterScalarDistance))
			{
				Distance = InnerDistance;
				Winding = 1;
				for (int32 Index = 0; Index < ContourCount; ++Index)
				{
					if (Windings[Index] > 0)
					{
						DistanceType ContourDistance = EdgeSelectors[Index].Distance();
						if (fabs(ResolveDistance(ContourDistance)) < fabs(OuterScalarDistance) && ResolveDistance(ContourDistance) > ResolveDistance(Distance))
						{
							Distance = ContourDistance;
						}
					}
				}
			}
			else if (OuterScalarDistance <= 0 && fabs(OuterScalarDistance) < fabs(InnerScalarDistance))
			{
				Distance = OuterDistance;
				Winding = -1;
				for (int32 Index = 0; Index < ContourCount; ++Index)
				{
					if (Windings[Index] < 0)
					{
						DistanceType ContourDistance = EdgeSelectors[Index].Distance();
						if (fabs(ResolveDistance(ContourDistance)) < fabs(InnerScalarDistance) && ResolveDistance(ContourDistance) < ResolveDistance(Distance))
						{
							Distance = ContourDistance;
						}
					}
				}
			}
			else
			{
				return ShapeDistance;
			}

			for (int32 Index = 0; Index < ContourCount; ++Index)
			{
				if (Windings[Index] != Winding)
				{
					DistanceType ContourDistance = EdgeSelectors[Index].Distance();
					if (ResolveDistance(ContourDistance) * ResolveDistance(Distance) >= 0 && fabs(ResolveDistance(ContourDistance)) < fabs(ResolveDistance(Distance)))
					{
						Distance = ContourDistance;
					}
				}
			}
			
			if (ResolveDistance(Distance) == ResolveDistance(ShapeDistance))
				Distance = ShapeDistance;
			return Distance;
		}

	private:
		FPoint2D Point;
		TArray<int32> Windings;
		TArray<EdgeSelectorType> EdgeSelectors;
	};
}
