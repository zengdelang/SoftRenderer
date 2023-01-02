#include "Core/Widgets/CurveComponent.h"
#include "Core/GeometryUtility.h"
#include "Core/Render/VertexHelper.h"

/////////////////////////////////////////////////////
// ULineComponent

UCurveComponent::UCurveComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), Curve(nullptr)
{
	LineThickness = 1.0f;
	PointColor = FLinearColor::White;
	PointSize = 10.0f;
	PointMode = EPointMode::None;
}

void UCurveComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	VertexHelper.Reset();

	const FVector2D UV1 = GetUV1FromGraphicEffects();
	FGeometryUtility::GenerateCurve(VertexHelper, bAntiAliasing, UV1, LineThickness, Color, Curve);
	// FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, LinePoints);
}

/////////////////////////////////////////////////////
