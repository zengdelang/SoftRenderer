#pragma once

#include "EdgeHolder.h"

namespace SDFGenerator
{
	/**
	 * A single closed contour of a shape.
	 */
	class FContour
	{
	public:
		/// The sequence of edges that make up the contour.
		TArray<FEdgeHolder> Edges;

		/**
		 * Adds an edge to the contour.
		 */
		void AddEdge(const FEdgeHolder& Edge);

		void AddEdge(FEdgeHolder&& Edge);

		/**
		 * Creates a new edge in the contour and returns its reference.
		 */
		FEdgeHolder& AddEdge();
		
		/**
		 * Adjusts the bounding box to fit the contour.
		 */
		void Bound(double& Left, double& Bottom, double& Right, double& Top) const;
		
		/**
		 * Adjusts the bounding box to fit the contour border's mitered corners.
		 */
		void BoundMiters(double& Left, double& Bottom, double& Right, double& Top, double Border, double MiterLimit, int32 Polarity) const;
		
		/**
		 * Computes the winding of the contour. Returns 1 if positive, -1 if negative.
		 */
		int32 Winding() const;
		
		/**
		 * Reverses the sequence of edges on the contour.
		 */
		void Reverse();
	};
}
