#pragma once

#include "CoreMinimal.h"
#include "HorizontalOrVerticalLayoutGroupSubComponent.h"
#include "HorizontalLayoutGroupSubComponent.generated.h"

/**
 * Layout child layout elements side by side.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Horizontal Layout Group"))
class UGUI_API UHorizontalLayoutGroupSubComponent : public UHorizontalOrVerticalLayoutGroupSubComponent
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
