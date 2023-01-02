#pragma once

#include "CoreMinimal.h"
#include "UIMargin.generated.h"

/**
 * Describes the space around a Widget.
 */
USTRUCT(BlueprintType)
struct FUIMargin
{
    GENERATED_USTRUCT_BODY()

    /** Holds the margin to the left. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Margin")
    float Left;

    /** Holds the margin to the top. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Margin")
    float Top;

    /** Holds the margin to the right. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Margin")
    float Right;

    /** Holds the margin to the bottom. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Margin")
    float Bottom;

public:
    /**
     * Default constructor.
     *
     * The default margin size is zero on all four sides..
     */
    FUIMargin()
        : Left(0.0f)
        , Top(0.0f)
        , Right(0.0f)
        , Bottom(0.0f)
    { }

    /** Construct a Margin with uniform space on all sides */
    FUIMargin(float UniformMargin)
        : Left(UniformMargin)
        , Top(UniformMargin)
        , Right(UniformMargin)
        , Bottom(UniformMargin)
    { }

    /** Construct a Margin where Horizontal describes Left and Right spacing while Vertical describes Top and Bottom spacing */
    FUIMargin(float Horizontal, float Vertical)
        : Left(Horizontal)
        , Top(Vertical)
        , Right(Horizontal)
        , Bottom(Vertical)
    { }

    /** Construct a Margin where Horizontal describes Left and Right spacing while Vertical describes Top and Bottom spacing */
    FUIMargin(const FVector2D& InVector)
        : Left(InVector.X)
        , Top(InVector.Y)
        , Right(InVector.X)
        , Bottom(InVector.Y)
    { }

    /** Construct a Margin where the spacing on each side is individually specified. */
    FUIMargin(float InLeft, float InTop, float InRight, float InBottom)
        : Left(InLeft)
        , Top(InTop)
        , Right(InRight)
        , Bottom(InBottom)
    { }

    /** Construct a Margin where the margins are coming from a FVector4 */
    FUIMargin(const FVector4& InVector)
        : Left(InVector.X)
        , Top(InVector.Y)
        , Right(InVector.Z)
        , Bottom(InVector.W)
    { }

public:
	/**
	 * Multiply the margin by a scalar.
	 *
	 * @param Scale How much to scale the margin.
	 * @return An FUIMargin where each value is scaled by Scale.
	 */
	FUIMargin operator*(float Scale) const
	{
		return FUIMargin(Left * Scale, Top * Scale, Right * Scale, Bottom * Scale);
	}

	/**
	 * Multiply the margin by another margin functioning as the scale.
	 *
	 * @param InScale How much to scale the margin.
	 * @return An FUIMargin where each value is scaled by Scale.
	 */
	FUIMargin operator*(const FUIMargin& InScale) const
	{
		return FUIMargin(Left * InScale.Left, Top * InScale.Top, Right * InScale.Right, Bottom * InScale.Bottom);
	}

	/**
	 * Adds another margin to this margin.
	 *
	 * @param Other The margin to add.
	 * @return A margin that represents this margin plus the other margin.
	 */
	FUIMargin operator+(const FUIMargin& InDelta) const
	{
		return FUIMargin(Left + InDelta.Left, Top + InDelta.Top, Right + InDelta.Right, Bottom + InDelta.Bottom);
	}

	/**
	 * Subtracts another margin from this margin.
	 *
	 * @param Other The margin to subtract.
	 * @return A margin that represents this margin minues the other margin.
	 */
	FUIMargin operator-(const FUIMargin& Other) const
	{
		return FUIMargin(Left - Other.Left, Top - Other.Top, Right - Other.Right, Bottom - Other.Bottom);
	}

	/**
	 * Compares this margin with another for equality.
	 *
	 * @param Other The other margin.
	 * @return true if the two margins are equal, false otherwise.
	 */
	bool operator==(const FUIMargin& Other) const
	{
		return (Left == Other.Left) && (Right == Other.Right) && (Top == Other.Top) && (Bottom == Other.Bottom);
	}

	/**
	 * Compares this margin with another for inequality.
	 *
	 * @param Other The other margin.
	 * @return true if the two margins are not equal, false otherwise.
	 */
	bool operator!=(const FUIMargin& Other) const
	{
		return Left != Other.Left || Right != Other.Right || Top != Other.Top || Bottom != Other.Bottom;
	}

public:
	/**
	 * Gets the offset for the top left.
	 *
	 * @return The top left offset.
	 */
	FVector2D GetTopLeft() const
	{
		return FVector2D(Left, Top);
	}

	/**
	 * Gets the margin's total size.
	 *
	 * @return Cumulative margin size.
	 */
	FVector2D GetDesiredSize() const
	{
		return FVector2D(Left + Right, Top + Bottom);
	}
	
    /**
	 * Gets the total horizontal or vertical margin.
	 *
	 * @return Cumulative horizontal margin.
	 */
    template<EOrientation Orientation>
    float GetTotalSpaceAlong() const
    {
        return 0.0f;
    }
	
};

template <> struct TIsPODType<FUIMargin> { enum { Value = true }; };

template<>
inline float FUIMargin::GetTotalSpaceAlong<Orient_Horizontal>() const { return Left + Right; }

template<>
inline float FUIMargin::GetTotalSpaceAlong<Orient_Vertical>() const { return Top + Bottom; }
