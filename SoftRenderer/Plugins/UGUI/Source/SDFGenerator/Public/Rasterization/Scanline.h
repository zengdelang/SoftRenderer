#pragma once

namespace SDFGenerator
{
	/// Fill rule dictates how intersection total is interpreted during rasterization.
	enum class EFillRule : uint8
	{
		FILL_NONZERO,
		FILL_ODD,
		// "even-odd"
		FILL_POSITIVE,
		FILL_NEGATIVE
	};

	/// Resolves the number of intersection into a binary fill value based on fill rule.
	bool InterpretFillRule(int32 Intersections, EFillRule FillRule);

	/**
	 * Represents a horizontal scanline intersecting a shape.
	 */
	class FScanline
	{
	public:
		/// An intersection with the scanline.
		struct FIntersection
		{
			/// X coordinate.
			double X;
			/// Normalized Y direction of the oriented edge at the point of intersection.
			int32 Direction;
		};

		static double Overlap(const FScanline& A, const FScanline& B, double xFrom, double xTo, EFillRule FillRule);

		FScanline();
		
		/**
		 * Populates the intersection list.
		 */
		void SetIntersections(const TArray<FIntersection>& InIntersections);

		void SetIntersections(TArray<FIntersection>&& InIntersections);

		/**
		 * Returns the number of intersections left of x.
		 */
		int32 CountIntersections(double X) const;
		
		/**
		 * Returns the total sign of intersections left of x.
		 */
		int32 SumIntersections(double X) const;
		
		/**
		 * Decides whether the scanline is filled at x based on fill rule.
		 */
		bool Filled(double X, EFillRule FillRule) const;

	private:
		TArray<FIntersection> Intersections;
		mutable int32 LastIndex;

		void Preprocess();
		int32 MoveTo(double X) const;
	};
}
