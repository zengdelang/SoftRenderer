#pragma once

#include "CoreMinimal.h"

class IClippableInterface;
class URectMask2DSubComponent;

/**
 * Mask related utility class. This class provides masking-specific utility functions.
 */
class FMaskUtilities
{
public:
	/**
	 * Notify all IClippables under the given component that they need to recalculate clipping.
	 *
	 * @param  MaskComp  The object thats changed for whose children should be notified.
	 */
	static void Notify2DMaskStateChanged(USceneComponent* MaskComp);

	/**
	 * Helper function to determine if the child is a descendant of father or is father.
	 *
	 * @param  Father  The transform to compare against.
	 * @param  Child  The starting transform to search up the hierarchy.
	 * @return Is child equal to father or is a descendant.
	 */
	static bool IsDescendantOrSelf(const USceneComponent* Father, const USceneComponent* Child);

	/**
	 * Find the correct RectMask2D for a given IClippable.
	 *
	 * @param  Clippable  to search from.
	 * @return  The Correct RectMask2D
	 */
	static URectMask2DSubComponent* GetRectMaskForClippable(IClippableInterface* Clippable);
	
	/**
	 * Search for all RectMask2D that apply to the given RectMask2D (includes self).
	 *
	 * @param  Clipper  Starting clipping object.
	 * @param  Masks  The list of Rect masks
	 */
	static void GetRectMasksForClip(const URectMask2DSubComponent* Clipper, TArray<URectMask2DSubComponent*>& Masks);

};
