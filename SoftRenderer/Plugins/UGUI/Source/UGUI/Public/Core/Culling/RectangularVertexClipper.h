#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasSubComponent.h"

class FRectangularVertexClipper
{
public:
	static FRect GetCanvasRect(const URectTransformComponent* TransformComp, const UCanvasSubComponent* CanvasComp);
	
};
