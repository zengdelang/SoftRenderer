#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LayoutElementInterface.generated.h"

UINTERFACE(BlueprintType)
class UGUI_API ULayoutElementInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * A component is treated as a layout element by the auto layout system if it implements ILayoutElement.
 *
 * The layout system will invoke CalculateLayoutInputHorizontal before querying minWidth, preferredWidth, and flexibleWidth. It can potentially save performance if these properties are cached when CalculateLayoutInputHorizontal is invoked, so they don't need to be recalculated every time the properties are queried.
 *
 * The layout system will invoke CalculateLayoutInputVertical before querying minHeight, preferredHeight, and flexibleHeight.It can potentially save performance if these properties are cached when CalculateLayoutInputVertical is invoked, so they don't need to be recalculated every time the properties are queried.
 *
 * The minWidth, preferredWidth, and flexibleWidth properties should not rely on any properties of the RectTransform of the layout element, otherwise the behavior will be non-deterministic.
 * The minHeight, preferredHeight, and flexibleHeight properties may rely on horizontal aspects of the RectTransform, such as the width or the X component of the position.
 * Any properties of the RectTransforms on child layout elements may always be relied on.
 */
class UGUI_API ILayoutElementInterface
{
    GENERATED_BODY()

public:
    /**
     * After this method is invoked, layout horizontal input properties should return up-to-date values.
     * Children will already have up-to-date layout horizontal inputs when this methods is called.
     */
    virtual void CalculateLayoutInputHorizontal() = 0;

    /**
      * After this method is invoked, layout vertical input properties should return up-to-date values.
      * Children will already have up-to-date layout vertical inputs when this methods is called.
      */
    virtual void CalculateLayoutInputVertical() = 0;

	   /**
      * The minimum width this layout element may be allocated.
      */
    virtual float GetMinWidth() = 0;
	
    /**
     * The preferred width this layout element should be allocated if there is sufficient space.
     *
     * PreferredWidth can be set to -1 to remove the size.
     */
    virtual float GetPreferredWidth() = 0;

    /**
     * The extra relative width this layout element should be allocated if there is additional available space.
     *
     * Setting preferredWidth to -1 removed the preferredWidth.
     */
    virtual float GetFlexibleWidth() = 0;
	
    /**
     * The minimum height this layout element may be allocated.
     */
    virtual float GetMinHeight() = 0;

    /**
     * The preferred height this layout element should be allocated if there is sufficient space.
     *
     * PreferredHeight can be set to -1 to remove the size.
     */
    virtual float GetPreferredHeight() = 0;
	
    /**
     * The extra relative height this layout element should be allocated if there is additional available space.
     */
    virtual float GetFlexibleHeight() = 0;

    /**
     * The layout priority of this component.
     *
     * If multiple components on the same GameObject implement the ILayoutElement interface, the values provided by components that return a higher priority value are given priority. However, values less than zero are ignored. This way a component can override only select properties by leaving the remaning values to be -1 or other values less than zero.
     */
    virtual int32 GetLayoutPriority() =0;
	
};
