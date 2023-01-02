#pragma once

#include "Vector2.h"

namespace SDFGenerator
{
	/**
	 * A transformation from shape coordinates to pixel coordinates.
	 */
	class FProjection
	{
	public:
		FProjection();
		FProjection(const FVector2& InScale, const FVector2& InTranslate);
		
		/**
		 * Converts the shape coordinate to pixel coordinate.
		 */
		FPoint2D Project(const FPoint2D& Coord) const;
		
		/**
		 * Converts the pixel coordinate to shape coordinate.
		 */
		FPoint2D Unproject(const FPoint2D& Coord) const;
		
		/**
		 * Converts the vector to pixel coordinate space.
		 */
		FVector2 ProjectVector(const FVector2& Vector) const;
		
		/**
		 * Converts the vector from pixel coordinate space.
		 */
		FVector2 UnprojectVector(const FVector2& Vector) const;
		
		/**
		 * Converts the X-coordinate from shape to pixel coordinate space.
		 */
		double ProjectX(double X) const;
		
		/**
		 * Converts the Y-coordinate from shape to pixel coordinate space.
		 */
		double ProjectY(double Y) const;
		
		/**
		 * Converts the X-coordinate from pixel to shape coordinate space.
		 */
		double UnprojectX(double X) const;
		
		/// Converts the Y-coordinate from pixel to shape coordinate space.
		double UnprojectY(double Y) const;

	private:
		FVector2 Scale;
		FVector2 Translate;
	};
}
