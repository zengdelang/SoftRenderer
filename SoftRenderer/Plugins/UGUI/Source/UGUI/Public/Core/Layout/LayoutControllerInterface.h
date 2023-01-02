#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LayoutControllerInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULayoutControllerInterface : public UInterface
{
    GENERATED_BODY()
};

/***
 * Base interface to implement by components that control the layout of RectTransforms.
 *
 * If a component is driving its own RectTransform it should implement the interface [[ILayoutSelfController]].
 * If a component is driving the RectTransforms of its children, it should implement [[ILayoutGroup]].
 *
 * The layout system will first invoke SetLayoutHorizontal and then SetLayoutVertical.
 *
 * In the SetLayoutHorizontal call it is valid to call LayoutUtility.GetMinWidth, LayoutUtility.GetPreferredWidth, and LayoutUtility.GetFlexibleWidth on the RectTransform of itself or any of its children.
 * In the SetLayoutVertical call it is valid to call LayoutUtility.GetMinHeight, LayoutUtility.GetPreferredHeight, and LayoutUtility.GetFlexibleHeight on the RectTransform of itself or any of its children.
 *
 * The component may use this information to determine the width and height to use for its own RectTransform or the RectTransforms of its children.
 *
 */
class UGUI_API ILayoutControllerInterface
{
    GENERATED_BODY()

public:
    /**
     *  Callback invoked by the auto layout system which handles horizontal aspects of the layout.
     */
    virtual void SetLayoutHorizontal() = 0;

    /***
     * Callback invoked by the auto layout system which handles vertical aspects of the layout.
     */
    virtual void SetLayoutVertical() = 0;
	
};
