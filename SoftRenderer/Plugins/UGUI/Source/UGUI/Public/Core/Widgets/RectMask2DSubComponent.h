#pragma once

#include "CoreMinimal.h"
#include "Core/UIMargin.h"
#include "Core/Culling/ClipperInterface.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasRaycastFilterInterface.h"
#include "Core/Culling/ClippableInterface.h"
#include "RectMask2DSubComponent.generated.h"

class UCanvasSubComponent;

/**
 * A 2D rectangular mask that allows for clipping / masking of areas outside the mask.
 *
 * A RectMask2D:
 *   *Only works in the 2D plane
 *   *Requires elements on the mask to be coplanar.
 *   *Does not require stencil buffer / extra draw calls
 *   *Requires fewer draw calls
 *   *Culls elements that are outside the mask area.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Rect Mask 2D", DisallowMultipleComponent))
class UGUI_API URectMask2DSubComponent : public UBehaviourSubComponent, public IClipperInterface, public ICanvasRaycastFilterInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = RectMask2D, meta = (ClampMin = "0", UIMin = "0"))
	FUIMargin Padding;

	UPROPERTY(EditAnywhere, Category = RectMask2D, meta = (ClampMin = "0", UIMin = "0"))
	FUIMargin Softness;
	
	UPROPERTY(Transient)
	TSet<UObject*> MaskableTargets;

	UPROPERTY(Transient)
	TSet<UObject*> ClipTargets;

	UPROPERTY(Transient)
	TArray<URectMask2DSubComponent*> Clippers;
	
	UPROPERTY(Transient)
	UCanvasSubComponent* RootCanvas;

	UPROPERTY(Transient)
	URectTransformComponent* RectTransform;
	
	FRect LastClipRectCanvasSpace;
	FRect LastClipSoftnessRectCanvasSpace;

	uint8 bForceClip : 1;
	uint8 bShouldRecalculateClipRects : 1;

protected:
	UCanvasSubComponent* GetCanvas();
	
public:
	/**
	 * Get the Rect for the mask in canvas space.
	 */
	FRect GetCanvasRect();

	FRect GetRootCanvasRect();
	
	/**
	 * Helper function to get the RectTransform for the mask.
	 */
	URectTransformComponent* GetRectTransform();

protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnCanvasHierarchyChanged() override;
	virtual void OnTransformParentChanged() override;
	//~ End BehaviourSubComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
public:
	//~ Begin ICanvasRaycastFilterInterface Interface
	virtual bool IsRaycastLocationValid(class IMaskableGraphicElementInterface* MaskableGraphicElement, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphicsScreenPoint) override;
	//~ End ICanvasRaycastFilterInterface Interface.

public:
	UFUNCTION(BlueprintCallable, Category = RectMask2D)
	FUIMargin GetPadding() const
	{
		return Padding;
	}

	UFUNCTION(BlueprintCallable, Category = RectMask2D)
	void SetPadding(FUIMargin InPadding)
	{
		Padding = InPadding;
	}

	UFUNCTION(BlueprintCallable, Category = RectMask2D)
	FUIMargin GetSoftness() const
	{
		return Softness;
	}

	UFUNCTION(BlueprintCallable, Category = RectMask2D)
	void SetSoftness(FUIMargin InSoftness)
	{
		Softness = InSoftness;
	}
	
public:
	virtual void PerformClipping() override;

	/**
	 * Add a IClippable to be tracked by the mask.
	 *
	 * @param  Clippable  Add the clippable object for this mask
	 */
	void AddClippable(IClippableInterface* Clippable);

	/**
	 * Remove an IClippable from being tracked by the mask.
	 *
	 * @param  Clippable  Remove the clippable object from this mask
	 */
	void RemoveClippable(IClippableInterface* Clippable);
	
};
