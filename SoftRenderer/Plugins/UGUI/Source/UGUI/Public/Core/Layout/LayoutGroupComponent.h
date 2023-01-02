#pragma once

#include "CoreMinimal.h"
#include "LayoutGroupInterface.h"
#include "LayoutElementInterface.h"
#include "LayoutRebuilder.h"
#include "Core/UIMargin.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Widgets/Text/TextAnchor.h"
#include "Core/WidgetActor.h"
#include "LayoutGroupComponent.generated.h"

/**
 * Abstract base class to use for layout groups.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisallowedSubClasses = "LayoutGroupSubComponent"))
class UGUI_API ULayoutGroupComponent : public URectTransformComponent, public ILayoutElementInterface, public ILayoutGroupInterface
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * The padding to add around the child layout elements.
	 */
	UPROPERTY(EditAnywhere, Category = Layout)
    FUIMargin Padding;

	/**
	 * The alignment to use for the child layout elements in the layout group.
	 *
	 * some child layout elements specify no flexible width and height, the children may not take up all the available space inside the layout group. The alignment setting specifies how to align them within the layout group when this is the case.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    ETextAnchor ChildAlignment;

    UPROPERTY(Transient)
    TArray<URectTransformComponent*> RectChildren;
	
    FVector2D TotalMinSize;
    FVector2D TotalPreferredSize;
    FVector2D TotalFlexibleSize;

private:
    FDelegateHandle EndFrameDelegateHandle;
	
public:
#if WITH_EDITORONLY_DATA
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

    //~ Begin BehaviourComponent Interface
    virtual void OnEnable() override;
    virtual void OnDisable() override;
    virtual void OnRectTransformDimensionsChange() override;
    virtual void OnTransformParentChanged() override;
    //~ End BehaviourComponent Interface.

protected:
	virtual void OnChildAttachmentChanged() override;
	
public:
    //~ Begin ILayoutElementInterface Interface
    virtual void CalculateLayoutInputHorizontal() override;
    virtual void CalculateLayoutInputVertical() override;
    virtual float GetMinWidth() override;
    virtual float GetPreferredWidth() override;
    virtual float GetFlexibleWidth() override;
    virtual float GetMinHeight() override;
    virtual float GetPreferredHeight() override;
    virtual float GetFlexibleHeight() override;
    virtual int32 GetLayoutPriority() override;
    //~ End ILayoutElementInterface Interface

public:
    //~ Begin ILayoutGroupInterface Interface
    virtual void SetLayoutHorizontal() override;
    virtual void SetLayoutVertical() override;
    //~ End ILayoutGroupInterface Interface

public:
    UFUNCTION(BlueprintCallable, Category = LayoutGroup)
    FUIMargin GetPadding() const
    {
        return Padding;
    }

    UFUNCTION(BlueprintCallable, Category = LayoutGroup)
    void SetPadding(FUIMargin InPadding)
    {
        if (Padding != InPadding)
        {
            Padding = InPadding;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = LayoutGroup)
    ETextAnchor GetChildAlignment() const
    {
        return ChildAlignment;
    }

    UFUNCTION(BlueprintCallable, Category = LayoutGroup)
    void SetChildAlignment(ETextAnchor InChildAlignment)
    {
        if (ChildAlignment != InChildAlignment)
        {
            ChildAlignment = InChildAlignment;
            SetDirty();
        }
    }

protected:
	/**
	 * The min size for the layout group on the given axis.
	 */
    FORCEINLINE float GetTotalMinSize(int32 Axis) const
    {
        return TotalMinSize[Axis];
    }

    FORCEINLINE float GetTotalPreferredSize(int32 Axis) const
    {
        return TotalPreferredSize[Axis];
    }

    /**
     * The flexible size for the layout group on the given axis.
     */
    FORCEINLINE float GetTotalFlexibleSize(int32 Axis) const
    {
        return TotalFlexibleSize[Axis];
    }

	/**
	 * @return Returns the calculated position of the first child layout element along the given axis.
	 */
    FORCEINLINE float GetStartOffset(int32 Axis, float RequiredSpaceWithoutPadding)
    {
        const float RequiredSpace = RequiredSpaceWithoutPadding + (Axis == 0 ? Padding.GetTotalSpaceAlong<Orient_Horizontal>() : Padding.GetTotalSpaceAlong<Orient_Vertical>());
        const float AvailableSpace = Rect.GetSize()[Axis];
        const float SurplusSpace = AvailableSpace - RequiredSpace;
        const float AlignmentOnAxis = GetAlignmentOnAxis(Axis);
        return (Axis == 0 ? Padding.Left : Padding.Top) + SurplusSpace * AlignmentOnAxis;
    }

    /**
	 * @return Returns the alignment on the specified axis as a fraction where 0 is left/top, 0.5 is middle, and 1 is right/bottom.
	 */
    FORCEINLINE float GetAlignmentOnAxis(int32 Axis) const
    {
        if (Axis == 0)
            return (static_cast<int32>(ChildAlignment) % 3) * 0.5f;
        return (static_cast<int32>(ChildAlignment) / 3) * 0.5f;
    }

    /**
	 * Used to set the calculated layout properties for the given axis.
	 */
    FORCEINLINE void SetLayoutInputForAxis(float TotalMin, float TotalPreferred, float TotalFlexible, int32 Axis)
    {
        TotalMinSize[Axis] = TotalMin;
        TotalPreferredSize[Axis] = TotalPreferred;
        TotalFlexibleSize[Axis] = TotalFlexible;
    }

    /**
	 * Set the position and size of a child layout element along the given axis.
	 *
	 * @param  InRect  The RectTransform of the child layout element.
	 * @param  Axis    The axis to set the position and size along. 0 is horizontal and 1 is vertical.
	 * @parama Pos     The position from the left side or top.
	 */
    FORCEINLINE void SetChildAlongAxis(URectTransformComponent* InRect, int32 Axis, float Pos) const
    {
        if (!IsValid(InRect))
            return;

        SetChildAlongAxisWithScale(InRect, Axis, Pos, 1.0f);
    }

    /**
     * Set the position and size of a child layout element along the given axis.
     *
     * @param  InRect  The RectTransform of the child layout element.
     * @param  Axis    The axis to set the position and size along. 0 is horizontal and 1 is vertical.
     * @parama Pos     The position from the left side or top.
     */
    FORCEINLINE void SetChildAlongAxisWithScale(URectTransformComponent* InRect, int32 Axis, float Pos, float ScaleFactor) const
    {
        if (!IsValid(InRect))
            return;

        // Inlined rect.SetInsetAndSizeFromParentEdge(...) and refactored code in order to multiply desired size by scaleFactor.
        // sizeDelta must stay the same but the size used in the calculation of the position must be scaled by the scaleFactor.

        FVector2D NewAnchoredPosition = InRect->GetAnchoredPosition();
        const auto& RectSizeDelta = InRect->GetSizeDelta();
        const auto& RectPivot = InRect->GetPivot();
        NewAnchoredPosition[Axis] = (Axis == 0) ? (Pos + RectSizeDelta[Axis] * RectPivot[Axis] * ScaleFactor) : (-Pos - RectSizeDelta[Axis] * (1.0f - RectPivot[Axis]) * ScaleFactor);

        InRect->SetAnchorAndPosition(FVector2D(0, 1), FVector2D(0, 1), NewAnchoredPosition);
    }

    /**
	 * Set the position and size of a child layout element along the given axis.
	 *
	 * @param  InRect  The RectTransform of the child layout element.
	 * @param  Axis    The axis to set the position and size along. 0 is horizontal and 1 is vertical.
	 * @parama Pos     The position from the left side or top.
	 */
    FORCEINLINE void SetChildAlongAxis(URectTransformComponent* InRect, int32 Axis, float Pos, float Size) const
    {
        if (!IsValid(InRect))
            return;

        SetChildAlongAxisWithScale(InRect, Axis, Pos, Size, 1.0f);
    }

   /**
     * Set the position and size of a child layout element along the given axis.
     *
     * @param  InRect  The RectTransform of the child layout element.SetAnchorMin
     * @param  Axis    The axis to set the position and size along. 0 is horizontal and 1 is vertical.
     * @parama Pos     The position from the left side or top.
     */
    FORCEINLINE void SetChildAlongAxisWithScale(URectTransformComponent* InRect, int32 Axis, float Pos, float Size, float ScaleFactor) const
    {
        if (!IsValid(InRect))
            return;

        // Inlined rect.SetInsetAndSizeFromParentEdge(...) and refactored code in order to multiply desired size by scaleFactor.
        // sizeDelta must stay the same but the size used in the calculation of the position must be scaled by the scaleFactor.

        auto RectSizeDelta = InRect->GetSizeDelta();
        RectSizeDelta[Axis] = Size;

        FVector2D NewAnchoredPosition = InRect->GetAnchoredPosition();
        const auto& RectPivot = InRect->GetPivot();
        NewAnchoredPosition[Axis] = (Axis == 0) ? (Pos + Size * RectPivot[Axis] * ScaleFactor) : (-Pos - Size * (1.0f - RectPivot[Axis]) * ScaleFactor);
  
        InRect->SetAnchorAndSizeAndPosition(FVector2D(0, 1), FVector2D(0, 1), RectSizeDelta, NewAnchoredPosition);
    }

    FORCEINLINE bool IsRootLayoutGroup() const
    {
        const auto LayoutParent = GetAttachParent();
        if (!IsValid(LayoutParent))
            return true;

        if (Cast<ILayoutGroupInterface>(LayoutParent))
        {
            return false;
        }

        const auto UIBehaviourParent = Cast<UBehaviourComponent>(LayoutParent);
        if (IsValid(UIBehaviourParent))
        {
            const auto& AllSubComponents = UIBehaviourParent->GetAllSubComponents();
        	for (const auto& SubComponent : AllSubComponents)
        	{
        		if (IsValid(SubComponent) && SubComponent->IsActiveAndEnabled())
        		{
                    const auto LayoutGroupInterface = Cast<ILayoutGroupInterface>(SubComponent);
                    if (LayoutGroupInterface)
                        return false;
        		}
        	}
        }

        return true;
    }
	
    FORCEINLINE void SetDirty()
    {
        if (!IsActiveAndEnabled())
            return;

        if (!FCanvasUpdateRegistry::IsRebuildingLayout())
        {
            FLayoutRebuilder::MarkLayoutForRebuild(this);
        }
        else
        {
            if (!EndFrameDelegateHandle.IsValid())
            {
                EndFrameDelegateHandle = FCoreDelegates::OnEndFrame.AddUObject(this, &ULayoutGroupComponent::DelayedSetDirty);
            }
        }
    }

    FORCEINLINE void DelayedSetDirty()
    {
        FLayoutRebuilder::MarkLayoutForRebuild(this);

        FCoreDelegates::OnEndFrame.Remove(EndFrameDelegateHandle);
        EndFrameDelegateHandle.Reset();
    }

};
