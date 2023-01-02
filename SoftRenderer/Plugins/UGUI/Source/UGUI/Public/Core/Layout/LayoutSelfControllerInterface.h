#pragma once

#include "CoreMinimal.h"
#include "LayoutControllerInterface.h"
#include "LayoutSelfControllerInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULayoutSelfControllerInterface : public ULayoutControllerInterface
{
	GENERATED_BODY()
};

/**
 * ILayoutSelfController is an ILayoutController that should drive its own RectTransform.
 *
 * The iLayoutSelfController derives from the base controller [[ILayoutController]] and controls the layout of a RectTransform.
 *
 * Use the ILayoutSelfController to manipulate a GameObject’s own RectTransform component, which you attach in the Inspector.Use ILayoutGroup to manipulate RectTransforms belonging to the children of the GameObject.
 *
 * Call ILayoutController.SetLayoutHorizontal to handle horizontal parts of the layout, and call ILayoutController.SetLayoutVertical to handle vertical parts.
 * You can change the height, width, position and rotation of the RectTransform.
 */
class UGUI_API ILayoutSelfControllerInterface : public ILayoutControllerInterface
{
	GENERATED_BODY()
	
};
