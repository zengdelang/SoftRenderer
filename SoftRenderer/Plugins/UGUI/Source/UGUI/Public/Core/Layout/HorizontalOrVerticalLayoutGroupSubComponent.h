#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/LayoutGroupSubComponent.h"
#include "HorizontalOrVerticalLayoutGroupSubComponent.generated.h"

/**
 * Abstract base class for HorizontalLayoutGroup and VerticalLayoutGroup to generalize common functionality.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UHorizontalOrVerticalLayoutGroupSubComponent : public ULayoutGroupSubComponent
{
    GENERATED_UCLASS_BODY()

protected:
	/**
	 * The spacing to use between layout elements in the layout group.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    float Spacing;

	/**
	 * Whether to force the children to expand to fill additional available horizontal space.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildForceExpandWidth : 1;

	/**
	 * Whether to force the children to expand to fill additional available vertical space.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildForceExpandHeight : 1;

	/**
	 * Returns true if the Layout Group controls the widths of its children. Returns false if children control their own widths.
	 *
	 * If set to false, the layout group will only affect the positions of the children while leaving the widths untouched. The widths of the children can be set via the respective RectTransforms in this case.
     *
     * If set to true, the widths of the children are automatically driven by the layout group according to their respective minimum, preferred, and flexible widths. This is useful if the widths of the children should change depending on how much space is available.In this case the width of each child cannot be set manually in the RectTransform, but the minimum, preferred and flexible width for each child can be controlled by adding a LayoutElement component to it.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildControlWidth : 1;

	/**
     * Returns true if the Layout Group controls the heights of its children. Returns false if children control their own heights.
	 *
     * If set to false, the layout group will only affect the positions of the children while leaving the heights untouched. The heights of the children can be set via the respective RectTransforms in this case.
     *
     * If set to true, the heights of the children are automatically driven by the layout group according to their respective minimum, preferred, and flexible heights. This is useful if the heights of the children should change depending on how much space is available.In this case the height of each child cannot be set manually in the RectTransform, but the minimum, preferred and flexible height for each child can be controlled by adding a LayoutElement component to it.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildControlHeight : 1;

	/**
	 * Whether children widths are scaled by their x scale.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildScaleWidth : 1;

	/**
	 * Whether children heights are scaled by their y scale.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
	uint8 bChildScaleHeight : 1;

public:
	UFUNCTION(BlueprintCallable, Category = Layout)
	float GetSpacing() const
	{
		return Spacing;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetSpacing(float InSpacing)
	{
		if (Spacing != InSpacing)
		{
			Spacing = InSpacing;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildForceExpandWidth() const
	{
		return bChildForceExpandWidth;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildForceExpandWidth(bool bInChildForceExpandWidth)
	{
		if (bChildForceExpandWidth != bInChildForceExpandWidth)
		{
			bChildForceExpandWidth = bInChildForceExpandWidth;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildForceExpandHeight() const
	{
		return bChildForceExpandHeight;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildForceExpandHeight(bool bInChildForceExpandHeight)
	{
		if (bChildForceExpandHeight != bInChildForceExpandHeight)
		{
			bChildForceExpandHeight = bInChildForceExpandHeight;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildControlWidth() const
	{
		return bChildControlWidth;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildControlWidth(bool bInChildControlWidth)
	{
		if (bChildControlWidth != bInChildControlWidth)
		{
			bChildControlWidth = bInChildControlWidth;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildControlHeight() const
	{
		return bChildControlHeight;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildControlHeight(bool bInChildControlHeight)
	{
		if (bChildControlHeight != bInChildControlHeight)
		{
			bChildControlHeight = bInChildControlHeight;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildScaleWidth() const
	{
		return bChildScaleWidth;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildScaleWidth(bool bInChildScaleWidth)
	{
		if (bChildScaleWidth != bInChildScaleWidth)
		{
			bChildScaleWidth = bInChildScaleWidth;
			SetDirty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	bool GetChildScaleHeight() const
	{
		return bChildScaleHeight;
	}

	UFUNCTION(BlueprintCallable, Category = Layout)
	void SetChildScaleHeight(bool bInChildScaleHeight)
	{
		if (bChildScaleHeight != bInChildScaleHeight)
		{
			bChildScaleHeight = bInChildScaleHeight;
			SetDirty();
		}
	}

protected:
	/**
	 * Calculate the layout element properties for this layout element along the given axis.
	 *
	 * @param  Axis  The axis to calculate for. 0 is horizontal and 1 is vertical.
	 * @param  bIsVertical  Is this group a vertical group?
	 */
	void CalcAlongAxis(int32 Axis, bool bIsVertical);

	/**
	 * Set the positions and sizes of the child layout elements for the given axis.
	 *
	 * @param  Axis  The axis to calculate for. 0 is horizontal and 1 is vertical.
	 * @param  bIsVertical  Is this group a vertical group?
	 */
	void SetChildrenAlongAxis(int32 Axis, bool bIsVertical);

private:
	void GetChildSizes(URectTransformComponent* Child, int32 Axis, bool bControlSize, bool bChildForceExpand, float& Min, float& Preferred, float& Flexible) const;
	
};
