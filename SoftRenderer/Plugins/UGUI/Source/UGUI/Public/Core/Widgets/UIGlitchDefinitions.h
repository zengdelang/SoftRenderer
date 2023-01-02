#pragma once

#include "CoreMinimal.h"
#include "UIGlitchDefinitions.generated.h"

UENUM(BlueprintType)
enum class EUIGlitchType : uint8
{
	// UIGlitchType_AnalogNoiseAndRGBSplit

	UIGlitchType_None UMETA(DisplayName = "None"),

	UIGlitchType_AnalogNoiseAndRGBSplit UMETA(DisplayName = "Analog Noise and RGB Split"),

    UIGlitchType_ImageBlock UMETA(DisplayName = "Image Block"),

	// UIGlitchType_AnalogNoise UMETA(DisplayName = "Analog Noise"),
	
};

USTRUCT(BlueprintType)
struct UGUI_API FUIGlitchAnalogNoiseAndRGBSplitSet
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.0f, ClampMax = 1.0f), Category = "UI Glitch")
	float AnalogNoiseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch")
	float AnalogNoiseTexScaleX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch")
	float AnalogNoiseTexScaleY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, ClampMax = 1.0f), Category = "UI Glitch")
	float AnalogNoiseFading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 1024), Category = "UI Glitch")
	float RGBSplitSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch")
	float RGBSplitAmplitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch")
	float RGBSplitAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch")
	UTexture* MaskTexture;

public:
	FUIGlitchAnalogNoiseAndRGBSplitSet()
		: AnalogNoiseSpeed(0.1f)
		, AnalogNoiseTexScaleX(0.02f)
		, AnalogNoiseTexScaleY(10.0f)
		, AnalogNoiseFading(0.5f)
		, RGBSplitSpeed(1.0f)
		, RGBSplitAmplitude(3.1f)
		, RGBSplitAmount(0.5f)

	{
	}
	virtual ~FUIGlitchAnalogNoiseAndRGBSplitSet() {}

	FUIGlitchAnalogNoiseAndRGBSplitSet& operator= (const FUIGlitchAnalogNoiseAndRGBSplitSet& Other)
	{
		AnalogNoiseSpeed = Other.AnalogNoiseSpeed;
		AnalogNoiseTexScaleX = Other.AnalogNoiseTexScaleX;
		AnalogNoiseTexScaleY = Other.AnalogNoiseTexScaleY;
		AnalogNoiseFading = Other.AnalogNoiseFading;

		RGBSplitSpeed = Other.RGBSplitSpeed;
		RGBSplitAmplitude = Other.RGBSplitAmplitude;
		RGBSplitAmount = Other.RGBSplitAmount;

		MaskTexture = Other.MaskTexture;

		return *this;
	}

};

USTRUCT(BlueprintType)
struct UGUI_API FUIGlitchImageBlockSet
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 1024), Category = "UI Glitch Image Block")
	float GlitchSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0), Category = "UI Glitch Image Block")
	int32 BlockSizeX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0), Category = "UI Glitch Image Block")
	int32 BlockSizeY;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch Image Block")
	float OffsetScaleX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI Glitch Image Block")
	float OffsetScaleY;

public:
	FUIGlitchImageBlockSet()
		: GlitchSpeed(10.0f)
		, BlockSizeX(16)
		, BlockSizeY(9)
		, OffsetScaleX(0.07f)
		, OffsetScaleY(0.05f)
	{
	}

	virtual ~FUIGlitchImageBlockSet() {}

	FUIGlitchImageBlockSet& operator= (const FUIGlitchImageBlockSet& Other)
	{
		GlitchSpeed = Other.GlitchSpeed;
		BlockSizeX = Other.BlockSizeX;
		BlockSizeY = Other.BlockSizeY;
		OffsetScaleX = Other.OffsetScaleX;
		OffsetScaleY = Other.OffsetScaleY;
		return *this;
	}
};


// USTRUCT(BlueprintType)
// struct UGUI_API FUIGlitchAnalogNoiseSetting
// {
// 	GENERATED_USTRUCT_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
// 	float GlitchSpeed;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	float TexScaleX;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	float TexScaleY;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
// 	float Fading;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite)
// 	UTexture* MaskTexture;
//
// public:
// 	FUIGlitchAnalogNoiseSetting()
// 		: GlitchSpeed(0.1f)
// 		, TexScaleX(0.02f)
// 		, TexScaleY(10.0f)
// 		, Fading(0.5f)
// 	{
// 	}
//
// 	virtual ~FUIGlitchAnalogNoiseSetting() {}
//
// 	FUIGlitchAnalogNoiseSetting& operator= (const FUIGlitchAnalogNoiseSetting& Other)
// 	{
// 		GlitchSpeed = Other.GlitchSpeed;
// 		TexScaleX = Other.TexScaleX;
// 		TexScaleY = Other.TexScaleY;
// 		Fading = Other.Fading;
// 		MaskTexture = Other.MaskTexture;
// 		return *this;
// 	}
// };