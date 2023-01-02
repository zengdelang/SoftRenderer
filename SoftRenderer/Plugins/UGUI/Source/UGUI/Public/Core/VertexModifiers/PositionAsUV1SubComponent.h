#pragma once

#include "CoreMinimal.h"
#include "BaseMeshEffectSubComponent.h"
#include "PositionAsUV1SubComponent.generated.h"

/**
 * An IVertexModifier which sets the raw vertex position into UV1 of the generated verts.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Position As UV1"))
class UGUI_API UPositionAsUV1SubComponent : public UBaseMeshEffectSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void ModifyMesh(FVertexHelper& VertexHelper) override;

};
