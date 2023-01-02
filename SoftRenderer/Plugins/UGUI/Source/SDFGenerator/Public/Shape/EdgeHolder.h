#pragma once

#include "EdgetSegments.h"

namespace SDFGenerator
{
	/**
	 * Container for a single edge of dynamic type.
	 */
	class FEdgeHolder
	{
	public:
		/// Swaps the edges held by a and b.
		static void Swap(FEdgeHolder& A, FEdgeHolder& B);

		FEdgeHolder();
		FEdgeHolder(FEdgeSegment* Segment);
		
		FEdgeHolder(FPoint2D P0, FPoint2D P1, EEdgeColor EdgeColor = EEdgeColor::WHITE);
		FEdgeHolder(FPoint2D P0, FPoint2D P1, FPoint2D P2, EEdgeColor EdgeColor = EEdgeColor::WHITE);
		FEdgeHolder(FPoint2D P0, FPoint2D P1, FPoint2D P2, FPoint2D P3, EEdgeColor EdgeColor = EEdgeColor::WHITE);
		
		FEdgeHolder(const FEdgeHolder& Orig);
		FEdgeHolder(FEdgeHolder&& Orig) noexcept;

		~FEdgeHolder();
		
		FEdgeHolder& operator=(const FEdgeHolder& Orig);
		FEdgeHolder& operator=(FEdgeHolder&& Orig) noexcept;

		FEdgeSegment& operator*();
		const FEdgeSegment& operator*() const;
		
		FEdgeSegment* operator->();
		const FEdgeSegment* operator->() const;

		operator FEdgeSegment*();
		operator const FEdgeSegment*() const;

	private:
		FEdgeSegment* EdgeSegment;
	};
}
