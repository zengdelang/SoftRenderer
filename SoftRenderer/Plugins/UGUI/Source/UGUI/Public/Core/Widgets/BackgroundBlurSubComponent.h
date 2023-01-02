#pragma once

#include "CoreMinimal.h"
#include "MaskableGraphicSubComponent.h"
#include "Core/UICommonDefinitions.h"
#include "BackgroundBlurElementInterface.h"
#include "BackgroundBlurSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Background Blur"))
class UGUI_API UBackgroundBlurSubComponent : public UMaskableGraphicSubComponent, public IBackgroundBlurElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * How blurry the background is. Larger numbers mean more blurry but will result in larger runtime cost on the gpu.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 100), Category = BackgroundBlur)
	float BlurStrength;

	/**
	 * This is the number of pixels which will be weighted in each direction from any given pixel when computing the blur
	 * A larger value is more costly but allows for stronger blurs.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 255, EditCondition = "bOverrideAutoRadiusCalculation"), Category = BackgroundBlur)
	int32 BlurRadius;

	/** True to modulate the strength of the blur based on the widget alpha. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BackgroundBlur)
	uint8 bApplyAlphaToBlur : 1;

	/** Whether or not the radius should be computed automatically or if it should use the radius */
	UPROPERTY()
	uint8 bOverrideAutoRadiusCalculation : 1;

	UPROPERTY(EditAnywhere, Category = BackgroundBlur)
	EBackgroundBlurMaskMode BlurMaskType;

	/** BlurComponent's alpha will mul MaskTexture's alpha */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BackgroundBlur)
	UTexture* MaskTexture;
	
protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

public:
    virtual float GetBlurStrength() override
    {
        return BlurStrength;
    }

    virtual void SetBlurStrength(float InBlurStrength) override
	{
		if (BlurStrength == InBlurStrength)
			return;

		BlurStrength = InBlurStrength;
    	
		if (GraphicData.IsValid())
		{
			GraphicData->BlurStrength = BlurStrength;

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetGraphicData(GraphicData);
			}
		}
		else
		{
			SetVerticesDirty();
		}
	}

	virtual int32 GetBlurRadius() override
	{
		return BlurRadius;
	}

	virtual void SetBlurRadius(int32 InBlurRadius) override
	{
		if (BlurRadius == InBlurRadius)
			return;

		BlurRadius = InBlurRadius;

		if (GraphicData.IsValid())
		{
			GraphicData->BlurRadius = BlurRadius;

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetGraphicData(GraphicData);
			}
		}
		else
		{
			SetVerticesDirty();
		}
	}

	virtual bool IsApplyAlphaToBlur() override
	{
		return bApplyAlphaToBlur;
	}

	virtual void SetApplyAlphaToBlur(bool bInApplyAlphaToBlur) override
	{
		if (bApplyAlphaToBlur == bInApplyAlphaToBlur)
			return;

		bApplyAlphaToBlur = bInApplyAlphaToBlur;

		if (GraphicData.IsValid())
		{
			GraphicData->bApplyAlphaToBlur = bApplyAlphaToBlur;

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetGraphicData(GraphicData);
			}
		}
		else
		{
			SetVerticesDirty();
		}
	}

	virtual bool IsOverrideAutoRadiusCalculation() override
	{
		return bOverrideAutoRadiusCalculation;
	}

	virtual void SetOverrideAutoRadiusCalculation(bool bInOverrideAutoRadiusCalculation) override
	{
		if (bOverrideAutoRadiusCalculation == bInOverrideAutoRadiusCalculation)
			return;

		bOverrideAutoRadiusCalculation = bInOverrideAutoRadiusCalculation;

		if (GraphicData.IsValid())
		{
			GraphicData->bOverrideAutoRadiusCalculation = bOverrideAutoRadiusCalculation;

			const auto CanvasRendererComp = GetCanvasRenderer();
			if (IsValid(CanvasRendererComp))
			{
				CanvasRendererComp->SetGraphicData(GraphicData);
			}
		}
		else
		{
			SetVerticesDirty();
		}
	}

	virtual UTexture* GetMainTexture() const override
    {
    	return MaskTexture;
    }

	virtual UTexture* GetMaskTexture() override
    {
    	return MaskTexture;
    }

	virtual void SetMaskTexture(UTexture2D* InMaskTexture) override
    {
    	if (MaskTexture != InMaskTexture)
    	{
    		MaskTexture = InMaskTexture;
    		SetMaterialDirty();
    	}
    }
};
