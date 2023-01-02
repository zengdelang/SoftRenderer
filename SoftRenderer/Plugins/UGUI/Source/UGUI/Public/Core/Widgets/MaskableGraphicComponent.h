#pragma once

#include "CoreMinimal.h"
#include "GraphicComponent.h"
#include "MaskableGraphicElementInterface.h"
#include "Core/Culling/ClippableInterface.h"
#include "MaskableGraphicComponent.generated.h"

class URectMask2DSubComponent;

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UMaskableGraphicComponent : public UGraphicComponent, public IClippableInterface, public IMaskableGraphicElementInterface
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
		UpdateClipParent();
		SetMaterialDirty();
	}
	
public:
	//~ Begin BehaviourComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnCanvasHierarchyChanged() override;
	virtual void OnTransformParentChanged() override;
	//~ End BehaviourComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
public:
	//~ Begin IClippableInterface Interface
	virtual void RecalculateClipping() override;
	virtual void Cull(FRect ClipRect, bool bValidRect) override;
	virtual void SetClipRect(FRect ClipRect, bool bValidRect, FRect ClipSoftnessRect) override;
	virtual USceneComponent* GetSceneComponent() override { return this; };
	//~ End IClippableInterface Interface.

private:
	void UpdateCull(bool bCull);

	FRect GetRootCanvasRect();

	void UpdateClipParent();
	
};
