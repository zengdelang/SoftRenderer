#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Core/Render/VertexHelper.h"
#include "MeshModifierInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API UMeshModifierInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IMeshModifierInterface
{
	GENERATED_BODY()

public:
	virtual void ModifyMesh(FVertexHelper& VertexHelper) = 0;
	
};
