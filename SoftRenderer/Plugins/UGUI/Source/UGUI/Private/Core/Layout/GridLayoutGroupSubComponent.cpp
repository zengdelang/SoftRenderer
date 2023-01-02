#include "Core/Layout/GridLayoutGroupSubComponent.h"

/////////////////////////////////////////////////////
// UGridLayoutGroupSubComponent

UGridLayoutGroupSubComponent::UGridLayoutGroupSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), StartCorner(), StartAxis(), Constraint()
{
    CellSize = FVector2D(100, 100);
    ConstraintCount = 2;
}

void UGridLayoutGroupSubComponent::CalculateLayoutInputHorizontal()
{
    Super::CalculateLayoutInputHorizontal();

    int32 MinColumns = 0;
    int32 PreferredColumns = 0;
    if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedColumnCount)
    {
        MinColumns = PreferredColumns = ConstraintCount;
    }
    else if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedRowCount)
    {
        MinColumns = PreferredColumns = FMath::CeilToInt(RectChildren.Num() / static_cast<float>(ConstraintCount) - 0.001f);
    }
    else
    {
        MinColumns = 1;
        PreferredColumns = FMath::CeilToInt(FMath::Sqrt(RectChildren.Num()));
    }

    const float Horizontal = Padding.GetTotalSpaceAlong<Orient_Horizontal>();
    SetLayoutInputForAxis(
        Horizontal + (CellSize.X + Spacing.X) * MinColumns - Spacing.X,
        Horizontal + (CellSize.X + Spacing.X) * PreferredColumns - Spacing.X,
        -1, 0);
}

void UGridLayoutGroupSubComponent::CalculateLayoutInputVertical()
{
    if (!IsValid(AttachTransform))
        return;
    
    int32 MinRows = 0;
    if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedColumnCount)
    {
        MinRows = FMath::CeilToInt(RectChildren.Num() / static_cast<float>(ConstraintCount) - 0.001f);
    }
    else if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedRowCount)
    {
        MinRows = ConstraintCount;
    }
    else
    {
        const float Width = AttachTransform->GetRect().Width;
        const float Horizontal = Padding.GetTotalSpaceAlong<Orient_Horizontal>();
        const int32 CellCountX = FMath::Max(1, FMath::FloorToInt((Width - Horizontal + Spacing.X + 0.001f) / (CellSize.X + Spacing.X)));
        MinRows = FMath::CeilToInt(RectChildren.Num() / static_cast<float>(CellCountX));
    }

    const float Vertical = Padding.GetTotalSpaceAlong<Orient_Vertical>();
    const float minSpace = Vertical + (CellSize.Y + Spacing.Y) * MinRows - Spacing.Y;
    SetLayoutInputForAxis(minSpace, minSpace, -1, 1);
}

void UGridLayoutGroupSubComponent::SetLayoutHorizontal()
{
    SetCellsAlongAxis(0);
}

void UGridLayoutGroupSubComponent::SetLayoutVertical()
{
    SetCellsAlongAxis(1);
}

void UGridLayoutGroupSubComponent::SetCellsAlongAxis(int32 Axis)
{
    if (!IsValid(AttachTransform))
        return;
    
    // Normally a Layout Controller should only set horizontal values when invoked for the horizontal axis
    // and only vertical values when invoked for the vertical axis.
    // However, in this case we set both the horizontal and vertical position when invoked for the vertical axis.
    // Since we only set the horizontal position and not the size, it shouldn't affect children's layout,
    // and thus shouldn't break the rule that all horizontal layout must be calculated before all vertical layout.
    
    if (Axis == 0)
    {
        // Only set the sizes when invoked for horizontal axis, not the positions.
        for (int32 Index = 0, Count = RectChildren.Num(); Index < Count; ++Index)
        {
            const auto RectChild = RectChildren[Index];
            if (IsValid(RectChild))
            {
                RectChild->SetAnchorAndSize(FVector2D(0, 1), FVector2D(0, 1), CellSize);
            }
        }
        return;
    }

    const float Width = AttachTransform->GetRect().Width;
    const float Height = AttachTransform->GetRect().Height;

    int32 CellCountX = 1;
    int32 CellCountY = 1;
    if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedColumnCount)
    {
        CellCountX = ConstraintCount;

        if (RectChildren.Num() > CellCountX)
            CellCountY = RectChildren.Num() / CellCountX + (RectChildren.Num() % CellCountX > 0 ? 1 : 0);
    }
    else if (Constraint == EGridLayoutConstraint::GridLayoutConstraint_FixedRowCount)
    {
        CellCountY = ConstraintCount;

        if (RectChildren.Num() > CellCountY)
            CellCountX = RectChildren.Num() / CellCountY + (RectChildren.Num() % CellCountY > 0 ? 1 : 0);
    }
    else
    {
        if (CellSize.X + Spacing.X <= 0)
        {
            CellCountX = MAX_int32;
        }
        else
        {
            const float Horizontal = Padding.GetTotalSpaceAlong<Orient_Horizontal>();
            CellCountX = FMath::Max(1, FMath::FloorToInt((Width - Horizontal + Spacing.X + 0.001f) / (CellSize.X + Spacing.X)));
        }

        if (CellSize.Y + Spacing.Y <= 0)
        {
           CellCountY = MAX_int32;
        }
        else
        {
            const float Vertical = Padding.GetTotalSpaceAlong<Orient_Vertical>();
            CellCountY = FMath::Max(1, FMath::FloorToInt((Height - Vertical + Spacing.Y + 0.001f) / (CellSize.Y + Spacing.Y)));
        }
    }

    const int32 CornerX = static_cast<int32>(StartCorner) % 2;
    const int32 CornerY = static_cast<int32>(StartCorner) / 2;

    int32 CellsPerMainAxis, ActualCellCountX, ActualCellCountY;
    if (StartAxis == EGridLayoutAxis::GridLayoutAxis_Horizontal)
    {
        CellsPerMainAxis = CellCountX;
        ActualCellCountX = FMath::Clamp(CellCountX, 1, RectChildren.Num());
        ActualCellCountY = FMath::Clamp(CellCountY, 1, FMath::CeilToInt(RectChildren.Num() / (float)CellsPerMainAxis));
    }
    else
    {
        CellsPerMainAxis = CellCountY;
        ActualCellCountY = FMath::Clamp(CellCountY, 1, RectChildren.Num());
        ActualCellCountX = FMath::Clamp(CellCountX, 1, FMath::CeilToInt(RectChildren.Num() / (float)CellsPerMainAxis));
    }

    const FVector2D RequiredSpace = FVector2D(
        ActualCellCountX * CellSize.X + (ActualCellCountX - 1) * Spacing.X,
        ActualCellCountY * CellSize.Y + (ActualCellCountY - 1) * Spacing.Y
    );
    const FVector2D StartOffset = FVector2D(
        GetStartOffset(0, RequiredSpace.X),
        GetStartOffset(1, RequiredSpace.Y)
    );

    for (int32 Index = 0, Count = RectChildren.Num(); Index < Count; ++Index)
    {
        int32 PositionX;
        int32 PositionY;
        if (StartAxis == EGridLayoutAxis::GridLayoutAxis_Horizontal)
        {
            PositionX = Index % CellsPerMainAxis;
            PositionY = Index / CellsPerMainAxis;
        }
        else
        {
            PositionX = Index / CellsPerMainAxis;
            PositionY = Index % CellsPerMainAxis;
        }

        if (CornerX == 1)
            PositionX = ActualCellCountX - 1 - PositionX;
        if (CornerY == 1)
            PositionY = ActualCellCountY - 1 - PositionY;

        SetChildAlongAxis(RectChildren[Index], 0, StartOffset.X + (CellSize[0] + Spacing[0]) * PositionX, CellSize[0]);
        SetChildAlongAxis(RectChildren[Index], 1, StartOffset.Y + (CellSize[1] + Spacing[1]) * PositionY, CellSize[1]);
    }
}

/////////////////////////////////////////////////////
