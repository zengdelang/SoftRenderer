#pragma once

#include "CoreMinimal.h"
#include "MaskableGraphicSubComponent.h"
#include "RawImageElementInterface.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "RawImageSubComponent.generated.h"
 
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Raw Image"))
class UGUI_API URawImageSubComponent : public UMaskableGraphicSubComponent, public ILayoutElementInterface, public IRawImageElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Graphic)
	UTexture* Texture;

	UPROPERTY(EditAnywhere, Category = Graphic, meta = (DisplayName = "UV Rect"))
	FUVRect UVRect;

	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bPreserveAspect : 1;

	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bFillViewRect : 1;
	
	UPROPERTY(EditAnywhere, Category = Graphic)
	FVector2D OverrideTextureSize;

	UPROPERTY(EditAnywhere, Category = Graphic)
	ERawImageMaskMode ImageMaskType;
	
	UPROPERTY(EditAnywhere, Category = Graphic, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Thickness;
	
public:
	virtual UTexture* GetMainTexture() const override 
	{
		return Texture;
	}

public:
	virtual UTexture* GetTexture() const override
	{
		return Texture;
	}
	
	virtual void SetTexture(UTexture* InTexture) override
	{
		if (Texture != InTexture)
		{
			Texture = InTexture;
			SetMaterialDirty();
		}
	}

	virtual FUVRect GetUVRect() const override
	{
		return UVRect;
	}

	virtual void SetUVRect(FUVRect InUVRect) override
	{
		if (UVRect != InUVRect)
		{
			UVRect = InUVRect;
			SetVerticesDirty();
			SetVerticesDirty();
		}
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

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual ERawImageMaskMode GetImageMaskMode() const override
	{
		return ImageMaskType;
	}

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual void SetImageMaskMode(ERawImageMaskMode InImageMaskType) override
	{
		if (ImageMaskType != InImageMaskType)
		{
			ImageMaskType = InImageMaskType;
			SetVerticesDirty();
		}
	}

	
	UFUNCTION(BlueprintCallable, Category = Image)
	virtual float GetThickness() const override
	{
		return Thickness;
	}

	UFUNCTION(BlueprintCallable, Category = Image)
	virtual void SetThickness(float InThickness) override
	{
		InThickness = FMath::Clamp(InThickness, 0.f, 1.f);
		if (Thickness != InThickness)
		{
			Thickness = InThickness;
			SetVerticesDirty();
		}
	}

protected:	
	void PreserveTextureAspectRatio(FRect& ViewRect, const FVector2D& TextureSize) const;
	FVector4 GetDrawingDimensions(bool bShouldPreserveAspect, FRect& OriginalViewRect);
	
protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;
	
public:
	virtual FVector2D GetNativeSize() override;

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

};
