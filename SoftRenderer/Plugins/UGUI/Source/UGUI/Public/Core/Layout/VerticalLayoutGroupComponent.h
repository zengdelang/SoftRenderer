#pragma once

#include "CoreMinimal.h"
#include "HorizontalOrVerticalLayoutGroupComponent.h"
#include "VerticalLayoutGroupComponent.generated.h"

/**
 * Layout child layout elements below each other.
 *
 * The VerticalLayoutGroup component is used to layout child layout elements below each other.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UVerticalLayoutGroupComponent : public UHorizontalOrVerticalLayoutGroupComponent
{
	GENERATED_BODY()

public:
    virtual void CalculateLayoutInputHorizontal() override
    {
        Super::CalculateLayoutInputHorizontal();
        CalcAlongAxis(0, true);
    }

    virtual void CalculateLayoutInputVertical() override
    {
        CalcAlongAxis(1, true);
    }

    virtual void SetLayoutHorizontal() override
    {
        SetChildrenAlongAxis(0, true);
    }

    virtual void SetLayoutVertical() override
    {
        SetChildrenAlongAxis(1, true);
    }
};
