#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ClippableInterface.generated.h"

struct FRect;

UINTERFACE(BlueprintType)
class UGUI_API UClippableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for elements that can be clipped if they are under an IClipper
 */
class UGUI_API IClippableInterface
{
	GENERATED_BODY()

public:
    /**
     * Will be called when the state of a parent IClippable changed.
     */
	virtual void RecalculateClipping() = 0;

	/**
	 * Clip and cull the IClippable given a specific clipping rect
	 *
	 * @param  ClipRect  The Rectangle in which to clip against.
     * @param  bValidRect  Is the Rect valid. If not then the rect has 0 size.
	 */
	virtual void Cull(FRect ClipRect, bool bValidRect) = 0;

	/**
	 * Set the clip rect for the IClippable.
	 *
	 * @param  ClipRect  The Rectangle for the clipping
     * @param  bValidRect  Is the rect valid.
	 */
    virtual void SetClipRect(FRect ClipRect, bool bValidRect, FRect ClipSoftnessRect) = 0;

	virtual USceneComponent* GetSceneComponent() = 0;
	
};
