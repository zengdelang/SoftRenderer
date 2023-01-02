#pragma once

#include "CoreMinimal.h"
#include "ImageElementInterface.h"
#include "MaskableGraphicComponent.h"
#include "PaperSprite.h"
#include "Sprite2D.h"
#include "Core/MathUtility.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "Core/Render/CanvasRaycastFilterInterface.h"
#include "Interfaces/SpriteTextureAtlasListenerInterface.h"
#include "ImageComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Widget), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UImageComponent : public UMaskableGraphicComponent, public ILayoutElementInterface, public ICanvasRaycastFilterInterface, public IImageElementInterface, public ISpriteTextureAtlasListenerInterface
{
	GENERATED_UCLASS_BODY()

protected:
    /**
	 * The sprite that is used to render this image.
	 */
    UPROPERTY(EditAnywhere, Category = Graphic, meta=(DisplayThumbnail="true", DisplayName="Source Image", AllowedClasses="Sprite2D,SlateTextureAtlasInterface"))
    UObject* Sprite;

    /**
	 * Set an override sprite to be used for rendering.
	 *
	 * The UI.Image-overrideSprite|overrideSprite variable allows a sprite to have the
	 * sprite changed.This change happens immediately.When the changed
	 * sprite is no longer needed the sprite can be reverted back to the
	 * original version.This happens when the overrideSprite is set to nullptr.
     */
    UPROPERTY(Transient, meta=(DisplayThumbnail="true", AllowedClasses="Sprite2D,SlateTextureAtlasInterface"))
    UObject* OverrideSprite;

    /**
	 * How to display the image.
	 *
	 * This can be used to display:
	 * - Whole images stretched to fit the RectTransform of the Image.
	 * - A 9-sliced image useful for various decorated UI boxes and other rectangular elements.
	 * - A tiled image with sections of the sprite repeated.
	 * - As a partial image, useful for wipes, fades, timers, status bars etc.
     */
    UPROPERTY(EditAnywhere, Category = Graphic)
    EImageFillType ImageType;

    UPROPERTY(EditAnywhere, Category = Graphic)
    EImageMaskMode ImageMaskType;

	/**
	 * Filling method for filled sprites.
	 */
    UPROPERTY(EditAnywhere, Category = Graphic)
    EFillMethod FillMethod;

	/** Amount of the Image shown. 0-1 range with 0 being nothing shown, and 1 being the full Image. */
    UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float FillAmount;

    UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float Thickness;

    /** Controls the origin point of the Fill process. Value means different things with each fill method. */
    UPROPERTY(EditAnywhere, Category = Graphic)
    int32 FillOrigin;

    UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    FUIMargin OverrideBorder;
    
	/**
	 * Pixel per unit modifier to change how sliced sprites are generated.
	 */
    UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.01", UIMin = "0.01"))
    float PixelsPerUnitMultiplier;

    /** cache referencePixelsPerUnit when canvas parent is disabled; */
    float CachedReferencePixelsPerUnit;

#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, Category = Graphic)
    EOriginHorizontal OriginHorizontal;

    UPROPERTY(EditAnywhere, Category = Graphic)
    EOriginVertical OriginVertical;

    UPROPERTY(EditAnywhere, Category = Graphic)
    EOrigin90 Origin90;

    UPROPERTY(EditAnywhere, Category = Graphic)
    EOrigin180 Origin180;

    UPROPERTY(EditAnywhere, Category = Graphic)
    EOrigin360 Origin360;
#endif
	
    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bPreserveAspect : 1;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bFillViewRect : 1;

    /**
	 * Whether or not to render the center of a Tiled or Sliced image.
     */
    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bFillCenter : 1;

	/** Whether the Image should be filled clockwise (true) or counter-clockwise (false). */
    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bFillClockwise : 1;

public:
#if WITH_EDITORONLY_DATA
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	//~ Begin BehaviourComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourComponent Interface.
	
public:
    //~ Begin ILayoutElementInterface Interface
    virtual void CalculateLayoutInputHorizontal() override {};
    virtual void CalculateLayoutInputVertical() override {};
    virtual float GetMinWidth() override { return 0; };
    virtual float GetPreferredWidth() override;
    virtual float GetFlexibleWidth() override { return -1; };
    virtual float GetMinHeight() override { return 0; };
    virtual float GetPreferredHeight() override;
    virtual float GetFlexibleHeight() override { return -1; };
    virtual int32 GetLayoutPriority() override { return 0; };
    //~ End ILayoutElementInterface Interface

public:
    //~ Begin ICanvasRaycastFilterInterface Interface
    virtual bool IsRaycastLocationValid(class IMaskableGraphicElementInterface* MaskableGraphicElement, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphicsScreenPoint) override;
    //~ End ICanvasRaycastFilterInterface Interface.

    //~ Begin ISpriteTextureAtlasListenerInterface Interface
    virtual void NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged) override
    {
        if (bUVChanged)
        {
            SetVerticesDirty();
        }

        if (bTextureChanged)
        {
            SetMaterialDirty();
        }
    }
    //~ End ISpriteTextureAtlasListenerInterface Interface.

protected:
    virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
    virtual FVector2D GetNativeSize() override;

    virtual UTexture* GetMainTexture() const override
    {
		const auto ActiveSprite = GetActiveSprite();

        if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
        {
            return PaperSprite->GetSlateAtlasData().AtlasTexture;
        }
        
        if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
        {
            return Sprite2D->GetSpriteTexture();
        }
        
        return nullptr;
    }
	
public:
    virtual UObject* GetActiveSprite() const override
    {
        if (IsValid(OverrideSprite))
            return OverrideSprite;
        return Sprite;
    }

    virtual UObject* GetSprite() const override
    {
        return Sprite;
    }

    virtual void SetSprite(UObject* InSprite) override
    {
        if (!IsActiveAndEnabled())
        {
            Sprite = InSprite;
            return;
        }
        
        if (IsValid(Sprite))
        {
        	if (Sprite != InSprite)
        	{
        	    const UTexture* OldAtlasTexture = nullptr;
        	    FVector2D OldSpriteSize = FVector2D::ZeroVector;

        	    const UTexture* NewAtlasTexture = nullptr;
        	    FVector2D NewSpriteSize = FVector2D::ZeroVector;
        	    
        	    if (const auto PaperSprite = Cast<UPaperSprite>(Sprite))
        	    {
        	        const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
        	        OldSpriteSize = GetSpriteSize(SlateAtlasData);
        	        OldAtlasTexture = SlateAtlasData.AtlasTexture;
        	    }
        	    else if (const auto Sprite2D = Cast<USprite2D>(Sprite))
        	    {
        	        Sprite2D->RemoveSpriteListener(this);
        	        Sprite2D->DecreaseReferenceCount();
        	        OldSpriteSize = Sprite2D->GetSpriteSize();
        	        OldAtlasTexture = Sprite2D->GetSpriteTextureRaw();
        	    }

        	    if (const auto PaperSprite = Cast<UPaperSprite>(InSprite))
        	    {
        	        const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
        	        NewSpriteSize = GetSpriteSize(SlateAtlasData);
        	        NewAtlasTexture = SlateAtlasData.AtlasTexture;
        	    }
        	    else if (const auto Sprite2D = Cast<USprite2D>(InSprite))
        	    {
        	        Sprite2D->AddSpriteListener(this);
        	        Sprite2D->IncreaseReferenceCount();
        	        NewSpriteSize = Sprite2D->GetSpriteSize();
        	        NewAtlasTexture = Sprite2D->GetSpriteTexture();
        	    }
        	    
                const bool bSkipLayoutUpdate = OldSpriteSize.Equals(NewSpriteSize);
                const bool bSkipMaterialUpdate = OldAtlasTexture == NewAtlasTexture;

                Sprite = InSprite;

                if (!bSkipLayoutUpdate)
					SetLayoutDirty();

                if (!bSkipMaterialUpdate)
                    SetMaterialDirty();

                SetVerticesDirty();
        	}
        }
        else if (IsValid(InSprite))
        {
            Sprite = InSprite;

            const UTexture* AtlasTexture = nullptr;
            FVector2D SpriteSize = FVector2D::ZeroVector;
            if (const auto PaperSprite = Cast<UPaperSprite>(InSprite))
            {
                const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
                SpriteSize = GetSpriteSize(SlateAtlasData);
                AtlasTexture = SlateAtlasData.AtlasTexture;
            }
            else if (const auto Sprite2D = Cast<USprite2D>(InSprite))
            {
            	Sprite2D->AddSpriteListener(this);
            	Sprite2D->IncreaseReferenceCount();
            	SpriteSize = Sprite2D->GetSpriteSize();
            	AtlasTexture = Sprite2D->GetSpriteTexture();
            }
            
        	if (!(FMathUtility::Approximately(SpriteSize.X, 0.0f) && FMathUtility::Approximately(SpriteSize.Y, 0.0f)))
                SetLayoutDirty();
        	
        	if (IsValid(AtlasTexture))
				SetMaterialDirty();
        	
            SetVerticesDirty();
        }
    }
	
    virtual UObject* GetOverrideSprite() const override
    {
        return GetActiveSprite();
    }

    virtual void SetOverrideSprite(UObject* InSprite) override
    {
    	if (!IsActiveAndEnabled())
    	{
    		OverrideSprite = InSprite;
    		return;
    	}
    	
        if (OverrideSprite != InSprite)
        {
        	if (const auto Sprite2D = Cast<USprite2D>(OverrideSprite))
        	{
        		Sprite2D->RemoveSpriteListener(this);
        		Sprite2D->DecreaseReferenceCount();
        	}
        	
            OverrideSprite = InSprite;

        	if (const auto Sprite2D = Cast<USprite2D>(InSprite))
        	{
        		Sprite2D->AddSpriteListener(this);
        		Sprite2D->IncreaseReferenceCount();
        	}
        	
            SetAllDirty();
        }
    }

    virtual EImageFillType GetImageType() const override
    {
        return ImageType;
    }

    virtual void SetImageType(EImageFillType InImageType) override
    {
        if (ImageType != InImageType)
        {
            ImageType = InImageType;
            SetVerticesDirty();
        }
    }
    
    virtual EImageMaskMode GetImageMaskMode() const override
    {
        return ImageMaskType;
    }
    
    virtual void SetImageMaskMode(EImageMaskMode InImageMaskType) override
    {
        if (ImageMaskType != InImageMaskType)
        {
            ImageMaskType = InImageMaskType;
            SetVerticesDirty();
        }
    }

    virtual EFillMethod GetFillMethod() const override
    {
        return FillMethod;
    }

    virtual void SetFillMethod(EFillMethod InFillMethod) override
    {
        if (FillMethod != InFillMethod)
        {
            FillMethod = InFillMethod;
            SetVerticesDirty();

            FillOrigin = 0;
        }
    }

    virtual float GetFillAmount() const override
    {
        return FillAmount;
    }

    virtual void SetFillAmount(float InFillAmount) override
    {
        InFillAmount = FMath::Clamp(InFillAmount, 0.f, 1.f);
        if (FillAmount != InFillAmount)
        {
            FillAmount = InFillAmount;
            SetVerticesDirty();
        }
    }

    virtual float GetThickness() const override
    {
        return Thickness;
    }

    virtual void SetThickness(float InThickness) override
    {
        InThickness = FMath::Clamp(InThickness, 0.f, 1.f);
        if (Thickness != InThickness)
        {
            Thickness = InThickness;
            SetVerticesDirty();
        }
    }

    virtual int32 GetFillOrigin() const override
    {
        return FillOrigin;
    }

    virtual void SetFillOrigin(int32 InFillOrigin) override
    {
        if (FillOrigin != InFillOrigin)
        {
            FillOrigin = InFillOrigin;
            SetVerticesDirty();
        }
    }

    virtual bool HasBorder() const override
    {
        return OverrideBorder.Left != 0 || OverrideBorder.Right != 0 || OverrideBorder.Top != 0 || OverrideBorder.Bottom != 0;
    }

    virtual FUIMargin GetOverrideBorder() const override
    {
        return OverrideBorder;
    }

    virtual FVector4 GetBorderSize() const override
    {
        const auto ActiveSprite = GetActiveSprite();
        if (IsValid(ActiveSprite))
        {
            FVector2D Size = FVector2D::ZeroVector;
            if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
            {
               Size = GetSpriteSize(PaperSprite->GetSlateAtlasData());
            }
            else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
            {
                Size = Sprite2D->GetSpriteSize();
            }
            return FVector4(Size.X * OverrideBorder.Left, Size.Y * OverrideBorder.Bottom, Size.X * OverrideBorder.Right, Size.Y * OverrideBorder.Top);
        }
        return FVector4(0, 0, 0, 0);
    }

    virtual void SetBorder(FUIMargin InBorder) override
    {
        if (OverrideBorder != InBorder)
        {
            OverrideBorder.Left = FMath::Clamp(InBorder.Left, 0.0f, 1.0f);
            OverrideBorder.Bottom = FMath::Clamp(InBorder.Bottom, 0.0f, 1.0f);
            OverrideBorder.Top = FMath::Clamp(InBorder.Top, 0.0f, 1.0f);
            OverrideBorder.Right = FMath::Clamp(InBorder.Right, 0.0f, 1.0f);

        	if (ImageType != EImageFillType::ImageFillType_Sliced ||
                ImageType != EImageFillType::ImageFillType_Tiled)
        	{
                SetVerticesDirty();
        	}
        }
    }

    virtual float GetPixelsPerUnitMultiplier() const override
    {
        return PixelsPerUnitMultiplier;
    }

    virtual void SetPixelsPerUnitMultiplier(float InValue) override
    {
        PixelsPerUnitMultiplier = FMath::Max(0.01f, InValue);
    }

    virtual bool GetPreserveAspect() const override
    {
        return bPreserveAspect;
    }

    virtual void SetPreserveAspect(bool bInPreserveAspect) override
    {
        if (bPreserveAspect != bInPreserveAspect)
        {
            bPreserveAspect = bInPreserveAspect;
            SetVerticesDirty();
        }
    }

    virtual bool GetFillViewRect() const override
    {
        return bFillViewRect;
    }

    virtual void SetFillViewRect(bool bInFillViewRect) override
    {
        if (bFillViewRect != bInFillViewRect)
        {
            bFillViewRect = bInFillViewRect;
        	
            if (bPreserveAspect)
            {
                SetVerticesDirty();
            }
        }
    }

    virtual bool GetFillCenter() const override
    {
        return bFillCenter;
    }

    virtual void SetFillCenter(bool bInFillCenter) override
    {
        if (bFillCenter != bInFillCenter)
        {
            bFillCenter = bInFillCenter;
            SetVerticesDirty();
        }
    }

    virtual bool GetFillClockwise() const override
    {
		return bFillClockwise;
    }

    virtual void SetFillClockwise(bool bInFillClockwise) override
    {
        if (bFillClockwise != bInFillClockwise)
        {
            bFillClockwise = bInFillClockwise;
            SetVerticesDirty();
        }
    }

    virtual float GetPixelsPerUnit() override;

protected:
    FORCEINLINE static FVector2D GetSpriteSize(const FSlateAtlasData& AtlasData)
    {
        const auto AtlasTexture = Cast<UTexture2D>(AtlasData.AtlasTexture);
        if (IsValid(AtlasTexture))
        {
            return AtlasData.SizeUV * AtlasTexture->GetImportedSize();
        }
    	return FVector2D::ZeroVector;
    }

    FORCEINLINE FVector4 GetInnerUV(const FVector4& OuterUV) const
    {
        const auto& StartUV = FVector2D(OuterUV.X, OuterUV.Y);
        const auto& SizeUV =  FVector2D(OuterUV.Z - OuterUV.X, OuterUV.W - OuterUV.Y);
    	
        const float X = StartUV.X + OverrideBorder.Left * SizeUV.X;
        const float Y = StartUV.Y + OverrideBorder.Top * SizeUV.Y;
        const float Z = StartUV.X + (1 - OverrideBorder.Right) * SizeUV.X;
        const float W = StartUV.Y + (1 - OverrideBorder.Bottom) * SizeUV.Y;
    	
        return FVector4(X, Y, Z, W);
    }

    FORCEINLINE FVector4 GetBorderSize(const FSlateAtlasData& AtlasData) const
    {
        const auto Size = GetSpriteSize(AtlasData);
        return FVector4(Size.X * OverrideBorder.Left, Size.Y * OverrideBorder.Bottom, Size.X * OverrideBorder.Right, Size.Y * OverrideBorder.Top);
    }

    FORCEINLINE FVector4 GetBorderSize(const USprite2D* Sprite2D) const
    {
        const auto Size = Sprite2D->GetSpriteSize();
        return FVector4(Size.X * OverrideBorder.Left, Size.Y * OverrideBorder.Bottom, Size.X * OverrideBorder.Right, Size.Y * OverrideBorder.Top);
    }

	float MultipliedPixelsPerUnit()
	{
        return PixelsPerUnitMultiplier * GetPixelsPerUnit();
	}
	
private:
    void PreserveSpriteAspectRatio(FRect& ViewRect, const FVector2D& SpriteSize) const;
    FVector4 GetDrawingDimensions(bool bShouldPreserveAspect, FRect& OriginalViewRect);
	
    void GenerateSimpleSprite(FVertexHelper& VertexHelper, bool bInPreserveAspect);

    void GenerateSimpleSpriteAA(FVertexHelper& VertexHelper, bool bInPreserveAspect);
	
	void GenerateSlicedSprite(FVertexHelper& VertexHelper);
	FVector4 GetAdjustedBorders(FVector4 InBorder, const FRect& InAdjustedRect) const;

    void GenerateTiledSprite(FVertexHelper& VertexHelper);
	
    void GenerateFilledSprite(FVertexHelper& VertexHelper, bool bInPreserveAspect);
	
};
