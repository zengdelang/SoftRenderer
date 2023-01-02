#pragma once

#include "Vector2.h"
#include "SignedDistance/SignedDistance.h"

namespace SDFGenerator
{
	// Parameters for iterative search of closest point on a cubic Bezier curve. Increase for higher precision.
	#define SDF_GENERATOR_CUBIC_SEARCH_STARTS 4
	#define SDF_GENERATOR_CUBIC_SEARCH_STEPS 4

	/**
	 * Edge color specifies which color channels an edge belongs to.
	 */
	enum class EEdgeColor : uint8
	{
		BLACK = 0,
		RED = 1,
		GREEN = 2,
		YELLOW = 3,
		BLUE = 4,
		MAGENTA = 5,
		CYAN = 6,
		WHITE = 7
	};

	enum class EEdgeSegmentType : uint8
	{
		None,
		LinearSegment,
		QuadraticSegment,
		CubicSegment,
	};

	/**
	 * An abstract edge segment.
	 */
	class FEdgeSegment
	{
	public:
		EEdgeColor Color;

		FEdgeSegment(EEdgeColor EdgeColor = EEdgeColor::WHITE) : Color(EdgeColor)
		{
		}

		virtual ~FEdgeSegment()
		{
		}

		/**
		 * Creates a copy of the edge segment.
		 */
		virtual FEdgeSegment* Clone() const = 0;
		
		/**
		 * Returns the point on the edge specified by the parameter (between 0 and 1).
		 */
		virtual FPoint2D Point(double Param) const = 0;
		
		/**
		 * Returns the direction the edge has at the point specified by the parameter.
		 */
		virtual FVector2 Direction(double Param) const = 0;
		
		/**
		 * Returns the change of direction (second derivative) at the point specified by the parameter.
		 */
		virtual FVector2 DirectionChange(double Param) const = 0;
		
		/**
		 * Returns the minimum signed distance between origin and the edge.
		 */
		virtual FSignedDistance SignedDistance(FPoint2D Origin, double& Param) const = 0;
		
		/**
		 * Converts a previously retrieved signed distance from origin to pseudo-distance.
		 */
		virtual void DistanceToPseudoDistance(FSignedDistance& Distance, FPoint2D Origin, double Param) const;
		
		/**
		 * Outputs a list of (at most three) intersections (their X coordinates) with an infinite horizontal scanline at y and returns how many there are.
		 */
		virtual int32 ScanlineIntersections(double X[3], int32 Dy[3], double Y) const = 0;
		
		/**
		 * Adjusts the bounding box to fit the edge segment.
		 */
		virtual void Bound(double& Left, double& Bottom, double& Right, double& Top) const = 0;

		/**
		 * Reverses the edge (swaps its start point and end point).
		 */
		virtual void Reverse() = 0;
		
		/**
		 * Moves the start point of the edge segment.
		 */
		virtual void MoveStartPoint(FPoint2D To) = 0;
		
		/**
		 * Moves the end point of the edge segment.
		 */
		virtual void MoveEndPoint(FPoint2D To) = 0;
		
		/**
		 * Splits the edge segments into thirds which together represent the original edge.
		 */
		virtual void SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const = 0;

		virtual EEdgeSegmentType GetType() { return EEdgeSegmentType::None; }
	};

	/**
	 * A line segment.
	 */
	class FLinearSegment : public FEdgeSegment
	{
	public:
		FPoint2D Points[2];

		FLinearSegment(FPoint2D P0, FPoint2D P1, EEdgeColor EdgeColor = EEdgeColor::WHITE);

		virtual FLinearSegment* Clone() const override;
		virtual FPoint2D Point(double Param) const override;
		virtual FVector2 Direction(double Param) const override;
		virtual FVector2 DirectionChange(double Param) const override;

		virtual FSignedDistance SignedDistance(FPoint2D Origin, double& Param) const override;
		virtual int32 ScanlineIntersections(double X[3], int32 Dy[3], double Y) const override;
		virtual void Bound(double& Left, double& Bottom, double& Right, double& Top) const override;

		virtual void Reverse() override;
		virtual void MoveStartPoint(FPoint2D To) override;
		virtual void MoveEndPoint(FPoint2D To) override;
		virtual void SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const override;

		virtual EEdgeSegmentType GetType() override { return EEdgeSegmentType::LinearSegment; }

		double Length() const;
	};

	/**
	 * A quadratic Bezier curve.
	 */
	class FQuadraticSegment : public FEdgeSegment
	{
	public:
		FPoint2D Points[3];

		FQuadraticSegment(FPoint2D P0, FPoint2D P1, FPoint2D P2, EEdgeColor EdgeColor = EEdgeColor::WHITE);

		virtual FQuadraticSegment* Clone() const override;
		virtual FPoint2D Point(double Param) const override;
		virtual FVector2 Direction(double Param) const override;
		virtual FVector2 DirectionChange(double Param) const override;

		virtual FSignedDistance SignedDistance(FPoint2D Origin, double& Param) const override;
		virtual int32 ScanlineIntersections(double X[3], int32 Dy[3], double Y) const override;
		virtual void Bound(double& Left, double& Bottom, double& Right, double& Top) const override;

		virtual void Reverse() override;
		virtual void MoveStartPoint(FPoint2D To) override;
		virtual void MoveEndPoint(FPoint2D To) override;
		virtual void SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const override;
		
		virtual EEdgeSegmentType GetType() override { return EEdgeSegmentType::QuadraticSegment; }

		double Length() const;
		FEdgeSegment* ConvertToCubic() const;
	};

	/**
	 * A cubic Bezier curve.
	 */
	class FCubicSegment : public FEdgeSegment
	{
	public:
		FPoint2D Points[4];

		FCubicSegment(FPoint2D P0, FPoint2D P1, FPoint2D P2, FPoint2D P3, EEdgeColor EdgeColor = EEdgeColor::WHITE);

		virtual FCubicSegment* Clone() const override;
		virtual FPoint2D Point(double Param) const override;
		virtual FVector2 Direction(double Param) const override;
		virtual FVector2 DirectionChange(double Param) const override;
		
		virtual FSignedDistance SignedDistance(FPoint2D Origin, double& Param) const override;
		virtual int32 ScanlineIntersections(double X[3], int32 Dy[3], double Y) const override;
		virtual void Bound(double& Left, double& Bottom, double& Right, double& Top) const override;

		virtual void Reverse() override;
		virtual void MoveStartPoint(FPoint2D To) override;
		virtual void MoveEndPoint(FPoint2D To) override;
		virtual void SplitInThirds(FEdgeSegment*& Part1, FEdgeSegment*& Part2, FEdgeSegment*& Part3) const override;

		virtual EEdgeSegmentType GetType() override { return EEdgeSegmentType::CubicSegment; }

		void Deconverge(int32 Param, double Amount);
	};
}
