#pragma once

#include "Contour.h"
#include "Rasterization/Scanline.h"

namespace SDFGenerator
{
	// Threshold of the dot product of adjacent edge directions to be considered convergent.
	#define SDF_GENERATOR_CORNER_DOT_EPSILON .000001

	// The proportional amount by which a curve's control point will be adjusted to eliminate convergent corners.
	#define SDF_GENERATOR_DECONVERGENCE_FACTOR .000001

	/// Vector shape representation.
	class FShape
	{
	public:
		struct FBounds
		{
			double Left, Bottom, Right, Top;
		};

		/// The list of contours the shape consists of.
		TArray<FContour> Contours;
		
		/// Specifies whether the shape uses bottom-to-top (false) or top-to-bottom (true) Y coordinates.
		bool bInverseYAxis;

		FShape();
		
		/**
		 * Adds a contour.
		 */
		void AddContour(const FContour& Contour);
 
		void AddContour(FContour&& Contour);

		/**
		 * Adds a blank contour and returns its reference.
		 */
		FContour& AddContour();
		
		/**
		 * Normalizes the shape geometry for distance field generation.
		 */
		void Normalize();
		
		/**
		 * Performs basic checks to determine if the object represents a valid shape.
		 */
		bool Validate() const;
		
		/**
		 * Adjusts the bounding box to fit the shape.
		 */
		void Bound(double& Left, double& Bottom, double& Right, double& Top) const;
		
		/**
		 * Adjusts the bounding box to fit the shape border's mitered corners.
		 */
		void BoundMiters(double& Left, double& Bottom, double& Right, double& Top, double Border, double MiterLimit, int32 Polarity) const;
		
		/**
		 * Computes the minimum bounding box that fits the shape, optionally with a (mitered) border.
		 */
		FBounds GetBounds(double Border = 0, double MiterLimit = 0, int32 Polarity = 0) const;
		
		/**
		 * Outputs the scanline that intersects the shape at y.
		 */
		void Scanline(FScanline& Line, double Y) const;
		
		/**
		 * Returns the total number of edge segments
		 */
		int32 EdgeCount() const;
		
		/**
		 * Assumes its contours are unoriented (even-odd fill rule). Attempts to orient them to conform to the non-zero winding rule.
		 */
		void OrientContours();
	};
}
