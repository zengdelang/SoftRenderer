#pragma once

#include "CoreMinimal.h"
#include "HorizontalOrVerticalLayoutGroupComponent.h"
#include "HorizontalLayoutGroupComponent.generated.h"

/**
 * Layout child layout elements side by side.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UHorizontalLayoutGroupComponent : public UHorizontalOrVerticalLayoutGroupComponent
{
	GENERATED_BODY()

public:
    virtual void CalculateLayoutInputHorizontal() override
    {
        Super::CalculateLayoutInputHorizontal();
        CalcAlongAxis(0, false);
    }

    virtual void CalculateLayoutInputVertical() override
    {
        CalcAlongAxis(1, false);
    }

    virtual void SetLayoutHorizontal() override
    {
        SetChildrenAlongAxis(0, false);
    }

    virtual void SetLayoutVertical() override
    {
        SetChildrenAlongAxis(1, false);
    }
	
};
