#include "Core/Culling/RectangularVertexClipper.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasSubComponent.h"

/////////////////////////////////////////////////////
// FRectangularVertexClipper

FRect FRectangularVertexClipper::GetCanvasRect(const URectTransformComponent* TransformComp, const UCanvasSubComponent* CanvasComp)
{
	if (!IsValid(TransformComp) || !IsValid(CanvasComp) || !IsValid(CanvasComp->GetRectTransform()))
	{
		return FRect();
	}

	FVector WorldCorners[4];
	TransformComp->GetWorldCorners(WorldCorners);

	const auto& WorldToCanvas = CanvasComp->GetRectTransform()->GetComponentTransform().Inverse();

	FVector CanvasCorners[2];
	CanvasCorners[0] = WorldToCanvas.TransformPosition(WorldCorners[0]);
	CanvasCorners[1] = WorldToCanvas.TransformPosition(WorldCorners[2]);
	
	return FRect(FVector2D(CanvasCorners[0]), FVector2D(CanvasCorners[1] - CanvasCorners[0]));
}

/////////////////////////////////////////////////////
