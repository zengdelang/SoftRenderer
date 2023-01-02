#pragma once

#include "CoreMinimal.h"
#include "SDFFont.h"
#include "TextAnchor.h"
#include "TextFontStyle.h"
#include "TextWrapMode.h"
#include "Core/MathUtility.h"

struct UGUI_API FTextGenerationSettings
{
public:
	TWeakObjectPtr<USDFFont> Font;
	TWeakObjectPtr<UObject> TextComponent;
	
    FLinearColor Color;

    int32 FontSize;
    float ImportFontSize;
	
    float LineSpacing;
    float Kerning;
	float NonTextScale;
	float UnderlineScale;
	
    EFontStyle FontStyle;
    ETextAnchor TextAnchor;

    int32 ResizeTextMinSize;
    int32 ResizeTextMaxSize;

    EVerticalWrapMode VerticalOverflow;
    EHorizontalWrapMode HorizontalOverflow;

    FVector2D GenerationExtents;
    FVector2D Pivot;

    uint8 bRichText : 1;
    uint8 bResizeTextForBestFit : 1;
    uint8 bElipsizeEnd : 1;
	uint8 bBlendComponentColor : 1;

public:
    FTextGenerationSettings()
    {
        Font = nullptr;
    	TextComponent = nullptr;
    	
    	Color = FColor::White;
    	
        FontSize = 0;
    	ImportFontSize = 0;
    	
        LineSpacing = 0;
        Kerning = 0;
    	NonTextScale = 1;
    	UnderlineScale = 1;
    	
        FontStyle = EFontStyle::FontStyle_Normal;
        TextAnchor = ETextAnchor::TextAnchor_UpperLeft;

        ResizeTextMinSize = 0;
        ResizeTextMaxSize = 0;

        HorizontalOverflow = EHorizontalWrapMode::HorizontalWrapMode_Wrap;
        VerticalOverflow = EVerticalWrapMode::VerticalWrapMode_Truncate;
    	
        GenerationExtents = FVector2D::ZeroVector;
        Pivot = FVector2D::ZeroVector;

        bRichText = false;
        bResizeTextForBestFit = false;
        bElipsizeEnd = false;
    	bBlendComponentColor = false;
    }
	
    bool Equals(const FTextGenerationSettings& Other) const
    {
    	if (Color == Other.Color
            && FontSize == Other.FontSize
            && FMathUtility::Approximately(ImportFontSize, Other.ImportFontSize)
            && ResizeTextMinSize == Other.ResizeTextMinSize
            && ResizeTextMaxSize == Other.ResizeTextMaxSize
            && FMathUtility::Approximately(LineSpacing, Other.LineSpacing)
            && FMathUtility::Approximately(Kerning, Other.Kerning)
            && FMathUtility::Approximately(NonTextScale, Other.NonTextScale)
            && FMathUtility::Approximately(UnderlineScale,Other.UnderlineScale)
            && FontStyle == Other.FontStyle
            && bRichText == Other.bRichText
            && TextAnchor == Other.TextAnchor
            && bResizeTextForBestFit == Other.bResizeTextForBestFit
            && HorizontalOverflow == Other.HorizontalOverflow
            && VerticalOverflow == Other.VerticalOverflow
            && GenerationExtents.Equals(Other.GenerationExtents)
            && Pivot.Equals(Other.Pivot)
            && bElipsizeEnd == Other.bElipsizeEnd
            && bBlendComponentColor == Other.bBlendComponentColor)
    	{
            return Font == Other.Font && TextComponent == Other.TextComponent;
    	}
        return false;
    }
	
};
