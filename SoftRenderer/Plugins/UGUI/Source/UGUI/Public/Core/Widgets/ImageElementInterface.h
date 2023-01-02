#pragma once

#include "CoreMinimal.h"
#include "PaperSprite.h"
#include "Core/UIMargin.h"
#include "UObject/Interface.h"
#include "ImageElementInterface.generated.h"

struct FRect;
struct FVertexHelper;

/**
 * Image fill type controls how to display the image.
 */
UENUM(BlueprintType)
enum class EImageFillType : uint8
{
    /**
     * Displays the full Image
     * 
     * This setting shows the entire image stretched across the Image's RectTransform
     */
    ImageFillType_Simple UMETA(DisplayName = "Simple"),

    /**
     * Displays the Image as a 9-sliced graphic.
     * 
     * A 9-sliced image displays a central area stretched across the image surrounded by a border comprising of 4 corners and 4 stretched edges
     * This has the effect of creating a resizable skinned rectangular element suitable for dialog boxes, windows, and general UI elements.
     */
    ImageFillType_Sliced UMETA(DisplayName = "Sliced"),

    /**
     * Displays a sliced Sprite with its resizable sections tiled instead of stretched.
     * 
     * A Tiled image behaves similarly to a UI.Image.Type.Sliced|Sliced image, except that the resizable sections of the image are repeated instead of being stretched. This can be useful for detailed UI graphics that do not look good when stretched.
     * It uses the Sprite.border value to determine how each part (border and center) should be tiled.
     * The Image sections will repeat the corresponding section in the Sprite until the whole section is filled. The corner sections will be unaffected and will draw in the same way as a Sliced Image. The edges will repeat along their lengths. The center section will repeat across the whole central part of the Image.
     * The Image section will repeat the corresponding section in the Sprite until the whole section is filled.
     * Be aware that if you are tiling a Sprite with borders or a packed sprite, a mesh will be generated to create the tiles. The size of the mesh will be limited to 16250 quads; if your tiling would require more tiles, the size of the tiles will be enlarged to ensure that the number of generated quads stays below this limit.
     * For optimum efficiency, use a Sprite with no borders and with no packing, and make sure the Sprite.texture wrap mode is set to TextureWrapMode.Repeat.These settings will prevent the generation of additional geometry.If this is not possible, limit the number of tiles in your Image.
     */
    ImageFillType_Tiled UMETA(DisplayName = "Tiled"),

    /**
     * Displays only a portion of the Image.
     * 
     * A Filled Image will display a section of the Sprite, with the rest of the RectTransform left transparent. The Image.FillAmount determines how much of the Image to show, and Image.FillMethod controls the shape in which the Image will be cut.
     * This can be used for example to display circular or linear status information such as timers, health bars, and loading bars.
     */
     ImageFillType_Filled UMETA(DisplayName = "Filled"),
};

/**
 * The possible fill method types for a Filled Image.
 */
UENUM(BlueprintType)
enum class EFillMethod : uint8
{
    /**
     * The Image will be filled Horizontally.
     *
     * The Image will be Cropped at either left or right size depending on Image.FillOrigin at the Image.FillAmount
     */
    FillMethod_Horizontal UMETA(DisplayName = "Horizontal"),

    /**
     * Displays the Image as a 9-sliced graphic.
     *
     * A 9-sliced image displays a central area stretched across the image surrounded by a border comprising of 4 corners and 4 stretched edges
     * This has the effect of creating a resizable skinned rectangular element suitable for dialog boxes, windows, and general UI elements.
     */
	   FillMethod_Vertical UMETA(DisplayName = "Vertical"),

    /**
	    * The Image will be filled Radially with the radial center in one of the corners.
	    *
	    * For this method the Image.FillAmount represents an angle between 0 and 90 degrees. The Image will be cut by a line passing at the Image.FillOrigin at the specified angle.
	    */
	   FillMethod_Radial90 UMETA(DisplayName = "Radial90"),

    /**
     * The Image will be filled Radially with the radial center in one of the edges.
     *
     * For this method the Image.FillAmount represents an angle between 0 and 180 degrees. The Image will be cut by a line passing at the Image.FillOrigin at the specified angle.
     */
    FillMethod_Radial180 UMETA(DisplayName = "Radial180"),

    /**
     * The Image will be filled Radially with the radial center at the center.
     *
     * or this method the Image.FillAmount represents an angle between 0 and 360 degrees. The Arc defined by the center of the Image, the Image.FillOrigin and the angle will be cut from the Image.
     */
    FillMethod_Radial360 UMETA(DisplayName = "Radial360"),
};

/**
 * Origin for the Image.FillMethod.Horizontal.
 */
UENUM(BlueprintType)
enum class EOriginHorizontal : uint8
{
    /**
     * Origin at the Left side.
     */
    OriginHorizontal_Left UMETA(DisplayName = "Left"),

    /**
     * Origin at the Right side.
     */
    OriginHorizontal_Right UMETA(DisplayName = "Right"),
};

/**
 * Origin for the Image.FillMethod.Vertical.
 */
UENUM(BlueprintType)
enum class EOriginVertical : uint8
{
    /**
     * Origin at the Bottom Edge.
     */
    OriginVertical_Bottom UMETA(DisplayName = "Bottom"),

    /**
     * Origin at the Top Edge.
     */
    OriginVertical_Top UMETA(DisplayName = "Top"),
};

/**
 * Origin for the Image.FillMethod.Radial90.
 */
UENUM(BlueprintType)
enum class EOrigin90 : uint8
{
    /**
     * Radial starting at the Bottom Left corner.
     */
    Origin90_BottomLeft UMETA(DisplayName = "BottomLeft"),

    /**
     * Radial starting at the Top Left corner.
     */
    Origin90_TopLeft UMETA(DisplayName = "TopLeft"),

    /**
     * Radial starting at the Top Right corner.
     */
    Origin90_TopRight UMETA(DisplayName = "TopRight"),

    /**
     * Radial starting at the Bottom Right corner.
     */
    Origin90_BottomRight UMETA(DisplayName = "BottomRight"),
};

/**
 * Origin for the Image.FillMethod.Radial180.
 */
UENUM(BlueprintType)
enum class EOrigin180 : uint8
{
    /**
     * Center of the radial at the center of the Bottom edge.
     */
    Origin180_Bottom UMETA(DisplayName = "Bottom"),

    /**
     * Center of the radial at the center of the Left edge.
     */
    Origin180_Left UMETA(DisplayName = "Left"),

    /**
     * Center of the radial at the center of the Top edge.
     */
    Origin180_Top UMETA(DisplayName = "Top"),

    /**
     * Center of the radial at the center of the Right edge.
     */
    Origin180_Right UMETA(DisplayName = "Right"),
};

/**
 * One of the points of the Arc for the Image.FillMethod.Radial360.
 */
UENUM(BlueprintType)
enum class EOrigin360 : uint8
{
    /**
     * Arc starting at the center of the Bottom edge.
     */
    Origin360_Bottom UMETA(DisplayName = "Bottom"),

    /**
     * Arc starting at the center of the Right edge.
     */
    Origin360_Right UMETA(DisplayName = "Right"),

    /**
     * Arc starting at the center of the Top edge.
     */
    Origin360_Top UMETA(DisplayName = "Top"),

    /**
     * Arc starting at the center of the Left edge.
     */
    Origin360_Left UMETA(DisplayName = "Left"),
};

UENUM(BlueprintType)
enum class EImageMaskMode : uint8
{
    MaskMode_None UMETA(DisplayName = "None"),
    MaskMode_Circle UMETA(DisplayName = "Circle"),
    MaskMode_CircleRing UMETA(DisplayName = "CircleRing")
};

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UImageElementInterface : public UInterface
{
     GENERATED_UINTERFACE_BODY()
};

class UGUI_API IImageElementInterface
{
    GENERATED_IINTERFACE_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = Image)
    virtual UObject* GetActiveSprite() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual UObject* GetSprite() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetSprite(UObject* InSprite) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual UObject* GetOverrideSprite() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetOverrideSprite(UObject* InSprite) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual EImageFillType GetImageType() const { return EImageFillType::ImageFillType_Simple; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetImageType(EImageFillType InImageType) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual EImageMaskMode GetImageMaskMode() const { return EImageMaskMode::MaskMode_None; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetImageMaskMode(EImageMaskMode InImageMaskType) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual EFillMethod GetFillMethod() const { return EFillMethod::FillMethod_Horizontal; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillMethod(EFillMethod InFillMethod) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual  float GetFillAmount() const { return 0; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillAmount(float InFillAmount) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual float GetThickness() const { return 1; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetThickness(float InThickness) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual int32 GetFillOrigin() const { return 0; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillOrigin(int32 InFillOrigin) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual bool HasBorder() const { return false; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual FUIMargin GetOverrideBorder() const { return FUIMargin(); }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual FVector4 GetBorderSize() const { return FVector4(); }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetBorder(FUIMargin InBorder) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual float GetPixelsPerUnitMultiplier() const { return 1; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetPixelsPerUnitMultiplier(float InValue) {}
  
    UFUNCTION(BlueprintCallable, Category = Image)
    virtual bool GetPreserveAspect() const { return false; }
 
    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetPreserveAspect(bool bInPreserveAspect) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual bool GetFillViewRect() const { return false; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillViewRect(bool bInFillViewRect) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual bool GetFillCenter() const { return true; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillCenter(bool bInFillCenter) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual bool GetFillClockwise() const { return false; }

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual void SetFillClockwise(bool bInFillClockwise) {}

    UFUNCTION(BlueprintCallable, Category = Image)
    virtual float GetPixelsPerUnit() { return 1; };

protected:
	   static void AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color, FVector2D InQuadUVs[], FVector2D UV1,
	                       EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect);
    static void AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color, FVector2D InQuadUVs[], FVector2D UV1, FVector2D InQuadUV2s[],
                        EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect);
    static void AddQuad(FVertexHelper& VertexHelper, FVector2D InQuadPositions[], FLinearColor Color, FVector2D InQuadUVs[], FVector2D UV1, FVector2D InQuadUV2s[],
                 EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect, const int32 StartPointIndex);
    static void AddQuad(FVertexHelper& VertexHelper, FVector2D PosMin, FVector2D PosMax, FLinearColor Color, FVector2D UVMin, FVector2D UVMax, FVector2D UV1,
                        EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect);
    static void AddQuad(FVertexHelper& VertexHelper, FVector2D PosMin, FVector2D PosMax, FLinearColor Color, FVector2D UVMin, FVector2D UVMax, FVector2D UV1, FVector2D UV2Min, FVector2D UV2Max,
                        EImageMaskMode MaskType, FVector2D UV5, const FRect& ViewRect);
 
	   static bool RadialCut(FVector2D InQuadPositions[], FVector2D InQuadUVs[], float Fill, bool bInvert, int32 Corner);
	   static bool RadialCut(FVector2D InQuadPositions[], FVector2D InQuadUVs[], FVector2D InQuadUV2s[], float Fill, bool bInvert, int32 Corner);
	   static void RadialCut(FVector2D InQuadXY[], float Cos, float Sin, bool bInvert, int32 Corner);
	   static void RadialCutUV2(FVector2D InQuadXY[], float Cos, float Sin, bool bInvert, int32 Corner);
    static void RadialCutUV2(FVector2D InQuadUV2XY[], float Fill, bool bClockwise, int32 StartQuadIndex, int32 QuadIndex, int32 CornerIndex);
};
