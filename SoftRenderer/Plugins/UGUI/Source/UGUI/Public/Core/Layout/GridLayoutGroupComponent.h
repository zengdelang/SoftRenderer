#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/LayoutGroupComponent.h"
#include "GridLayoutGroupComponent.generated.h"

/**
 *  Which corner is the starting corner for the grid.
 */
UENUM(BlueprintType)
enum class EGridLayoutCorner : uint8
{
    /**
     * Upper Left corner.
     */
    GridLayoutCorner_UpperLeft UMETA(DisplayName = "Upper Left"),

    /**
	 * Upper Right corner.
	 */
    GridLayoutCorner_UpperRight UMETA(DisplayName = "Upper Right"),

    /**
     * Lower Left corner.
     */
    GridLayoutCorner_LowerLeft UMETA(DisplayName = "Lower Left"),

    /**
     * Lower Right corner.
     */
    GridLayoutCorner_LowerRight UMETA(DisplayName = "Lower Right"),	
};

/** 
 * The grid axis we are looking at.
 *
 * As the storage is a [][] we make access easier by passing a axis.
 */
UENUM(BlueprintType)
enum class EGridLayoutAxis : uint8
{
    /**
     * Horizontal axis
     */
    GridLayoutAxis_Horizontal UMETA(DisplayName = "Horizontal"),

    /**
     * Vertical axis.
     */
     GridLayoutAxis_Vertical UMETA(DisplayName = "Vertical"),
};

/**
 * Constraint type on either the number of columns or rows.
 */
UENUM(BlueprintType)
enum class EGridLayoutConstraint : uint8
{
    /**
     * Don't constrain the number of rows or columns.
     */
    GridLayoutConstraint_Flexible UMETA(DisplayName = "Flexible"),

    /**
     * Constrain the number of columns to a specified number.
     */
    GridLayoutConstraint_FixedColumnCount UMETA(DisplayName = "Fixed Column Count"),

    /**
     * Constraint the number of rows to a specified number.
     */
    GridLayoutConstraint_FixedRowCount UMETA(DisplayName = "Fixed Row Count"),
};

/**
 * Layout class to arrange children elements in a grid format.
 *
 * The GridLayoutGroup component is used to layout child layout elements in a uniform grid where all cells have the same size. The size and the spacing between cells is controlled by the GridLayoutGroup itself. The children have no influence on their sizes.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UGridLayoutGroupComponent : public ULayoutGroupComponent
{
	GENERATED_UCLASS_BODY()

protected:
	/**
	 * Which corner should the first cell be placed in?
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    EGridLayoutCorner StartCorner;

	/**
	 * Which axis should cells be placed along first
	 *
	 * When startAxis is set to horizontal, an entire row will be filled out before proceeding to the next row. When set to vertical, an entire column will be filled out before proceeding to the next column.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    EGridLayoutAxis StartAxis;

	/**
	 * The size to use for each cell in the grid.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    FVector2D CellSize;

	/**
	 * The spacing to use between layout elements in the grid on both axises.
	 *
	 * Specifying a constraint can make the GridLayoutGroup work better in conjunction with a [[ContentSizeFitter]] component. When GridLayoutGroup is used on a RectTransform with a manually specified size, there's no need to specify a constraint.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    FVector2D Spacing;

	/**
	 * Which constraint to use for the GridLayoutGroup.
	 */
    UPROPERTY(EditAnywhere, Category = Layout)
    EGridLayoutConstraint Constraint;

	/**
	 * How many cells there should be along the constrained axis.
	 */
    UPROPERTY(EditAnywhere, Category = Layout, meta = (ClampMin = "1", UIMin = "1"))
    int32 ConstraintCount;

public:
    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    EGridLayoutCorner GetStartCorner() const
    {
        return StartCorner;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetStartCorner(EGridLayoutCorner InStartCorner)
    {
        if (StartCorner != InStartCorner)
        {
            StartCorner = InStartCorner;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    EGridLayoutAxis GetStartAxis() const
    {
        return StartAxis;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetStartAxis(EGridLayoutAxis InStartAxis)
    {
        if (StartAxis != InStartAxis)
        {
            StartAxis = InStartAxis;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    FVector2D GetCellSize() const
    {
        return CellSize;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetCellSize(FVector2D InCellSize)
    {
        if (CellSize != InCellSize)
        {
            CellSize = InCellSize;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    FVector2D GetSpacing() const
    {
        return Spacing;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetSpacing(FVector2D InSpacing)
    {
        if (Spacing != InSpacing)
        {
            Spacing = InSpacing;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    EGridLayoutConstraint GetConstraint() const
    {
        return Constraint;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetConstraint(EGridLayoutConstraint InConstraint)
    {
        if (Constraint != InConstraint)
        {
            Constraint = InConstraint;
            SetDirty();
        }
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    int32 GetConstraintCount() const
    {
        return ConstraintCount;
    }

    UFUNCTION(BlueprintCallable, Category = GridLayoutGroup)
    void SetConstraintCount(int32 InConstraintCount)
    {
        InConstraintCount = FMath::Max(1, InConstraintCount);
        if (ConstraintCount != InConstraintCount)
        {
            ConstraintCount = InConstraintCount;
            SetDirty();
        }
    }

public:
    //~ Begin ILayoutElementInterface Interface
    virtual void CalculateLayoutInputHorizontal() override;
    virtual void CalculateLayoutInputVertical() override;
    //~ End ILayoutElementInterface Interface

public:
    //~ Begin ILayoutGroupInterface Interface
    virtual void SetLayoutHorizontal() override;
    virtual void SetLayoutVertical() override;
    //~ End ILayoutGroupInterface Interface

private:
    void SetCellsAlongAxis(int32 Axis);
	
};
