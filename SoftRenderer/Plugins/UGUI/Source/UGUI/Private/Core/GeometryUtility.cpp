#include "Core/GeometryUtility.h"

void FGeometryUtility::GenerateCap(FVertexHelper& VertexHelper, const FVector2D& CapOrigin,
                                   const FVector2D& CapDirection, const FVector2D& Up, const FLinearColor& Color,
                                   const FVector2D& UV1, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	const int32 FirstVertIndex = VertexHelper.GetCurrentVerticesCount();

	const FVector2D CenterUV = FVector2D((TopLeftUV.X + BottomRightUV.X) * 0.5, TopLeftUV.Y);
	FVector2D UpUV = TopLeftUV;
	FVector2D DownUV = FVector2D(BottomRightUV.X, TopLeftUV.Y);

	VertexHelper.AddVert(FVector(CapOrigin, 0), Color, CenterUV, UV1, FVector2D::ZeroVector);
	VertexHelper.AddVert(FVector(CapOrigin + CapDirection + Up, 0), Color, UpUV, UV1,
	                     FVector2D(1, 1));
	VertexHelper.AddVert(FVector(CapOrigin + CapDirection - Up, 0), Color, DownUV, UV1,
	                     FVector2D(1, 1));
	VertexHelper.AddVert(FVector(CapOrigin + Up, 0), Color, UpUV, UV1, FVector2D(1, 0));
	VertexHelper.AddVert(FVector(CapOrigin - Up, 0), Color, DownUV, UV1, FVector2D(1,0));

	VertexHelper.AddTriangle(FirstVertIndex + 0, FirstVertIndex + 3, FirstVertIndex + 1);
	VertexHelper.AddTriangle(FirstVertIndex + 0, FirstVertIndex + 1, FirstVertIndex + 2);
	VertexHelper.AddTriangle(FirstVertIndex + 0, FirstVertIndex + 2, FirstVertIndex + 4);
}

/**
* Calculates the intersection of two line segments P1->P2, P3->P4
* The tolerance setting is used when the lines aren't currently intersecting but will intersect in the future
* The higher the tolerance the greater the distance that the intersection point can be.
*
* @return true if the line intersects.  Populates Intersection
*/
bool FGeometryUtility::LineIntersect(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4,
                                     FVector2D& Intersect, float Tolerance)
{
	const float NumA = (P4.X - P3.X) * (P1.Y - P3.Y) - (P4.Y - P3.Y) * (P1.X - P3.X);
	const float NumB = (P2.X - P1.X) * (P1.Y - P3.Y) - (P2.Y - P1.Y) * (P1.X - P3.X);

	const float Denom = (P4.Y - P3.Y) * (P2.X - P1.X) - (P4.X - P3.X) * (P2.Y - P1.Y);

	if (FMath::IsNearlyZero(NumA) && FMath::IsNearlyZero(NumB))
	{
		// Lines are the same
		Intersect = (P1 + P2) / 2;
		return true;
	}

	if (FMath::IsNearlyZero(Denom))
	{
		// Lines are parallel
		return false;
	}

	const float B = NumB / Denom;
	const float A = NumA / Denom;

	// Note that this is a "tweaked" intersection test for the purpose of joining line segments.  We don't just want to know if the line segments
	// Intersect, but where they would if they don't currently. Except that we don't care in the case that where the segments 
	// intersection is so far away that its infeasible to use the intersection point later.
	if (A >= -Tolerance && A <= (1.0f + Tolerance) && B >= -Tolerance && B <= (1.0f + Tolerance))
	{
		Intersect = P1 + A * (P2 - P1);
		return true;
	}

	return false;
}

void FGeometryUtility::GenerateLines(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClosedLine,
	FVector2D UV1, float Thickness, const FLinearColor& Color, const TArray<FVector2D>& Points, float Tolerance, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (Points.Num() < 2)
	{
		return;
	}

	if (bClosedLine)
	{
		bClosedLine = Points.Num() > 2;
	}

	
	FVector2D UpUV = TopLeftUV;
	FVector2D DownUV = FVector2D(BottomRightUV.X, TopLeftUV.Y);

	const int32 NumPoints = Points.Num();
	if (bAntiAliasing)
	{
		//
		//  The true center of the line is represented by o---o---o
		//
		//
		//           Two triangles make up each trapezoidal line segment
		//                /        |  |   
		//               v         |  |   
		//    +-+---------------+  |  | 
		//    |\|              / \ v  | 
		//    | o-------------o   \   |  +--------- U==0
		//    |/|            / \   \  |  | 
		//    +-+-----------+   \   \ v  v  
		//                   \   \   +------+-+
		//     ^              \   \ /       |/| 
		//     |               \   o--------o | <-- Endcap
		//     Endcap           \ /         |\|
		//                       +----------+-+
		//                               ^
		//                               |
		//                               +--------- U==1
		//
		// Each trapezoidal section has a Vertex.U==1 on the bottom and Vertex.U==0 on top.
		// Endcaps have Vertex.U==0.5 in the middle and Vertex.U==0 on the outside.
		// This enables easy distance calculations to the "true center" of the line for
		// anti-aliasing calculations performed in the pixels shader.

		static const float TwoRootTwo = 2 * FMath::Sqrt(2);
		// Compute the actual size of the line we need based on thickness.
		// Each line segment will be a bit thicker than the line to account
		// for the size of the filter.
		const float LineThickness = (TwoRootTwo + Thickness);

		// The amount we increase each side of the line to generate enough pixels
		const float HalfThickness = LineThickness * .5f;
		
		if (!bClosedLine)
		{
			VertexHelper.Reserve(VertexHelper.GetCurrentVerticesCount() + 2 * (NumPoints - 1) + 12,
							VertexHelper.GetCurrentIndexCount() + 6 * (NumPoints - 1) + 18);
			
			// 1. Generate the StartCap
			FVector2D StartPos = Points[0];
			FVector2D EndPos = Points[1];
			FVector2D Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();
			FVector2D Up = Normal * HalfThickness;

			const FVector2D StartCapDirection = HalfThickness * ((StartPos - EndPos).GetSafeNormal());
			GenerateCap(VertexHelper, StartPos, StartCapDirection, Up, Color, UV1, TopLeftUV, BottomRightUV);

			const int32 IndexStart = VertexHelper.GetCurrentVerticesCount();

			// First two points in the line.
			VertexHelper.AddVert(FVector(StartPos + Up, 0), Color, UpUV, UV1, FVector2D(1.0, 0));
			VertexHelper.AddVert(FVector(StartPos - Up, 0), Color, DownUV, UV1, FVector2D(-1.0, 0));

			// 2. Generate the rest of the segments
			for (int32 Point = 1; Point < NumPoints; ++Point)
			{
				EndPos = Points[Point];
				// Determine if we should check the intersection point with the next line segment.
				// We will adjust were this line ends to the intersection
				const bool bCheckIntersection = (Point + 1) < NumPoints;

				// Compute the normal to the line
				Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

				// Create the new vertices for the thick line segment
				Up = Normal * HalfThickness;

				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;

				if (bCheckIntersection)
				{
					// The end point of the next segment
					const FVector2D NextEndPos = Points[Point + 1];

					// The normal of the next segment
					const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
						GetSafeNormal();

					// The next amount to adjust the vertices by 
					FVector2D NextUp = NextNormal * HalfThickness;

					FVector2D IntersectionPoint;
					if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp,
					                  IntersectionPoint, Tolerance))
					{
						// If the lines intersect adjust where the line starts
						IntersectUpper = IntersectionPoint;
					}

					if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp,
					                  IntersectionPoint, Tolerance))
					{
						// If the lines intersect adjust where the line starts
						IntersectLower = IntersectionPoint;
					}
				}

				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1, FVector2D(1.0, 0));
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1, FVector2D(-1.0, 0));

				// Counterclockwise winding on triangles
				VertexHelper.AddTriangle(IndexStart + 2 * Point - 1, IndexStart + 2 * Point - 2,
				                         IndexStart + 2 * Point + 0);
				VertexHelper.AddTriangle(IndexStart + 2 * Point - 1, IndexStart + 2 * Point + 0,
				                         IndexStart + 2 * Point + 1);

				StartPos = EndPos;
			}

			// 3. Generate the EndCap
			EndPos = Points[NumPoints - 1];
			StartPos = Points[NumPoints - 2];

			const FVector2D EndCapDirection = HalfThickness * ((EndPos - StartPos).GetSafeNormal());
			GenerateCap(VertexHelper, EndPos, EndCapDirection, Up, Color, UV1, TopLeftUV, BottomRightUV);
		}
		else
		{
			VertexHelper.Reserve(VertexHelper.GetCurrentVerticesCount() + 2 * NumPoints,
							VertexHelper.GetCurrentIndexCount() + 6 * NumPoints);
			
			const int32 IndexStart = VertexHelper.GetCurrentVerticesCount();

			FVector2D StartPos = Points[Points.Num() - 1];
			FVector2D EndPos = Points[0];

			// Compute the normal to the line
			FVector2D Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

			// Create the new vertices for the thick line segment
			FVector2D Up = Normal * HalfThickness;

			{
				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;
			
				// The end point of the next segment
				const FVector2D NextEndPos = Points[1];
			
				// The normal of the next segment
				const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
					GetSafeNormal();
			
				// The next amount to adjust the vertices by 
				FVector2D NextUp = NextNormal * HalfThickness;
			
				FVector2D IntersectionPoint;
				if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp, IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectUpper = IntersectionPoint;
				}
				
				if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp, IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectLower = IntersectionPoint;
				}
			
				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1, FVector2D(1.0, 0));
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1, FVector2D(-1, 0));
			}
			
			for (int32 Point = 0; Point < NumPoints - 1; ++Point)
			{
				StartPos = Points[Point];
				EndPos = Points[Point + 1];

				// Compute the normal to the line
				Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

				// Create the new vertices for the thick line segment
				Up = Normal * HalfThickness;

				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;

				// The end point of the next segment
				const FVector2D NextEndPos = (Point == NumPoints - 2 ? Points[0] : Points[Point + 2]);

				// The normal of the next segment
				const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
					GetSafeNormal();

				// The next amount to adjust the vertices by 
				FVector2D NextUp = NextNormal * HalfThickness;

				FVector2D IntersectionPoint;
				if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp,
				                  IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectUpper = IntersectionPoint;
				}

				if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp,
				                  IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectLower = IntersectionPoint;
				}

				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1, FVector2D(1.0, 0));
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1, FVector2D(-1, 0));

				// Counterclockwise winding on triangles
				VertexHelper.AddTriangle(IndexStart + 2 * Point + 1, IndexStart + 2 * Point + 0,
				                         IndexStart + 2 * Point + 2);
				VertexHelper.AddTriangle(IndexStart + 2 * Point + 1, IndexStart + 2 * Point + 2,
				                         IndexStart + 2 * Point + 3);
			}

			// Counterclockwise winding on triangles
			VertexHelper.AddTriangle(IndexStart + 2 * (NumPoints - 1) + 1, IndexStart + 2 * (NumPoints - 1) + 0,
			                         IndexStart + 0);
			VertexHelper.AddTriangle(IndexStart + 2 * (NumPoints - 1) + 1, IndexStart + 0, IndexStart + 1);
		}
	}
	else
	{
		// Generate the line segments using non-aa'ed polylines.
		const float HalfThickness = Thickness * 0.5f;

		if (!bClosedLine)
		{
			VertexHelper.Reserve(VertexHelper.GetCurrentVerticesCount() + 4 * (NumPoints - 2) + 2,
							VertexHelper.GetCurrentIndexCount() + 6 * (NumPoints - 1));
			
			FVector2D StartPos = Points[0];
			FVector2D EndPos = Points[1];

			FVector2D Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();
			FVector2D Up = Normal * HalfThickness;

			const int32 IndexStart = VertexHelper.GetCurrentVerticesCount();

			// First two points in the line.
			VertexHelper.AddVert(FVector(StartPos + Up, 0), Color, UpUV, UV1, FVector2D::ZeroVector);
			VertexHelper.AddVert(FVector(StartPos - Up, 0), Color, DownUV, UV1, FVector2D::ZeroVector);

			// Generate the rest of the segments
			for (int32 Point = 1; Point < NumPoints; ++Point)
			{
				EndPos = Points[Point];
				// Determine if we should check the intersection point with the next line segment.
				// We will adjust were this line ends to the intersection
				const bool bCheckIntersection = (Point + 1) < NumPoints;

				// Compute the normal to the line
				Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

				// Create the new vertices for the thick line segment
				Up = Normal * HalfThickness;

				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;

				if (bCheckIntersection)
				{
					// The end point of the next segment
					const FVector2D NextEndPos = Points[Point + 1];

					// The normal of the next segment
					const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
						GetSafeNormal();

					// The next amount to adjust the vertices by 
					FVector2D NextUp = NextNormal * HalfThickness;

					FVector2D IntersectionPoint;
					if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp,
					                  IntersectionPoint, Tolerance))
					{
						// If the lines intersect adjust where the line starts
						IntersectUpper = IntersectionPoint;
					}

					if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp,
					                  IntersectionPoint, Tolerance))
					{
						// If the lines intersect adjust where the line starts
						IntersectLower = IntersectionPoint;
					}
				}

				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1,
				                     FVector2D::ZeroVector);
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1,
				                     FVector2D::ZeroVector);

				// Counterclockwise winding on triangles
				VertexHelper.AddTriangle(IndexStart + 2 * Point - 1, IndexStart + 2 * Point - 2,
				                         IndexStart + 2 * Point + 0);
				VertexHelper.AddTriangle(IndexStart + 2 * Point - 1, IndexStart + 2 * Point + 0,
				                         IndexStart + 2 * Point + 1);

				StartPos = EndPos;
			}
		}
		else
		{
			const int32 IndexStart = VertexHelper.GetCurrentVerticesCount();

			VertexHelper.Reserve(IndexStart + 4 * (NumPoints - 1),
				VertexHelper.GetCurrentIndexCount() + 6 * (NumPoints + 1));

			FVector2D StartPos = Points[Points.Num() - 1];
			FVector2D EndPos = Points[0];

			// Compute the normal to the line
			FVector2D Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

			// Create the new vertices for the thick line segment
			FVector2D Up = Normal * HalfThickness;

			{
				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;

				// The end point of the next segment
				const FVector2D NextEndPos = Points[1];

				// The normal of the next segment
				const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
					GetSafeNormal();

				// The next amount to adjust the vertices by 
				FVector2D NextUp = NextNormal * HalfThickness;

				FVector2D IntersectionPoint;
				if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp, IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectUpper = IntersectionPoint;
				}

				if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp, IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectLower = IntersectionPoint;
				}

				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1,
				                     FVector2D::ZeroVector);
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1,
				                     FVector2D::ZeroVector);
			}

			// Generate the rest of the segments
			for (int32 Point = 0; Point < NumPoints; ++Point)
			{
				EndPos = Points[Point];

				// Compute the normal to the line
				Normal = FVector2D(StartPos.Y - EndPos.Y, EndPos.X - StartPos.X).GetSafeNormal();

				// Create the new vertices for the thick line segment
				Up = Normal * HalfThickness;

				FVector2D IntersectUpper = EndPos + Up;
				FVector2D IntersectLower = EndPos - Up;

				// The end point of the next segment
				const FVector2D NextEndPos = (Point == NumPoints - 1 ? Points[0] : Points[Point + 1]);

				// The normal of the next segment
				const FVector2D NextNormal = FVector2D(EndPos.Y - NextEndPos.Y, NextEndPos.X - EndPos.X).
					GetSafeNormal();

				// The next amount to adjust the vertices by 
				FVector2D NextUp = NextNormal * HalfThickness;

				FVector2D IntersectionPoint;
				if (LineIntersect(StartPos + Up, EndPos + Up, EndPos + NextUp, NextEndPos + NextUp,
				                  IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectUpper = IntersectionPoint;
				}

				if (LineIntersect(StartPos - Up, EndPos - Up, EndPos - NextUp, NextEndPos - NextUp,
				                  IntersectionPoint, Tolerance))
				{
					// If the lines intersect adjust where the line starts
					IntersectLower = IntersectionPoint;
				}

				VertexHelper.AddVert(FVector(IntersectUpper, 0), Color, UpUV, UV1,
				                     FVector2D::ZeroVector);
				VertexHelper.AddVert(FVector(IntersectLower, 0), Color, DownUV, UV1,
				                     FVector2D::ZeroVector);

				// Counterclockwise winding on triangles
				VertexHelper.AddTriangle(IndexStart + 2 * Point + 1, IndexStart + 2 * Point + 0,
				                         IndexStart + 2 * Point + 2);
				VertexHelper.AddTriangle(IndexStart + 2 * Point + 1, IndexStart + 2 * Point + 2,
				                         IndexStart + 2 * Point + 3);

				StartPos = EndPos;
			}

			// Counterclockwise winding on triangles
			VertexHelper.AddTriangle(IndexStart + 2 * (NumPoints - 1) + 3, IndexStart + 2 * (NumPoints - 1) + 2,
			                         IndexStart + 0);
			VertexHelper.AddTriangle(IndexStart + 2 * (NumPoints - 1) + 3, IndexStart + 0, IndexStart + 1);
		}
	}
}

void FGeometryUtility::GeneratePolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos,
	const int32 EdgeNum, const float StartRadian, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (EdgeNum < 3)
		return;

	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
	float CurRadian = StartRadian;

	VertexHelper.Reserve(StartIndex + EdgeNum + 1, VertexHelper.GetCurrentIndexCount() + 3 * EdgeNum);

	const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
	const float UVDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
	const float UVDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
	if (bAntiAliasing)
	{
		VertexHelper.AddVert(FVector(CenterPointPos.X, CenterPointPos.Y, 0 ), Color, CenterUV, UV1, FVector2D::ZeroVector);
		for (int32 Index = 0; Index < EdgeNum; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosY = CenterPointPos.Y + PolygonEdgeSize * SinY;
			VertexHelper.AddVert(FVector(NowPointPosX, NowPointPosY, 0), Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1, FVector2D(1, 0));
			CurRadian += 2 * PI / EdgeNum;
		}
	}
	else
	{
		VertexHelper.AddVert(FVector(CenterPointPos.X, CenterPointPos.Y, 0), Color, CenterUV, UV1);
		for (int32 Index = 0; Index < EdgeNum; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosY = CenterPointPos.Y + PolygonEdgeSize * SinY;
			VertexHelper.AddVert({ NowPointPosX, NowPointPosY, 0 }, Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1);
			CurRadian += 2 * PI / EdgeNum;
		}
	}

	for (int32 Index = 1; Index < EdgeNum; Index++)
	{
		VertexHelper.AddTriangle(0 + StartIndex, Index + StartIndex, Index + 1 + StartIndex);
	}
	VertexHelper.AddTriangle(0 + StartIndex, EdgeNum + StartIndex, 1 + StartIndex);
}

void FGeometryUtility::GenerateHollowPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing,
	const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, const float Thickness, FVector2D UV1,
	float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (EdgeNum < 3)
		return;

	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
	float CurRadian = StartRadian;

	VertexHelper.Reserve(StartIndex + EdgeNum * 2 + 1, VertexHelper.GetCurrentIndexCount() + 3 * EdgeNum * 2);

	const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
	const float UVOutDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
	const float UVOutDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
	const float UVInDeltaX = (BottomRightUV.X - TopLeftUV.X) * (0.5 - Thickness/2);
	const float UVInDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * (0.5 - Thickness/2);
	if (bAntiAliasing)
	{

		for (int32 Index = 0; Index < EdgeNum; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosXOut = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosYOut = CenterPointPos.Y + PolygonEdgeSize * SinY;
			const float NowPointPosXIn = CenterPointPos.X + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * CosX;
			const float NowPointPosYIn = CenterPointPos.Y + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * SinY;
			VertexHelper.AddVert(FVector(NowPointPosXOut, NowPointPosYOut, 0), Color, CenterUV + FVector2D(UVOutDeltaX * CosX, -UVOutDeltaY * SinY), UV1, FVector2D(1, 0));
			VertexHelper.AddVert(FVector(NowPointPosXIn, NowPointPosYIn, 0), Color, CenterUV + FVector2D(UVInDeltaX * CosX, -UVInDeltaY * SinY), UV1, FVector2D(1, 0));
			CurRadian += 2 * PI / EdgeNum;
		}
	}
	else
	{
		for (int32 Index = 0; Index < EdgeNum; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosXOut = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosYOut = CenterPointPos.Y + PolygonEdgeSize * SinY;
			const float NowPointPosXIn = CenterPointPos.X + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * CosX;
			const float NowPointPosYIn = CenterPointPos.Y + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * SinY;

			VertexHelper.AddVert(FVector(NowPointPosXOut, NowPointPosYOut, 0), Color, CenterUV + FVector2D(UVOutDeltaX * CosX, -UVOutDeltaY * SinY), UV1);
			VertexHelper.AddVert(FVector(NowPointPosXIn, NowPointPosYIn, 0), Color, CenterUV + FVector2D(UVInDeltaX * CosX, -UVInDeltaY * SinY), UV1);
			CurRadian += 2 * PI / EdgeNum;
		}
	}

	for (int32 Index = 0; Index < (EdgeNum - 1); Index++)
	{
		VertexHelper.AddTriangle((0 + Index * 2) + StartIndex, (2 + Index * 2) + StartIndex, (1 + Index * 2) + StartIndex);
		VertexHelper.AddTriangle((2 + Index * 2) + StartIndex, (3 + Index * 2) + StartIndex, (1 + Index * 2) + StartIndex);
	}
	VertexHelper.AddTriangle(2 * (EdgeNum - 1) + StartIndex, 0 + StartIndex, 2 * (EdgeNum - 1) + 1 + StartIndex);
	VertexHelper.AddTriangle(0 + StartIndex, 1 + StartIndex, 2 * (EdgeNum - 1) + 1 + StartIndex);
}

void FGeometryUtility::GenerateTriangle(FVertexHelper& VertexHelper, bool bAntiAliasing,
                                        const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (bAntiAliasing)
	{
		GeneratePolygon(VertexHelper, bAntiAliasing, CenterPointPos, 3, PI / 2, UV1, Size, Color, TopLeftUV, BottomRightUV);
	}
	else
	{
		const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
		VertexHelper.Reserve(StartIndex + 3, VertexHelper.GetCurrentIndexCount() + 3);

		const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
		const float UVDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
		const float UVDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
		
		float CurRadian = PI / 2;
		for (int32 Index = 0; Index < 3; Index++)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + Size * CosX;
			const float NowPointPosY = CenterPointPos.Y + Size * SinY;
			VertexHelper.AddVert(FVector(NowPointPosX, NowPointPosY, 0), Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1);
			CurRadian += 2 * PI / 3;
		}
		
		VertexHelper.AddTriangle(0 + StartIndex, 1 + StartIndex, 2 + StartIndex);
	}
}

void FGeometryUtility::GenerateSquare(FVertexHelper& VertexHelper, bool bAntiAliasing,
	const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (bAntiAliasing)
	{
		GeneratePolygon(VertexHelper, bAntiAliasing, CenterPointPos, 4, PI / 4, UV1, Size, Color, TopLeftUV, BottomRightUV);
	}
	else
	{
		const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
		VertexHelper.Reserve(StartIndex + 4, VertexHelper.GetCurrentIndexCount() + 6);

		const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
		const float UVDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
		const float UVDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
		
		float CurRadian = PI / 4;
		for (int32 Index = 0; Index < 4; Index++)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + Size * CosX;
			const float NowPointPosY = CenterPointPos.Y + Size * SinY;
			VertexHelper.AddVert(FVector(NowPointPosX, NowPointPosY, 0), Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1);
			CurRadian += 2 * PI / 4;
		}
		
		VertexHelper.AddTriangle(0 + StartIndex, 1 + StartIndex, 2 + StartIndex);
		VertexHelper.AddTriangle(2 + StartIndex, 3 + StartIndex, 0 + StartIndex);
	}
}

void FGeometryUtility::GenerateDiamond(FVertexHelper& VertexHelper, bool bAntiAliasing,
	const FVector2D CenterPointPos, float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	GeneratePolygon(VertexHelper, bAntiAliasing, CenterPointPos, 4, 0, UV1, Size, Color, TopLeftUV, BottomRightUV);
}

void FGeometryUtility::GenerateCircle(FVertexHelper& VertexHelper, bool bAntiAliasing, const FVector2D CenterPointPos, const int32 EdgeNum, 
	float Size, FVector2D UV1, const FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (bAntiAliasing)
	{
		const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
		
		VertexHelper.Reserve(StartIndex + 4, VertexHelper.GetCurrentIndexCount() + 6);
		
		float CurRadian = PI / 4;
		TArray<FVector2D, TInlineAllocator<4>> PointsUV0 = { FVector2D(BottomRightUV.X, TopLeftUV.Y), TopLeftUV, FVector2D(TopLeftUV.X, BottomRightUV.Y), BottomRightUV};
		TArray<FVector2D, TInlineAllocator<4>> PointsUV4 = { FVector2D(1.0f, 0), FVector2D(0, 0), FVector2D(0, 1.0f), FVector2D(1.0f, 1.0f) };
		for (int32 Index = 0; Index < 4; Index++)
		{
			const float NowPointPosX = CenterPointPos.X + Size * FMath::Cos(CurRadian) * FMath::Sqrt(2.0);
			const float NowPointPosY = CenterPointPos.Y + Size * FMath::Sin(CurRadian) * FMath::Sqrt(2.0);
			VertexHelper.AddVert({ NowPointPosX, NowPointPosY, 0 }, Color, PointsUV0[Index], UV1, FVector2D::ZeroVector, PointsUV4[Index], FVector2D(1, 0));
			CurRadian += 2 * PI / 4;
		}

		VertexHelper.AddTriangle(0 + StartIndex, 1 + StartIndex, 2 + StartIndex);
		VertexHelper.AddTriangle(2 + StartIndex, 3 + StartIndex, 0 + StartIndex);
	}
	else
	{
		GeneratePolygon(VertexHelper, bAntiAliasing, CenterPointPos, EdgeNum, 0, UV1, Size, Color, TopLeftUV, BottomRightUV);
	}
}

void FGeometryUtility::GenerateCircleRings(FVertexHelper& VertexHelper, bool bAntiAliasing,
	const FVector2D CenterPointPos, const int32 EdgeNum, float Size, float Thickness, FVector2D UV1,
	const FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	if (bAntiAliasing)
	{
		const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();

		VertexHelper.Reserve(StartIndex + 4, VertexHelper.GetCurrentIndexCount() + 6);

		float CurRadian = PI / 4;
		float r = (1 - Thickness)/2;
		TArray<FVector2D, TInlineAllocator<4>> PointsUV0 = { FVector2D(BottomRightUV.X, TopLeftUV.Y), TopLeftUV, FVector2D(TopLeftUV.X, BottomRightUV.Y), BottomRightUV };
		TArray<FVector2D, TInlineAllocator<4>> PointsUV4 = { FVector2D(1.0f, 0), FVector2D(0, 0), FVector2D(0, 1.0f), FVector2D(1.0f, 1.0f) };
		for (int32 Index = 0; Index < 4; Index++)
		{
			const float NowPointPosXOut = CenterPointPos.X + Size * FMath::Cos(CurRadian) * FMath::Sqrt(2.0);
			const float NowPointPosYOut = CenterPointPos.Y + Size * FMath::Sin(CurRadian) * FMath::Sqrt(2.0);
			VertexHelper.AddVert({ NowPointPosXOut, NowPointPosYOut, 0 }, Color, PointsUV0[Index], UV1, FVector2D(1, 0), PointsUV4[Index], FVector2D(1, r));
			CurRadian += 2 * PI / 4;
		}

		VertexHelper.AddTriangle(0 + StartIndex, 1 + StartIndex, 2 + StartIndex);
		VertexHelper.AddTriangle(2 + StartIndex, 3 + StartIndex, 0 + StartIndex);
	}
	else
	{
		GenerateHollowPolygon(VertexHelper, bAntiAliasing, CenterPointPos, EdgeNum, 0, Thickness, UV1, Size, Color, TopLeftUV, BottomRightUV);
	}
}

void FGeometryUtility::GeneratePercentPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClockwise, const float Percentage,
	const FVector2D CenterPointPos, const int32 EdgeNum, const float StartRadian, FVector2D UV1, float PolygonEdgeSize,
	FLinearColor Color, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	const int CurSegements = FMath::CeilToInt(EdgeNum * Percentage);
	if (CurSegements < 1)
		return;

	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
	float CurRadian = StartRadian;
	
	float DegreeDelta = 2 * PI * Percentage / CurSegements;
	DegreeDelta = bClockwise ? -1 * DegreeDelta : DegreeDelta;
	
	VertexHelper.Reserve(StartIndex + CurSegements + 2, VertexHelper.GetCurrentIndexCount() + 3 * CurSegements);

	const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
	const float UVDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
	const float UVDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
	if (bAntiAliasing)
	{
		VertexHelper.AddVert(FVector(CenterPointPos.X, CenterPointPos.Y, 0 ), Color, CenterUV, UV1, FVector2D::ZeroVector);
		for (int32 Index = 0; Index <= CurSegements; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosY = CenterPointPos.Y + PolygonEdgeSize * SinY;
			VertexHelper.AddVert(FVector(NowPointPosX, NowPointPosY, 0), Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1, FVector2D(1, 0));
			CurRadian += DegreeDelta;
		}
	}
	else
	{
		VertexHelper.AddVert(FVector(CenterPointPos.X, CenterPointPos.Y, 0), Color, CenterUV, UV1);
		for (int32 Index = 0; Index <= CurSegements; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosX = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosY = CenterPointPos.Y + PolygonEdgeSize * SinY;
			VertexHelper.AddVert({ NowPointPosX, NowPointPosY, 0 }, Color, CenterUV + FVector2D(UVDeltaX * CosX, -UVDeltaY * SinY), UV1);
			CurRadian += DegreeDelta;
		}
	}

	for (int32 Index = 0; Index < CurSegements; Index++)
	{
		VertexHelper.AddTriangle(StartIndex + 0, StartIndex + Index + 1, StartIndex + Index + 2);
	}
}

void FGeometryUtility::GeneratePercentHollowPolygon(FVertexHelper& VertexHelper, bool bAntiAliasing, bool bClockwise,
	const float Percentage, const float Thickness, const FVector2D CenterPointPos, const int32 EdgeNum,
	const float StartRadian, FVector2D UV1, float PolygonEdgeSize, FLinearColor Color, FVector2D TopLeftUV,
	FVector2D BottomRightUV)
{

	const int CurSegements = FMath::CeilToInt(EdgeNum * Percentage);
	if (CurSegements < 1)
		return;

	const int32 StartIndex = VertexHelper.GetCurrentVerticesCount();
	float CurRadian = StartRadian;
	
	float DegreeDelta = 2 * PI * Percentage / CurSegements;
	DegreeDelta = bClockwise ? -1 * DegreeDelta : DegreeDelta;

	VertexHelper.Reserve(StartIndex + (CurSegements + 1) * 2, VertexHelper.GetCurrentIndexCount() + 3 * CurSegements * 2);

	const FVector2D CenterUV = (TopLeftUV + BottomRightUV) * 0.5;
	const float UVOutDeltaX = (BottomRightUV.X - TopLeftUV.X) * 0.5;
	const float UVOutDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * 0.5;
	const float UVInDeltaX = (BottomRightUV.X - TopLeftUV.X) * (0.5 - Thickness/2);
	const float UVInDeltaY = (BottomRightUV.Y - TopLeftUV.Y) * (0.5 - Thickness/2);
	if (bAntiAliasing)
	{

		for (int32 Index = 0; Index <= CurSegements; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosXOut = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosYOut = CenterPointPos.Y + PolygonEdgeSize * SinY;
			const float NowPointPosXIn = CenterPointPos.X + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * CosX;
			const float NowPointPosYIn = CenterPointPos.Y + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * SinY;
			VertexHelper.AddVert(FVector(NowPointPosXOut, NowPointPosYOut, 0), Color, CenterUV + FVector2D(UVOutDeltaX * CosX, -UVOutDeltaY * SinY), UV1, FVector2D(1, 0));
			VertexHelper.AddVert(FVector(NowPointPosXIn, NowPointPosYIn, 0), Color, CenterUV + FVector2D(UVInDeltaX * CosX, -UVInDeltaY * SinY), UV1, FVector2D(1, 0));
			CurRadian += DegreeDelta;
		}
	}
	else
	{
		for (int32 Index = 0; Index <= CurSegements; ++Index)
		{
			const float CosX = FMath::Cos(CurRadian);
			const float SinY = FMath::Sin(CurRadian);
			const float NowPointPosXOut = CenterPointPos.X + PolygonEdgeSize * CosX;
			const float NowPointPosYOut = CenterPointPos.Y + PolygonEdgeSize * SinY;
			const float NowPointPosXIn = CenterPointPos.X + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * CosX;
			const float NowPointPosYIn = CenterPointPos.Y + (PolygonEdgeSize - Thickness * PolygonEdgeSize) * SinY;

			VertexHelper.AddVert(FVector(NowPointPosXOut, NowPointPosYOut, 0), Color, CenterUV + FVector2D(UVOutDeltaX * CosX, -UVOutDeltaY * SinY), UV1);
			VertexHelper.AddVert(FVector(NowPointPosXIn, NowPointPosYIn, 0), Color, CenterUV + FVector2D(UVInDeltaX * CosX, -UVInDeltaY * SinY), UV1);
			CurRadian += DegreeDelta;
		}
	}

	for (int32 Index = 0; Index < CurSegements; Index++)
	{
		VertexHelper.AddTriangle((0 + Index * 2) + StartIndex, (2 + Index * 2) + StartIndex, (1 + Index * 2) + StartIndex);
		VertexHelper.AddTriangle((2 + Index * 2) + StartIndex, (3 + Index * 2) + StartIndex, (1 + Index * 2) + StartIndex);
	}
}

void FGeometryUtility::GeneratePoints(FVertexHelper& VertexHelper, bool bAntiAliasing, 
                                      FVector2D UV1, float PointSize, const FLinearColor PointColor, EPointMode PointMode, const TArray<FVector2D>& Points, FVector2D TopLeftUV, FVector2D BottomRightUV)
{
	switch (PointMode)
	{
	case EPointMode::Square:
		for (const auto& Point : Points)
		{
			GenerateSquare(VertexHelper, bAntiAliasing, Point, PointSize, UV1, PointColor, TopLeftUV, BottomRightUV);
		}
		break;
	case EPointMode::Triangle:
		for (const auto& Point : Points)
		{
			GenerateTriangle(VertexHelper, bAntiAliasing, Point, PointSize, UV1, PointColor, TopLeftUV, BottomRightUV);
		}
		break;
	case EPointMode::Circle:
		for (const auto& Point : Points)
		{
			GenerateCircle(VertexHelper, bAntiAliasing, Point, 64, PointSize, UV1, PointColor, TopLeftUV, BottomRightUV);
		}
		break;
	case EPointMode::Diamond:
		for (const auto& Point : Points)
		{
			GenerateDiamond(VertexHelper, bAntiAliasing, Point, PointSize, UV1, PointColor, TopLeftUV, BottomRightUV);
		}
		break;
	default:
		break;
	}
}

void FGeometryUtility::GenerateCurve(FVertexHelper& VertexHelper, bool bAntiAliasing, FVector2D UV1, float Thickness,
	const FLinearColor& Color, const UCurveFloat* Curve)
{
	TArray<FVector2D> Points;
	if (IsValid(Curve))
	{
		const TArray<FRichCurveKey>& CurveKeys = Curve->FloatCurve.GetConstRefOfKeys();
		if (CurveKeys.Num() > 1)
		{
			for (int32 Index = 0; Index < (CurveKeys.Num() - 1) ; ++Index)
			{
				const FRichCurveKey& StartRichCurveKey = CurveKeys[Index];
				const FRichCurveKey& EndRichCurveKey = CurveKeys[Index + 1];

				float LeaveTangentHypotenuse = FMath::Sqrt(1 + (FMath::Square(StartRichCurveKey.LeaveTangent)));
				float ArriveTangentHypotenuse = FMath::Sqrt(1 + (FMath::Square(EndRichCurveKey.ArriveTangent)));
				FVector2D P1 = FVector2D(StartRichCurveKey.Time,StartRichCurveKey.Value) + StartRichCurveKey.LeaveTangentWeight * FVector2D( (1 / LeaveTangentHypotenuse), StartRichCurveKey.LeaveTangent / LeaveTangentHypotenuse);
				FVector2D P2 = FVector2D(EndRichCurveKey.Time,EndRichCurveKey.Value) - EndRichCurveKey.ArriveTangentWeight * FVector2D( (1 / ArriveTangentHypotenuse), EndRichCurveKey.ArriveTangent / ArriveTangentHypotenuse);
				
				BuildBezierGeometry(FVector2D(StartRichCurveKey.Time,StartRichCurveKey.Value), P1, P2, FVector2D(EndRichCurveKey.Time,EndRichCurveKey.Value), Points);
			}
			GenerateLines(VertexHelper, bAntiAliasing,false, UV1, Thickness, Color, Points);
		}
	}
	
}

void FGeometryUtility::BuildBezierGeometry(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, TArray<FVector2D>& Points)
{
	Subdivide(P0, P1, P2, P3,  Points, 1.0f);
}

void FGeometryUtility::Subdivide(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, TArray<FVector2D>& Points, float MaxBiasTimesTwo)
{
	const float Curviness = ComputeCurviness(P0, P1, P2, P3);
	if (Curviness > MaxBiasTimesTwo)
	{
		// Split the Bezier into two curves.
		FVector2D TwoCurves[7];
		DeCasteljauSplit(P0, P1, P2, P3, TwoCurves);
		// Subdivide left, then right
		Subdivide(TwoCurves[0], TwoCurves[1], TwoCurves[2], TwoCurves[3], Points, MaxBiasTimesTwo);
		Subdivide(TwoCurves[3], TwoCurves[4], TwoCurves[5], TwoCurves[6], Points, MaxBiasTimesTwo);
	}
	else
	{
		Points.Emplace(P3);
	}
}

float FGeometryUtility::ComputeCurviness(const FVector2D&  P0, const FVector2D&  P1, const FVector2D&  P2, const FVector2D&  P3)
{
	FVector2D TwoP1Deviations = P0 + P2 - 2 * P1;
	FVector2D TwoP2Deviations = P1 + P3 - 2 * P2;
	float TwoDeviations = FMath::Abs(TwoP1Deviations.X) + FMath::Abs(TwoP1Deviations.Y) + FMath::Abs(TwoP2Deviations.X) + FMath::Abs(TwoP2Deviations.Y);
	return TwoDeviations;
}

void FGeometryUtility::DeCasteljauSplit(const FVector2D& P0, const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, FVector2D OutCurveParams[7])
{
	FVector2D L1 = (P0 + P1) * 0.5f;
	FVector2D M = (P1 + P2) * 0.5f;
	FVector2D R2 = (P2 + P3) * 0.5f;

	FVector2D L2 = (L1 + M) * 0.5f;
	FVector2D R1 = (M + R2) * 0.5f;

	FVector2D L3R0 = (L2 + R1) * 0.5f;

	OutCurveParams[0] = P0;
	OutCurveParams[1] = L1;
	OutCurveParams[2] = L2;
	OutCurveParams[3] = L3R0;
	OutCurveParams[4] = R1;
	OutCurveParams[5] = R2;
	OutCurveParams[6] = P3;
}
