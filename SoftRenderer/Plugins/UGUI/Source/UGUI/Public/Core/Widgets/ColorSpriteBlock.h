#pragma once

#include "CoreMinimal.h"
#include "PaperSprite.h"
#include "ColorSpriteBlock.generated.h"

USTRUCT(BlueprintType)
struct UGUI_API FColorSpriteBlock
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	FLinearColor NormalColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	UPaperSprite* NormalSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	FLinearColor HighlightedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	UPaperSprite* HighlightedSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	FLinearColor PressedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	UPaperSprite* PressedSprite;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	FLinearColor SelectedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	UPaperSprite* SelectedSprite;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	FLinearColor DisabledColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ColorSpriteBlock)
	UPaperSprite* DisabledSprite;

public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "5", UIMin = "1", UIMax = "5"), Category = ColorSpriteBlock)
	float ColorMultiplier;

public:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", UIMin = "0.0"), Category = ColorSpriteBlock)
	float FadeDuration;

public:
	FColorSpriteBlock()
	{
		NormalColor = FLinearColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
		HighlightedColor = FLinearColor(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 255 / 255.0f);
		PressedColor = FLinearColor(200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 255 / 255.0f);
		SelectedColor = FLinearColor(245 / 255.0f, 245 / 255.0f, 245 / 255.0f, 255 / 255.0f);
		DisabledColor = FLinearColor(200 / 255.0f, 200 / 255.0f, 200 / 255.0f, 128 / 255.0f);

		NormalSprite = nullptr;
		HighlightedSprite = nullptr;
		PressedSprite = nullptr;
		SelectedSprite = nullptr;
		DisabledSprite = nullptr;
		
		ColorMultiplier = 1.0f;
		FadeDuration = 0.1f;
	}
	
};
