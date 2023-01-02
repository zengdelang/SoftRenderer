#pragma once

#include "CoreMinimal.h"
#include "GraphicElementInterface.h"
#include "Core/CanvasElementInterface.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "GraphicSubComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisplayName = "Graphic", DisallowMultipleComponent, RequireSubClasses = "CanvasRendererSubComponent"))
class UGUI_API UGraphicSubComponent : public UBehaviourSubComponent, public ICanvasElementInterface, public IGraphicElementInterface
{
	GENERATED_UCLASS_BODY()

	friend class UBehaviourComponent;
	
protected:
	UPROPERTY(EditAnywhere, Category = Graphic)
	FLinearColor Color;

	UPROPERTY(EditAnywhere, Category = Graphic)
	UMaterialInterface* Material;

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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Graphic)
	uint8 bGraying : 1;

private:
	uint8 bCurrentGraying : 1;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Graphic)
	uint8 bInvertColor : 1;

private:
	uint8 bCurrentInvertColor : 1;

public:
#if WITH_EDITOR
	uint8 bRegisterGraphicForEditor : 1;
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Graphic)
	float RenderOpacity;

private:
	float CurrentRenderOpacity;
	
public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
	virtual void OnRectTransformDimensionsChange() override;
	virtual void OnTransformParentChanged() override;
	virtual void OnTransformChanged() override;
	virtual void OnBlocksRaycastsStateChanged() override;
	//~ End BehaviourSubComponent Interface.
	
public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return AttachTransform; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface

public:
	virtual void OnTextLocalizationChanged()
	{
		SetAllDirty();
	}

protected:
	virtual void OnGrayingStateChanged();
	virtual void OnInvertColorStateChanged();
	virtual void OnRenderOpacityChanged();

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

protected:
	void InternalUpdateGrayingState();
	void InternalUpdateInvertColorState();
	void InternalUpdateRenderOpacity();
	
public:
	UFUNCTION(BlueprintCallable, Category = Graphic)
	void SetGraying(bool bInGraying);

	UFUNCTION(BlueprintCallable, Category = Graphic)
	bool IsGraying() const { return bGraying; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	bool IsGrayingInHierarchy() const;

public:
	UFUNCTION(BlueprintCallable, Category = Graphic)
	void SetInvertColor(bool bInInvertColor);

	UFUNCTION(BlueprintCallable, Category = Graphic)
	bool IsInvertColor() const { return bInvertColor; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	bool IsInvertColorInHierarchy() const;

public:
	UFUNCTION(BlueprintCallable, Category = Graphic)
	void SetRenderOpacity(float InRenderOpacity);

	UFUNCTION(BlueprintCallable, Category = Graphic)
	float GetRenderOpacity() const { return RenderOpacity; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	float GetRenderOpacityInHierarchy() const;
	
public:
	virtual UCanvasRendererSubComponent* GetCanvasRenderer() override
	{
		if (IsValid(CanvasRenderer))
			return CanvasRenderer;

		CanvasRenderer = Cast<UCanvasRendererSubComponent>(GetComponent(UCanvasRendererSubComponent::StaticClass(), true));
		return CanvasRenderer;
	}

	virtual URectTransformComponent* GetTransformComponent() override { return AttachTransform; }
	
	virtual UTexture* GetMainTexture() const override
	{
		return nullptr;
	}

	virtual UMaterialInterface* GetMaterial() const override
	{
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
