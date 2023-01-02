#pragma once

#include "CoreMinimal.h"
#include "LayoutElementInterface.h"
#include "LayoutIgnorerInterface.h"
#include "LayoutRebuilder.h"
#include "LayoutElementSubComponent.generated.h"

/**
 * Add this component to a GameObject to make it into a layout element or override values on an existing layout element.
 */
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Layout Element"))
class UGUI_API ULayoutElementSubComponent : public UBehaviourSubComponent, public ILayoutElementInterface, public ILayoutIgnorerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bIgnoreLayout : 1;

	UPROPERTY(EditAnywhere, Category = Layout)
	float MinWidth;

	UPROPERTY(EditAnywhere, Category = Layout)
	float MinHeight;

	UPROPERTY(EditAnywhere, Category = Layout)
	float PreferredWidth;

	UPROPERTY(EditAnywhere, Category = Layout)
	float PreferredHeight;

	UPROPERTY(EditAnywhere, Category = Layout)
	float FlexibleWidth;

	UPROPERTY(EditAnywhere, Category = Layout)
	float FlexibleHeight;

	UPROPERTY(EditAnywhere, Category = Layout)
	int32 LayoutPriority;

public:
    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetIgnoreLayout(bool bInIgnoreLayout)
    {
        if (bIgnoreLayout != bInIgnoreLayout)
        {
            bIgnoreLayout = bInIgnoreLayout;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetMinWidth(float InMinWidth)
    {
        if (MinWidth != InMinWidth)
        {
            MinWidth = InMinWidth;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetMinHeight(float InMinHeight)
    {
        if (MinHeight != InMinHeight)
        {
            MinHeight = InMinHeight;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetPreferredWidth(float InPreferredWidth)
    {
        if (PreferredWidth != InPreferredWidth)
        {
            PreferredWidth = InPreferredWidth;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetPreferredHeight(float InPreferredHeight)
    {
        if (PreferredHeight != InPreferredHeight)
        {
            PreferredHeight = InPreferredHeight;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetFlexibleWidth(float InFlexibleWidth)
    {
        if (FlexibleWidth != InFlexibleWidth)
        {
            FlexibleWidth = InFlexibleWidth;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetFlexibleHeight(float InFlexibleHeight)
    {
        if (FlexibleHeight != InFlexibleHeight)
        {
            FlexibleHeight = InFlexibleHeight;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutElement)
    void SetLayoutPriority(int32 InLayoutPriority)
    {
        if (LayoutPriority != InLayoutPriority)
        {
            LayoutPriority = InLayoutPriority;
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
	
    virtual void OnTransformParentChanged() override { SetDirty(); }
    //~ End BehaviourSubComponent Interface
	
public:
	//~ Begin ILayoutIgnorerInterface Interface
	virtual bool IgnoreLayout() override { return bIgnoreLayout; }
	//~ End ILayoutIgnorerInterface Interface
	
public:
	//~ Begin ILayoutElementInterface Interface
	virtual void CalculateLayoutInputHorizontal() override {}
	virtual void CalculateLayoutInputVertical() override {}
	virtual float GetMinWidth() override { return MinWidth; }
	virtual float GetPreferredWidth() override { return PreferredWidth; }
	virtual float GetFlexibleWidth() override { return FlexibleWidth; }
	virtual float GetMinHeight() override { return MinHeight; }
	virtual float GetPreferredHeight() override { return PreferredHeight; }
	virtual float GetFlexibleHeight() override { return FlexibleHeight; }
	virtual int32 GetLayoutPriority() override { return LayoutPriority; }
	//~ End ILayoutElementInterface Interface

protected:
	FORCEINLINE void SetDirty()
	{
		if (!IsActiveAndEnabled())
			return;

		FLayoutRebuilder::MarkLayoutForRebuild(Cast<URectTransformComponent>(GetOuter()));
	}
	
};
