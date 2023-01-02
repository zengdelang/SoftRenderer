#pragma once

#include "CoreMinimal.h"
#include "LayoutControllerInterface.h"
#include "LayoutGroupInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULayoutGroupInterface : public ULayoutControllerInterface
{
	GENERATED_BODY()
};

/**
 * ILayoutGroup is an ILayoutController that should drive the RectTransforms of its children.
 *
 * ILayoutGroup derives from ILayoutController and requires the same members to be implemented.
 */
class UGUI_API ILayoutGroupInterface : public ILayoutControllerInterface
{
	GENERATED_BODY()
	
};
