#pragma once

#include "CoreMinimal.h"
#include "LayoutRebuilder.h"
#include "LayoutSelfControllerInterface.h"
#include "Core/CanvasUpdateRegistry.h"
#include "ContentSizeFitterSubComponent.generated.h"

/**
 *  The size fit modes available to use.
 */
UENUM(BlueprintType)
enum class EFitMode : uint8
{
    /**
     * Don't perform any resizing.
     */
    FitMode_Unconstrained UMETA(DisplayName = "Unconstrained"),

    /**
     * Resize to the minimum size of the content.
     */
    FitMode_MinSize UMETA(DisplayName = "Min Size"),

    /**
     * Resize to the preferred size of the content.
     */
    FitMode_PreferredSize UMETA(DisplayName = "Preferred Size"),

};

/**
 * Resizes a RectTransform to fit the size of its content.
 *
 * The ContentSizeFitter can be used on GameObjects that have one or more ILayoutElement components, such as Text, Image, HorizontalLayoutGroup, VerticalLayoutGroup, and GridLayoutGroup.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (DisplayName = "Content Size Fitter"))
class UGUI_API UContentSizeFitterSubComponent : public UBehaviourSubComponent, public ILayoutSelfControllerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * The fit mode to use to determine the width.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
    EFitMode HorizontalFit;

	/**
	 * The fit mode to use to determine the height.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
    EFitMode VerticalFit;

	uint8 bHandlingSelfFitting : 1;
	
public:
    UFUNCTION(BlueprintCallable, Category = ContentSizeFitter)
    EFitMode GetHorizontalFit() const
    {
        return HorizontalFit;
    }

    UFUNCTION(BlueprintCallable, Category = ContentSizeFitter)
    void SetHorizontalFit(EFitMode InHorizontalFit)
    {
        if (HorizontalFit != InHorizontalFit)
        {
            HorizontalFit = InHorizontalFit;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = ContentSizeFitter)
    EFitMode GetVerticalFit() const
    {
        return VerticalFit;
    }

    UFUNCTION(BlueprintCallable, Category = ContentSizeFitter)
    void SetVerticalFit(EFitMode InVerticalFit)
    {
        if (VerticalFit != InVerticalFit)
        {
            VerticalFit = InVerticalFit;
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

    virtual void OnRectTransformDimensionsChange() override;
    //~ End BehaviourSubComponent Interface
	
public:
	//~ Begin ILayoutSelfControllerInterface Interface
    virtual void SetLayoutHorizontal() override
    {
        HandleSelfFittingAlongAxis(0);
    }
	
    virtual void SetLayoutVertical() override
    {
        HandleSelfFittingAlongAxis(1);
    }
	//~ End ILayoutSelfControllerInterface Interface

private:
    void HandleSelfFittingAlongAxis(int32 Axis);
	
protected:
	FORCEINLINE void SetDirty() const
	{
		if (!IsActiveAndEnabled())
			return;
		
		FLayoutRebuilder::MarkLayoutForRebuild(Cast<URectTransformComponent>(GetOuter()));
	}
};
