#pragma once

#include "CoreMinimal.h"
#include "GraphicSubComponent.h"
#include "MaskableGraphicElementInterface.h"
#include "Core/Culling/ClippableInterface.h"
#include "MaskableGraphicSubComponent.generated.h"

class URectMask2DSubComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UMaskableGraphicSubComponent : public UGraphicSubComponent, public IClippableInterface, public IMaskableGraphicElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	URectMask2DSubComponent* ParentMask;

protected:
	/**
	 * bMaskable is whether this graphic is allowed to be masked or not.
	 *
	 * The default for bMaskable is true, so graphics under a mask are masked out of the box.
	 * The maskable property can be turned off from script by the user if masking is not desired.
	 */
	UPROPERTY(EditAnywhere, Category=Graphic)
	uint8 bMaskable : 1;

public:
	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	virtual bool IsMaskable() const override
	{
		return bMaskable;
	}

	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	virtual void SetMaskable(bool bInMaskable) override
	{
		if (bMaskable == bInMaskable)
			return;
		
		bMaskable = bInMaskable;
		SetMaterialDirty();
	}
	
public:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnCanvasHierarchyChanged() override;
	virtual void OnTransformParentChanged() override;
	//~ End BehaviourSubComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:
	//~ Begin IClippableInterface Interface
	virtual void RecalculateClipping() override;
	virtual void Cull(FRect ClipRect, bool bValidRect) override;
	virtual void SetClipRect(FRect ClipRect, bool bValidRect, FRect ClipSoftnessRect) override;
	virtual USceneComponent* GetSceneComponent() override { return AttachTransform; };
	//~ End IClippableInterface Interface.

private:
	void UpdateCull(bool bCull);

	FRect GetRootCanvasRect();

	void UpdateClipParent();
	
};
