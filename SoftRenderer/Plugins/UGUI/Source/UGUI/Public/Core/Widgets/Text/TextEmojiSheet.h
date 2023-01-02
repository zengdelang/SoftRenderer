#pragma once

#include "CoreMinimal.h"
#include "PaperSprite.h"
#include "TextEmojiSheet.generated.h"

USTRUCT(Blueprintable)
struct FTextEmoji
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, Category = TextEmoji)
    float Width;

	UPROPERTY(EditAnywhere, Category = TextEmoji)
    float Height;

	UPROPERTY(EditAnywhere, Category = TextEmoji)
    float PaddingLeft;

	UPROPERTY(EditAnywhere, Category = TextEmoji)
    float PaddingRight;

    UPROPERTY(EditAnywhere, Category = TextEmoji)
    float PaddingTop;

    UPROPERTY(EditAnywhere, Category = TextEmoji)
    float PaddingBottom;

    UPROPERTY(EditAnywhere, Category = TextEmoji, meta = (DisplayName = "Scale"))
    float ScaleValue;

    UPROPERTY(EditAnywhere, Category = TextEmoji)
    FLinearColor Color;

    UPROPERTY(EditAnywhere, Category = TextEmoji)
    uint8 bScaleByFontSize : 1;
    
    UPROPERTY(EditAnywhere, Category = TextEmoji)
    uint8 bAnimation : 1;

    UPROPERTY(EditAnywhere, Category = TextEmoji, meta = (ClampMin = "1", UIMin = "1"))
    int32 FrameRate;

    UPROPERTY(EditAnywhere, Category = TextEmoji, meta=(DisplayThumbnail="true", DisplayName="Source Image", AllowedClasses="Sprite2D,SlateTextureAtlasInterface"))
    TArray<UObject*> Sprites;

public:
    FTextEmoji()
    {
        Width = 100;
        Height = 100;
        PaddingLeft = 0;
        PaddingRight = 0;
        PaddingTop = 0;
        PaddingBottom = 0;
        ScaleValue = 1;
        Color = FLinearColor::White;
        bScaleByFontSize = true;
        bAnimation = false;
        FrameRate = 30;
    }

    float GetScale(int32 FontSize, float Scale) const
    {
        if (bScaleByFontSize)
        {
            float FinalHeight = Height;
            if (FMath::IsNearlyZero(Height))
            {
                FinalHeight = 0.01;
            }
            return FontSize / FinalHeight * FMath::Max(0.01f, Scale) * ScaleValue;
        }
        return 1;
    }
    
    float GetWidth(float Scale) const
    {
        const float FinalWidth = Width + PaddingLeft + PaddingRight;
        if (bScaleByFontSize)
        {
            return FinalWidth * Scale;
        }
        return FinalWidth;
    }
    
    float GetHeight(float Scale) const
    {
        const float FinalHeight = Height + PaddingTop + PaddingBottom;
        if (bScaleByFontSize)
        {
            return FinalHeight * Scale;
        }
        return FinalHeight;
    }
};

UCLASS(BlueprintType)
class UGUI_API UTextEmojiSheet : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = TextEmojiSheet)
    TMap<FName, FTextEmoji> TextEmojis;
	
};
