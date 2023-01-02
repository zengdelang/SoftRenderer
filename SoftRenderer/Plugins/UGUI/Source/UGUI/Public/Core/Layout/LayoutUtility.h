#pragma once

#include "CoreMinimal.h"
#include "LayoutElementInterface.h"
#include "RectTransformComponent.h"

/**
 * Utility functions for querying layout elements for their minimum, preferred, and flexible sizes.
 */
class UGUI_API FLayoutUtility
{
public:
	typedef float (*FPropertyFinder)(ILayoutElementInterface* Element);
	
public:
	/**
	 * Returns the minimum size of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 * @param  Axis  The axis to query. This can be 0 or 1.
	 * 
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
    static float GetMinSize(URectTransformComponent* Rect, int32 Axis);

	/**
	 * Returns the preferred size of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 * @param  Axis  The axis to query. This can be 0 or 1.
	 * 
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetPreferredSize(URectTransformComponent* Rect, int32 Axis);

	/**
	 * Returns the flexible size of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 * @param  Axis  The axis to query. This can be 0 or 1.
	 * 
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetFlexibleSize(URectTransformComponent* Rect, int32 Axis);
	
    /**
     * Returns the minimum width of the layout element.
     *
     * @param  Rect  The RectTransform of the layout element to query.
     *
     * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
     */
    static float GetMinWidth(URectTransformComponent* Rect);

	/**
	 * Returns the preferred width of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 *
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetPreferredWidth(URectTransformComponent* Rect);

	/**
	 * Returns the flexible width of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 *
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetFlexibleWidth(URectTransformComponent* Rect);

	/**
	 * Returns the minimum height of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 *
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetMinHeight(URectTransformComponent* Rect);

	/**
	 * Returns the minimum height of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 *
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetPreferredHeight(URectTransformComponent* Rect);

	/**
	 * Returns the minimum height of the layout element.
	 *
	 * @param  Rect  The RectTransform of the layout element to query.
	 *
	 * All components on the GameObject that implement the ILayoutElement are queried. The one with the highest priority which has a value for this setting is used. If multiple components have this setting and have the same priority, the maximum value out of those is used.
	 */
	static float GetFlexibleHeight(URectTransformComponent* Rect);
	
	/**
	 * Gets a calculated layout property for the layout element with the given RectTransform.
	 * 
	 * @param  Rect  The RectTransform of the layout element to get a property for.
	 * @param  PropertyFinder  The property to calculate.
	 * @param  DefaultValue  The default value to use if no component on the layout element supplies the given property
	 * 
	 * The calculated value of the layout property.
	 */
	static float GetLayoutProperty(URectTransformComponent* Rect, FPropertyFinder PropertyFinder, float DefaultValue);
	
};
