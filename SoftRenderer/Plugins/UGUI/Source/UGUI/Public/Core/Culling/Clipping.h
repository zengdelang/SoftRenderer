#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/RectMask2DSubComponent.h"

/**
 * Utility class to help when clipping using IClipper.
 */
class FClipping
{
public:
	/**
	 * Find the Rect to use for clipping.
	 * Given the input RectMask2ds find a rectangle that is the overlap of all the inputs.
	 *
	 * @param  RectMaskParents  RectMasks to build the overlap rect from
	 * @param  bValidRect   Was there a valid Rect found.
	 * @return The final compounded overlapping rect
	 */
	static FRect FindCullAndClipRect(const TArray<URectMask2DSubComponent*>& RectMaskParents, bool& bValidRect);
	
};
