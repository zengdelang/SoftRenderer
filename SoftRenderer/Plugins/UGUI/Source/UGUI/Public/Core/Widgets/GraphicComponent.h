#pragma once

#include "CoreMinimal.h"
#include "GraphicElementInterface.h"
#include "UIPrimitiveElementInterface.h"
#include "Core/CanvasElementInterface.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "GraphicComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisallowedSubClasses = "GraphicSubComponent"))
class UGUI_API UGraphicComponent : public URectTransformComponent, public ICanvasElementInterface, public IGraphicElementInterface, public IUIPrimitiveElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Graphic)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = Graphic)
	UMaterialInterface* Material;

	UPROPERTY()
	UMaterialInterface* OverrideMaterial;
	
protected:
	UPROPERTY(Transient)
	UCanvasSubComponent* Canvas;

	UPROPERTY(Transient)
	UCanvasRendererSubComponent* CanvasRenderer;
	
	uint8 bIsVertsDirty : 1;
	uint8 bIsMaterialDirty : 1;
	uint8 bIsGraphicsEffectDirty : 1;
	uint8 bIsRenderOpacityDirty : 1;
	
	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bRaycastTarget : 1;

	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bAntiAliasing : 1;

public:
#if WITH_EDITOR
	uint8 bRegisterGraphicForEditor : 1;
#endif
	
public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	//~ Begin BehaviourComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
	virtual void OnRectTransformDimensionsChange() override;
	virtual void OnTransformParentChanged() override;
	virtual void OnTransformChanged() override;
	virtual void OnBlockRaycastsStateChanged() override;
	//~ End BehaviourComponent Interface.

public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return this; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface

	//~ Begin IUIPrimitiveElementInterface Interface
	virtual int32 GetNumOverrideMaterials() const override;
	virtual UMaterialInterface* GetOverrideMaterial(int32 MaterialIndex) const override;
	virtual void SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) override;
	//~ End IUIPrimitiveElementInterface Interface
	
public:
	virtual void OnTextLocalizationChanged()
	{
		SetAllDirty();
	}
	
protected:
	virtual void OnGrayingStateChanged() override;
	virtual void OnInvertColorStateChanged() override;
	virtual void OnRenderOpacityChanged() override;

public:
	/**
	 * Called when the state of the parent Canvas is changed.
	 */
	virtual void OnCanvasHierarchyChanged() override;
	
	/**
	 * This method must be called when CanvasRenderer.IsCull() is modified.
	 *
	 * This can be used to perform operations that were previously skipped because the Graphic was culled.
	 */
	virtual void OnCullingChanged();
	
public:
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual UCanvasSubComponent* GetCanvas() override;

private:
	void CacheCanvas();
	
public:
	virtual UCanvasRendererSubComponent* GetCanvasRenderer() override
	{
		if (IsValid(CanvasRenderer))
			return CanvasRenderer;

		CanvasRenderer = Cast<UCanvasRendererSubComponent>(GetComponent(UCanvasRendererSubComponent::StaticClass(), true));
		return CanvasRenderer;
	}

	virtual URectTransformComponent* GetTransformComponent() override { return this; }
	
	virtual UTexture* GetMainTexture() const override
	{
		return nullptr;
	}

	virtual UMaterialInterface* GetMaterial() const override
	{
		if (OverrideMaterial)
			return OverrideMaterial;
		return Material;
	}

	virtual void SetMaterial(UMaterialInterface* InMaterial) override
	{
		if (Material != InMaterial)
		{
			Material = InMaterial;
			SetMaterialDirty();
		}
	}

	virtual FLinearColor GetColor() const override
	{
		return Color;
	}

	virtual void SetColor(FLinearColor InColor) override
	{
		if (Color != InColor)
		{
			Color = InColor;
			SetVerticesDirty();
		}
	}

	virtual bool IsAntiAliasing() const override
	{
		return bAntiAliasing;
	}

	virtual void SetAntiAliasing(bool bInAntiAliasing) override;

	virtual bool IsRaycastTarget() const override
	{
		return bRaycastTarget;
	}

	virtual void SetRaycastTarget(bool bInRaycastTarget) override;

	virtual void SetHidePrimitive(bool bInHidePrimitive) override;

public:
	virtual bool Raycast(const FVector& WorldRayOrigin, const FVector& WorldRayDir) override;

	virtual void SetLayoutDirty() override;
	virtual void SetVerticesDirty() override;
	virtual void SetGraphicEffectsDirty() override;
	virtual void SetRenderOpacityDirty() override;
	virtual void SetMaterialDirty() override;

protected:
	virtual void UpdateMaterial() override;
	virtual void UpdateGraphicEffects() override;
	virtual void UpdateRenderOpacity() override;
	virtual void UpdateGeometry() override;

	virtual FVector2D GetUV1FromGraphicEffects() const;

private:
	void DoMeshGeneration();
	
	FORCEINLINE bool IsRaycastTargetForGraphic() const
	{
#if WITH_EDITOR
		if (GetWorld() && !GetWorld()->IsGameWorld())
		{
			return HasBeenEnabled() && bRegisterGraphicForEditor && IsBlockRaycastsInHierarchy();
		}
#endif
		return HasBeenEnabled() && bRaycastTarget && IsBlockRaycastsInHierarchy();
	}

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
	virtual FVector2D PixelAdjustPoint(FVector2D Point) override;
	virtual FRect GetPixelAdjustedRect() override;

public:
	virtual void CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha, bool bUseRGB) override;
	
};
