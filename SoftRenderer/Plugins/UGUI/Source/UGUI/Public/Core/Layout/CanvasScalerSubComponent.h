#pragma once

#include "CoreMinimal.h"
#include "Core/Render/CanvasSubComponent.h"
#include "CanvasScalerSubComponent.generated.h"

/**
 * Determines how UI elements in the Canvas are scaled.
 */
UENUM(BlueprintType)
enum class ECanvasScalerScaleMode : uint8
{
	/**
	 * Using the Constant Pixel Size mode, positions and sizes of UI elements are specified in pixels on the screen.
	 */
	ScaleMode_ConstantPixelSize UMETA(DisplayName = "Constant Pixel Size"),

	/**
	 * Using the Scale With Screen Size mode, positions and sizes can be specified according to the pixels of a specified reference resolution.
     * If the current screen resolution is larger than the reference resolution, the Canvas will keep having only the resolution of the reference resolution, but will scale up in order to fit the screen. If the current screen resolution is smaller than the reference resolution, the Canvas will similarly be scaled down to fit.
	 */
	ScaleMode_ScaleWithScreenSize UMETA(DisplayName = "Scale With Screen Size"),

	/**
	 * Using the Constant Physical Size mode, positions and sizes of UI elements are specified in physical units, such as millimeters, points, or picas.
	 */
	ScaleMode_ConstantPhysicalSize UMETA(DisplayName = "Constant Physical Size"),
};

/**
 * Scale the canvas area with the width as reference, the height as reference, or something in between.
 */
UENUM(BlueprintType)
enum class ECanvasScalerScreenMatchMode : uint8
{
	/**
	 * Scale the canvas area with the width as reference, the height as reference, or something in between.
	 */
	ScreenMatchMode_MatchWidthOrHeight UMETA(DisplayName = "Match Width Or Height"),

	/**
	 * Expand the canvas area either horizontally or vertically, so the size of the canvas will never be smaller than the reference.
	 */
	ScreenMatchMode_Expand UMETA(DisplayName = "Expand"),

	/**
	 * Crop the canvas area either horizontally or vertically, so the size of the canvas will never be larger than the reference.
	 */
	ScreenMatchMode_Shrink UMETA(DisplayName = "Shrink"),
};

/**
 * The possible physical unit types
 */
UENUM(BlueprintType)
enum class ECanvasScalerUnit : uint8
{
	/**
	 * Use centimeters.
	 * A centimeter is 1/100 of a meter
	 */
	Unit_Centimeters UMETA(DisplayName = "Centimeters"),

	/**
	 * Use millimeters.
	 * A millimeter is 1/10 of a centimeter, and 1/1000 of a meter.
	 */
	Unit_Millimeters UMETA(DisplayName = "Millimeters"),

	/**
	 * Use inches.
	 */
	Unit_Inches UMETA(DisplayName = "Inches"),

	/**
	 * Use points.
	 * One point is 1/12 of a pica, and 1/72 of an inch.
	 */
	Unit_Points UMETA(DisplayName = "Points"),

	/**
	 * Use picas.
	 * One pica is 1/6 of an inch.
	 */
	Unit_Picas UMETA(DisplayName = "Picas"),
};

/**
 * The Canvas Scaler component is used for controlling the overall scale and pixel density of UI elements in the Canvas. This scaling affects everything under the Canvas, including font sizes and image borders.
 *
 * For a Canvas set to 'Screen Space - Overlay', the Canvas Scaler UI Scale Mode can be set to Constant Pixel Size, Scale With Screen Size, or Constant Physical Size.
 *
 * Using the Constant Pixel Size mode, positions and sizes of UI elements are specified in pixels on the screen. This is also the default functionality of the Canvas when no Canvas Scaler is attached. However, With the Scale Factor setting in the Canvas Scaler, a constant scaling can be applied to all UI elements in the Canvas.
 *
 * Using the Scale With Screen Size mode, positions and sizes can be specified according to the pixels of a specified reference resolution. If the current screen resolution is larger than the reference resolution, the Canvas will keep having only the resolution of the reference resolution, but will scale up in order to fit the screen. If the current screen resolution is smaller than the reference resolution, the Canvas will similarly be scaled down to fit. If the current screen resolution has a different aspect ratio than the reference resolution, scaling each axis individually to fit the screen would result in non-uniform scaling, which is generally undesirable. Instead of this, the ReferenceResolution component will make the Canvas resolution deviate from the reference resolution in order to respect the aspect ratio of the screen. It is possible to control how this deviation should behave using the ::ref::screenMatchMode setting.
 *
 * Using the Constant Physical Size mode, positions and sizes of UI elements are specified in physical units, such as millimeters, points, or picas. This mode relies on the device reporting its screen DPI correctly. You can specify a fallback DPI to use for devices that do not report a DPI.
 *
 * For a Canvas set to 'World Space' the Canvas Scaler can be used to control the pixel density of UI elements in the Canvas.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Canvas Scaler", DisallowMultipleComponent, RequireSubClasses = "CanvasSubComponent"))
class UGUI_API UCanvasScalerSubComponent : public UBehaviourSubComponent
{
	GENERATED_UCLASS_BODY()
		 
protected:
	/**
	 * Determines how UI elements in the Canvas are scaled.
	 */
	UPROPERTY(EditAnywhere, meta = (DisplayName = "UI Scale Mode"), Category = CanvasScaler)
	ECanvasScalerScaleMode UIScaleMode;

	/**
	 * If a sprite has this 'Pixels Per Unit' setting, then one pixel in the sprite will cover one unit in the UI.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	float ReferencePixelsPerUnit;

	/**
	 * Scales all UI elements in the Canvas by this factor.
	 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01",  UIMin = "0.01"), Category = CanvasScaler)
	float ScaleFactor;

	/**
	 * The resolution the UI layout is designed for. If the screen resolution is larger, the UI will be scaled up, and if it's smaller, the UI will be scaled down. This is done in accordance with the Screen Match Mode.
	 */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.00001", UIMin = "0.00001"), Category = CanvasScaler)
	FVector2D ReferenceResolution;

	/**
	 * A mode used to scale the canvas area if the aspect ratio of the current resolution doesn't fit the reference resolution.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	ECanvasScalerScreenMatchMode ScreenMatchMode;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"), Category = CanvasScaler)
	float MatchWidthOrHeight;

	/**
	 * The physical unit to specify positions and sizes in.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	ECanvasScalerUnit PhysicalUnit;

	/**
	 * The DPI to assume if the screen DPI is not known.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	float FallbackScreenDPI;

	/**
	 * The pixels per inch to use for sprites that have a 'Pixels Per Unit' setting that matches the 'Reference Pixels Per Unit' setting.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	float DefaultSpriteDPI;

	/**
	 * The amount of pixels per unit to use for dynamically created bitmaps in the UI, such as Text.
	 */
	UPROPERTY(EditAnywhere, Category = CanvasScaler)
	float DynamicPixelsPerUnit;
	
protected:
	UPROPERTY(Transient)
	UCanvasSubComponent* CacheCanvas;

	float PrevScaleFactor;
	float PrevReferencePixelsPerUnit;

	FDelegateHandle ViewportResizeDelegateHandle;

#if WITH_EDITOR
	FDelegateHandle EditorViewportResizeDelegateHandle;
#endif
	
protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourSubComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	/**
	 * Method that handles calculations of canvas scaling.
	 */
	virtual void Handle();

	/**
	 * Handles canvas scaling for world canvas.
	 */
	virtual void HandleWorldCanvas();

	/**
	 * Handles canvas scaling for a constant pixel size.
	 */
	virtual void HandleConstantPixelSize();

	/**
	 * Handles canvas scaling that scales with the screen size.
	 */
	virtual void HandleScaleWithScreenSize();

	/**
	 * Handles canvas scaling for a constant physical size.
	 */
	virtual void HandleConstantPhysicalSize();
	
public:
	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	ECanvasScalerScaleMode GetUIScaleMode() const
	{
		return UIScaleMode;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetUIScaleMode(ECanvasScalerScaleMode InUIScaleMode);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetReferencePixelsPerUnit() const
	{
		return ReferencePixelsPerUnit;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetReferencePixelsPerUnit(float InReferencePixelsPerUnit);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetScaleFactor() const
	{
		return ScaleFactor;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetScaleFactor(float InScaleFactor);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	FVector2D GetReferenceResolution() const
	{
		return ReferenceResolution;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetReferenceResolution(FVector2D InReferenceResolution);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	ECanvasScalerScreenMatchMode GetScreenMatchMode() const
	{
		return ScreenMatchMode;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetScreenMatchMode(ECanvasScalerScreenMatchMode InScreenMatchMode);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetMatchWidthOrHeight() const
	{
		return MatchWidthOrHeight;
	}

	/**
	 * Setting to scale the Canvas to match the width or height of the reference resolution, or a combination.
	 *
	 * If the setting is set to 0, the Canvas is scaled according to the difference between the current screen resolution width and the reference resolution width. If the setting is set to 1, the Canvas is scaled according to the difference between the current screen resolution height and the reference resolution height.
	 *
     * For values in between 0 and 1, the scaling is based on a combination of the relative width and height.
     *
     * Consider an example where the reference resolution of 640x480, and the current screen resolution is a landscape mode of 480x640.
     *
     * If the scaleWidthOrHeight setting is set to 0, the Canvas is scaled by 0.75 because the current resolution width of 480 is 0.75 times the reference resolution width of 640. The Canvas resolution gets a resolution of 640x853.33. This resolution has the same width as the reference resolution width, but has the aspect ratio of the current screen resolution. Note that the Canvas resolution of 640x853.33 is the current screen resolution divided by the scale factor of 0.75.
     *
     * If the scaleWidthOrHeight setting is set to 1, the Canvas is scaled by 1.33 because the current resolution height of 640 is 1.33 times the reference resolution height of 480. The Canvas resolution gets a resolution of 360x480. This resolution has the same height as the reference resolution width, but has the aspect ratio of the current screen resolution. Note that the Canvas resolution of 360x480 is the current screen resolution divided by the scale factor of 1.33.
     *
     * If the scaleWidthOrHeight setting is set to 0.5, we find the horizontal scaling needed (0.75) and the vertical scaling needed (1.33) and find the average. However, we do the average in logarithmic space. A regular average of 0.75 and 1.33 would produce a result of 1.04. However, since multiplying by 1.33 is the same as diving by 0.75, the two scale factor really corresponds to multiplying by 0.75 versus dividing by 0.75, and the average of those two things should even out and produce a neutral result. The average in logarithmic space of 0.75 and 1.33 is exactly 1.0, which is what we want. The Canvas resolution hence ends up being 480x640 which is the current resolution divided by the scale factor of 1.0.
     *
     * The logic works the same for all values. The average between the horizontal and vertical scale factor is a weighted average based on the matchWidthOrHeight value.
	 */
	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetMatchWidthOrHeight(float InMatchWidthOrHeight);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	ECanvasScalerUnit GetPhysicalUnit() const
	{
		return PhysicalUnit;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetPhysicalUnit(ECanvasScalerUnit InPhysicalUnit);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetFallbackScreenDPI() const
	{
		return FallbackScreenDPI;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetFallbackScreenDPI(float InFallbackScreenDPI);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetDefaultSpriteDPI() const
	{
		return DefaultSpriteDPI;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetDefaultSpriteDPI(float InDefaultSpriteDPI);

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	float GetDynamicPixelsPerUnit() const
	{
		return DynamicPixelsPerUnit;
	}

	UFUNCTION(BlueprintCallable, Category = CanvasScaler)
	void SetDynamicPixelsPerUnit(float InDynamicPixelsPerUnit);
	
protected:
	/**
	 * Sets the scale factor on the canvas.
	 *
	 * @param  InScaleFactor  The scale factor to use.
	 */
	void SetCanvasScaleFactor(float InScaleFactor);

	/**
	 * Sets the referencePixelsPerUnit on the Canvas.
	 *
	 * @param  InReferencePixelsPerUnit  The new reference pixels per Unity value
	 */
	void SetCanvasReferencePixelsPerUnit(float InReferencePixelsPerUnit);

	void OnViewportResized();

#if WITH_EDITOR
	void OnEditorViewportResized(const UWorld* InWorld);
#endif
	
};
