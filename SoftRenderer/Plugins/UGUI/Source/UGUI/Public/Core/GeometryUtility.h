#pragma once

#include "CoreMinimal.h"
#include "Core/Render/VertexHelper.h"

UENUM()
enum class EPointMode : uint8
{
	None = 0 UMETA(Tooltip = "Default : Do not render the point alone"),
	Square = 1 UMETA(Tooltip = "Render the point as a square"),
	Triangle = 2 UMETA(Tooltip = "Render the point as a triangle"),
	Circle = 3 UMETA(Tooltip = "Render the point as a circle"),
	Diamond = 4 UMETA(Tooltip = "Render the point as a diamond"),
};

class UGUI_API FGeometryUtility
{
public:
	static void GenerateCap(FVertexHelper& VertexHelper, const FVector2D& CapOrigin, const FVector2D& CapDirection, const FVector2D& Up, const FLinearColor& Color, const FVector2D& UV1, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1,1));

	/**
	* Calculates the intersection of two line segments P1->P2, P3->P4
	* The tolerance setting is used when the lines aren't currently intersecting but will intersect in the future
	* The higher the tolerance the greater the distance that the intersection point can be.
	*
	* @return true if the line intersects.  Populates Intersection
	*/
	static bool LineIntersect(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, FVector2D& Intersect, float Tolerance = .1f);
	
	static void GenerateLines(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClosedLine, FVector2D UV1, float Thickness, const FLinearColor& Color, const TArray<FVector2D>& Points, float Tolerance = .1f, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D::ZeroVector);

	static void GeneratePolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));
	
	static void GenerateHollowPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, const float Thickness, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));
	
	static void GenerateTriangle(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));
	
	static void GenerateSquare(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GenerateDiamond(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GenerateCircle(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, const int32 EdgeNum, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GenerateCircleRings(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, const int32 EdgeNum, float Size, float Thickness, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GeneratePercentPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClockwise, const float Percentage, const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GeneratePercentHollowPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClockwise, const float Percentage, const float Thickness, const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));
		
	static void GeneratePoints(FVertexHelper& VertexHelper, bool bAntiAliasing, FVector2D UV1, float PointSize, const FLinearColor PointColor, EPointMode PointMode, const TArray<FVector2D>& Points, FVector2D TopLeftUV = FVector2D::ZeroVector, FVector2D BottomRightUV = FVector2D(1, 1));

	static void GenerateCurve(FVertexHelper& VertexHelper, bool bAntiAliasing, FVector2D UV1, float Thickness, const FLinearColor& Color, const UCurveFloat* Curve);

	static void BuildBezierGeometry(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, TArray<FVector2D>& Points);
	
protected:
	static void Subdivide(const FVector2D&  P0, const FVector2D&  P1, const FVector2D&  P2, const FVector2D&  P3, TArray<FVector2D>& Points, float MaxBiasTimesTwo = 2.0f);
	static float ComputeCurviness(const FVector2D&  P0, const FVector2D&  P1, const FVector2D&  P2, const FVector2D&  P3);
	static void DeCasteljauSplit(const FVector2D&  P0, const FVector2D&  P1, const FVector2D&  P2, const FVector2D& P3, FVector2D OutCurveParams[7]);
	
};
