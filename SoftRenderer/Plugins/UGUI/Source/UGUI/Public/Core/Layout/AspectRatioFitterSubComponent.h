#pragma once

#include "CoreMinimal.h"
#include "LayoutRebuilder.h"
#include "LayoutSelfControllerInterface.h"
#include "AspectRatioFitterSubComponent.generated.h"

/**
 *  Specifies a mode to use to enforce an aspect ratio.
 */
UENUM(BlueprintType)
enum class EAspectMode : uint8
{
    /**
     * Don't perform any resizing.
     */
    AspectMode_None UMETA(DisplayName = "None"),
	
    /**
     * Changes the height of the rectangle to match the aspect ratio.
     */
    AspectMode_WidthControlsHeight UMETA(DisplayName = "Width Controls Height"),

    /**
     * Changes the width of the rectangle to match the aspect ratio.
     */
    AspectMode_HeightControlsWidth UMETA(DisplayName = "Height Controls Width"),

    /**
     * Sizes the rectangle such that it's fully contained within the parent rectangle.
     */
    AspectMode_FitInParent UMETA(DisplayName = "Fit In Parent"),

    /**
     * Sizes the rectangle such that the parent rectangle is fully contained within.
     */
    AspectMode_EnvelopeParent UMETA(DisplayName = "Envelope Parent"),
	
};

/**
 * Resizes a RectTransform to fit a specified aspect ratio.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (DisplayName = "Aspect Ratio Fitter", DisallowMultipleComponent))
class UGUI_API UAspectRatioFitterSubComponent : public UBehaviourSubComponent, public ILayoutSelfControllerInterface
{
    GENERATED_UCLASS_BODY()

protected:
    /**
     * The mode to use to enforce the aspect ratio.
     */
    UPROPERTY(EditAnywhere, Category = Layout)
    EAspectMode AspectMode;

    /**
     * The aspect ratio to enforce. This means width divided by height.
     */
    UPROPERTY(EditAnywhere, Category = Layout, meta = (ClampMin = "0.001", ClampMax = "1000.0", UIMin = "0.001", UIMax = "1000.0"))
    float AspectRatio;

public:
    UFUNCTION(BlueprintCallable, Category = AspectRatioFitter)
    EAspectMode GetAspectMode() const
    {
        return AspectMode;
    }

    UFUNCTION(BlueprintCallable, Category = AspectRatioFitter)
    void SetAspectMode(EAspectMode InAspectMode)
    {
        if (AspectMode != InAspectMode)
        {
            AspectMode = InAspectMode;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = AspectRatioFitter)
    float GetAspectRatio() const
    {
        return AspectRatio;
    }

    UFUNCTION(BlueprintCallable, Category = AspectRatioFitter)
    void SetAspectRatio(float InAspectRatio)
    {
        InAspectRatio = FMath::Clamp(InAspectRatio, 0.001f, 1000.0f);
        if (AspectRatio != InAspectRatio)
        {
            AspectRatio = InAspectRatio;
            SetDirty();
        }
    }

public:
#if WITH_EDITORONLY_DATA
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

protected:
    //~ Begin BehaviourSubComponent Interface
    virtual void OnEnable() override
    {
        Super::OnEnable();
        SetDirty();
    }

    virtual void OnDisable() override
    {
        FLayoutRebuilder::MarkLayoutForRebuild(Cast<URectTransformComponent>(GetOuter()));
        Super::OnDisable();
    }

    virtual void OnRectTransformDimensionsChange() override { UpdateRect(); }
    //~ End BehaviourSubComponent Interface

public:
    //~ Begin ILayoutSelfControllerInterface Interface
	
    /** Method called by the layout system. Has no effect */
    virtual void SetLayoutHorizontal() override {};

	/** Method called by the layout system. Has no effect */
    virtual void SetLayoutVertical() override {}
	
    //~ End ILayoutSelfControllerInterface Interface

private:
    void UpdateRect() const;

    float GetSizeDeltaToProduceSize(float Size, int32 Axis) const;

    FVector2D GetParentSize() const;
	
protected:
    FORCEINLINE void SetDirty()
    {
        UpdateRect();
    }
	
};
