#pragma once

#include "CoreMinimal.h"
#include "Widgets/UIGlitchDefinitions.h"
#include "UICommonDefinitions.generated.h"

UENUM(BlueprintType)
enum class ECanvasRenderMode : uint8
{
	CanvasRenderMode_ScreenSpaceOverlay UMETA(DisplayName = ScreenSpaceOverlay),
	CanvasRenderMode_ScreenSpaceFree UMETA(DisplayName = ScreenSpaceFree),
	CanvasRenderMode_WorldSpace UMETA(DisplayName = WorldSpace),
	CanvasRenderMode_MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECanvasRenderTargetMode : uint8
{
	BackBuffer UMETA(DisplayName = BackBuffer),
	ExternalTarget UMETA(DisplayName = ExternalTarget),
};

USTRUCT(BlueprintType)
struct FUVRect
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV Rect")
    float X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV Rect")
    float Y;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV Rect")
    float W;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV Rect")
    float H;

public:
    FUVRect()
        : X(0.0f)
        , Y(0.0f)
        , W(0.0f)
        , H(0.0f)
    { }

    FUVRect(float InX, float InY, float InW, float InH)
        : X(InX)
        , Y(InY)
        , W(InW)
        , H(InH)
    { }
	
public:
    bool operator==(const FUVRect& Other) const
    {
        return (X == Other.X) && (Y == Other.Y) && (W == Other.W) && (H == Other.H);
    }
	
    bool operator!=(const FUVRect& Other) const
    {
        return X != Other.X || Y != Other.Y || W != Other.W || H != Other.H;
    }
	
};

UENUM()
enum class EUIGraphicType : uint8
{
	UIMesh = 0,
	PostProcessBlur = 1,
	FX = 2,
	StaticMesh = 3,
	PostProcessGlitch = 4,
};

/**
 * A rectangle defined by upper-left and lower-right corners.
 *
 * Assumes a "screen-like" coordinate system where the origin is in the top-left, with the Y-axis going down.
 * Functions like "contains" etc will not work with other conventions.
 *
 *      +---------> X
 *      |
 *      |    (Left,Top)
 *      |            o----o
 *      |            |    |
 *      |            o----o
 *      |                (Right, Bottom)
 *      \/
 *      Y
 */
class UGUI_API FUIRect
{
public:
	float Left;
	float Top;
	float Right;
	float Bottom;

	explicit FUIRect(float InLeft = -1, float InTop = -1, float InRight = -1, float InBottom = -1)
		: Left(InLeft)
		, Top(InTop)
		, Right(InRight)
		, Bottom(InBottom)
	{ }

	FUIRect(const FVector2D& InStartPos, const FVector2D& InEndPos)
		: Left(InStartPos.X)
		, Top(InStartPos.Y)
		, Right(InEndPos.X)
		, Bottom(InEndPos.Y)
	{ }

public:
	/**
	 * Returns the size of the rectangle.
	 *
	 * @return The size as a vector.
	 */
	FORCEINLINE FVector2D GetSize() const
	{
		return FVector2D(Right - Left, Bottom - Top);
	}

};

class FUIGraphicData
{
public:
	virtual FUIGraphicData* CopyGraphicData() = 0;
	virtual ~FUIGraphicData() {};

public:
	virtual void UpdateGraphicData() {}

};

class FUIBlurGraphicData : public FUIGraphicData
{
public:
	int32 KernelSize;
	int32 DownsampleAmount;

	float Strength;
	float Alpha;

	float BlurStrength;
	int32 BlurRadius;

	uint8 bApplyAlphaToBlur : 1;
	uint8 bOverrideAutoRadiusCalculation : 1;
	uint8 bComputeEffectiveKernelSize : 1;

public:
	FUIBlurGraphicData()
	{
		KernelSize = 3;
		DownsampleAmount = 0;

		Strength = 0;
		Alpha = 1;

		BlurStrength = 0;
		BlurRadius = 0;

		bApplyAlphaToBlur = true;
		bOverrideAutoRadiusCalculation = false;
		bComputeEffectiveKernelSize = true;
	}
	
	virtual ~FUIBlurGraphicData() override
	{

	}

public:
	virtual FUIGraphicData* CopyGraphicData() override
	{
		FUIBlurGraphicData* NewData = new FUIBlurGraphicData();
		NewData->KernelSize = KernelSize;
		NewData->DownsampleAmount = DownsampleAmount;
		NewData->Strength = Strength;
		NewData->Alpha = Alpha;
		NewData->BlurStrength = BlurStrength;
		NewData->BlurRadius = BlurRadius;
		NewData->bApplyAlphaToBlur = bApplyAlphaToBlur;
		NewData->bOverrideAutoRadiusCalculation = bOverrideAutoRadiusCalculation;
		NewData->bComputeEffectiveKernelSize = bComputeEffectiveKernelSize;
		return NewData;
	}

	virtual void UpdateGraphicData() override
	{
		if (bComputeEffectiveKernelSize)
		{
			bComputeEffectiveKernelSize = false;

			Strength = BlurStrength * (bApplyAlphaToBlur ? Alpha : 1.f);
			ComputeEffectiveKernelSize(Strength, KernelSize, DownsampleAmount);

			if (DownsampleAmount > 0)
			{
				Strength /= DownsampleAmount;
			}

			Strength = FMath::Max(0.5f, Strength);
		}
	}

	void ComputeEffectiveKernelSize(float InStrength, int32& OutKernelSize, int32& OutDownsampleAmount) const
	{
		// If the radius isn't set, auto-compute it based on the strength
		OutKernelSize = BlurRadius;

		if (!bOverrideAutoRadiusCalculation)
		{
			OutKernelSize = FMath::RoundToInt(InStrength * 3.f);
		}

		// Downsample if needed
		if (OutKernelSize > 9)
		{
			OutDownsampleAmount = OutKernelSize >= 64 ? 4 : 2;
			OutKernelSize /= OutDownsampleAmount;
		}

		// Kernel sizes must be odd
		if (OutKernelSize % 2 == 0)
		{
			++OutKernelSize;
		}

		static int32 MaxKernelSize = 255;
		OutKernelSize = FMath::Clamp(OutKernelSize, 3, MaxKernelSize);
	}
};

class FUIGlitchGraphicData : public FUIGraphicData
{
public:
	uint8 bUseGlitch : 1;
	EUIGlitchType Method;
	float Strength;

	int32 DownSampleAmount;

	FUIGlitchAnalogNoiseAndRGBSplitSet UIGlitchAnalogNoiseAndRGBSplitSet;
	FUIGlitchImageBlockSet UIGlitchImageBlockSet;

public:
	FUIGlitchGraphicData()
	{
		bUseGlitch = false;
		Method = EUIGlitchType::UIGlitchType_None;

		Strength = 1;
		DownSampleAmount = 1;
	}

	virtual FUIGraphicData* CopyGraphicData() override
	{
		FUIGlitchGraphicData* NewData = new FUIGlitchGraphicData();
		NewData->bUseGlitch = bUseGlitch;
		NewData->Method = Method;
		NewData->Strength = Strength;

		NewData->DownSampleAmount = DownSampleAmount;

		NewData->UIGlitchAnalogNoiseAndRGBSplitSet = UIGlitchAnalogNoiseAndRGBSplitSet;
		NewData->UIGlitchImageBlockSet = UIGlitchImageBlockSet;

		return NewData;
	}

	virtual void UpdateGraphicData() override
	{

	}
};
