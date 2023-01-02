#pragma once

#include "CoreMinimal.h"
#include "Animation/TweenRunner.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "UObject/Interface.h"
#include "GraphicElementInterface.generated.h"

struct FVertexHelper;

DECLARE_MULTICAST_DELEGATE(FGraphicLayoutDirtySignature);
DECLARE_MULTICAST_DELEGATE(FGraphicVertsDirtySignature);
DECLARE_MULTICAST_DELEGATE(FGraphicMaterialDirtySignature);

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UGraphicElementInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

class UGUI_API IGraphicElementInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	FGraphicLayoutDirtySignature OnDirtyLayoutCallback;

	FGraphicVertsDirtySignature OnDirtyVertsCallback;

	FGraphicMaterialDirtySignature OnDirtyMaterialCallback;
	
protected:
	TSharedPtr<FTweenRunner> ColorTweenRunner;

public:
	static FVertexHelper StaticVertexHelper;

public:
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual UCanvasSubComponent* GetCanvas() { return nullptr; }

	virtual FVector2D GetNativeSize() { return FVector2D::ZeroVector; }
 
public:
	virtual UCanvasRendererSubComponent* GetCanvasRenderer() { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual URectTransformComponent* GetTransformComponent() { return nullptr; }
	
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual UTexture* GetMainTexture() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual UMaterialInterface* GetMaterial() const { return nullptr; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetMaterial(UMaterialInterface* InMaterial) {};

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual FLinearColor GetColor() const { return FLinearColor::White; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetColor(FLinearColor InColor) {}

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual bool IsAntiAliasing() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetAntiAliasing(bool bInAntiAliasing) {}

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual bool IsRaycastTarget() const { return false; }

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetRaycastTarget(bool bInRaycastTarget) {}

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual	void SetHidePrimitive(bool bInHidePrimitive) {}
	
public:
	/**
	 * Absolute depth of the graphic, used by rendering and events -- lowest to highest.
	 *
	 * The depth is relative to the first root canvas.
	 *
	 * Canvas
	 *  Graphic - 1
	 *  Graphic - 2
	 *  Nested Canvas
	 *     Graphic - 3
	 *     Graphic - 4
	 *  Graphic - 5
	 *
	 * This value is used to determine draw and event ordering.
	 */
	virtual int32 GetDepth();

	/**
	 * When a GraphicRaycaster is raycasting into the scene it does two things. First it filters the elements using their RectTransform rect. Then it uses this Raycast function to determine the elements hit by the raycast.
	 */
	virtual bool Raycast(const FVector& WorldRayOrigin, const FVector& WorldRayDir) { return false; }

	/**
	 * Set all properties of the Graphic dirty and needing rebuilt.
	 * Dirties Layout, Vertices, and Materials.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetAllDirty();

	/**
	 * Mark the layout as dirty and needing rebuilt.
	 * Send a OnDirtyLayoutCallback notification if any elements are registered.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetLayoutDirty() {}

	/**
	 * Mark the vertices as dirty and needing rebuilt.
	 *
	 * Send a OnDirtyVertsCallback notification if any elements are registered.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetVerticesDirty() {}

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetGraphicEffectsDirty() {}

	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetRenderOpacityDirty() {}
	
	/**
	 * Mark the material as dirty and needing rebuilt.
	 * Send a OnDirtyMaterialCallback notification if any elements are registered.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual void SetMaterialDirty() {}

protected:
	/**
	 * Call to update the Material of the graphic onto the CanvasRenderer.
	 */
	virtual void UpdateMaterial() {}

	virtual void UpdateGraphicEffects() {};

	virtual void UpdateRenderOpacity() {};
	
	/**
	 * Call to update the geometry of the graphic onto the CanvasRenderer.
	 */
	virtual void UpdateGeometry() {};

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) {};

public:
	/**
	 * Adjusts the given pixel to be pixel perfect.
	 *
	 * Note: This is only accurate if the Graphic root Canvas is in Screen Space.
	 *
	 * @param  Point  Local space point.
	 * @return Pixel perfect adjusted point.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual FVector2D PixelAdjustPoint(FVector2D Point) { return FVector2D::ZeroVector; }

	/**
	 * Returns a pixel perfect Rect closest to the Graphic RectTransform.
	 *
	 * Note: This is only accurate if the Graphic root Canvas is in Screen Space.
	 *
	 * @return A Pixel perfect Rect.
	 */
	UFUNCTION(BlueprintCallable, Category = Graphic)
	virtual FRect GetPixelAdjustedRect() { return FRect(); };

protected:
	void SetCanvasRendererColor(FLinearColor InColor);

public:
	/**
	 * Tweens the CanvasRenderer color associated with this Graphic.
	 *
	 * @param   TargetColor  Target color.
	 * @param   Duration  Tween duration.
	 * @param   bIgnoreTimeScale  Should ignore Time.scale?
	 * @param   bUseAlpha  Should also Tween the alpha channel?
	 */
	virtual void CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha);

	/**
	 * Tweens the CanvasRenderer color associated with this Graphic.
	 *
	 * @param   TargetColor  Target color.
	 * @param   Duration  Tween duration.
	 * @param   bIgnoreTimeScale  Should ignore Time.scale?
	 * @param   bUseAlpha  Should also Tween the alpha channel?
	 * @param   bUseRGB  Should the color or the alpha be used to tween
	 */
	virtual void CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha, bool bUseRGB) {}

	/**
	 * Tweens the alpha of the CanvasRenderer color associated with this Graphic.
	 *
	 * @param   Alpha  Target alpha.
	 * @param   Duration  Duration of the tween in seconds.
	 * @param   bIgnoreTimeScale  Should ignore Time.scale?
	 */
	virtual void CrossFadeAlpha(float Alpha, float Duration, bool bIgnoreTimeScale);

private:
	static FLinearColor CreateColorFromAlpha(float Alpha);
	
};
