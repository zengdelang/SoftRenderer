#pragma once

#include "CoreMinimal.h"
#include "ShadowSubComponent.h"
#include "OutlineSubComponent.generated.h"

/**
 * Adds an outline to a graphic using IVertexModifier.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = Outline))
class UGUI_API UOutlineSubComponent : public UShadowSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	virtual void ModifyMesh(FVertexHelper& VertexHelper) override;

};
