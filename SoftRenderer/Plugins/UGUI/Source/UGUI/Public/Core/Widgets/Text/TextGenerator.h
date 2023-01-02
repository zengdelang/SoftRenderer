#pragma once

#include "CoreMinimal.h"
#include "TextGenerationSettings.h"
#include "Core/Renderer/UIVertex.h"

struct FUICharInfo
{
    FVector2D CursorPos;
    float CharWidth;
	 
public:
    FUICharInfo()
    {
        CursorPos = FVector2D::ZeroVector;
        CharWidth = 0;
    }
};

struct FUILineInfo
{
    int32 StartCharIdx;
    float Height;
    float TopY;

public:
    FUILineInfo()
    {
        StartCharIdx = 0;
        Height = 0;
        TopY = 0;
    }
};

struct FEmojiRegion
{
    FName EmojiSymbol;
    FVector BottomLeft;
    FVector TopRight;

	uint8 bVisible : 1;

public:
    FEmojiRegion()
    {
        EmojiSymbol = TEXT("");
        BottomLeft = FVector::ZeroVector;
        TopRight = FVector::ZeroVector;
    	bVisible = false;
    }
};

struct FWidgetRegion
{
    FName WidgetSymbol;
    FVector BottomLeft;
    FVector TopRight;

public:
    FWidgetRegion()
    {
        WidgetSymbol = TEXT("");
        BottomLeft = FVector::ZeroVector;
        TopRight = FVector::ZeroVector;
    }
};

struct FHypertextRegion
{
    FString Hypertext;
    int32 HypertextIndex;
    FVector BottomLeft;
    FVector TopRight;

public:
    FHypertextRegion()
    {
        Hypertext = TEXT("");
        HypertextIndex = 0;
        BottomLeft = FVector::ZeroVector;
        TopRight = FVector::ZeroVector;
    }
};

class UGUI_API FTextGenerator
{	
public:
	FTextGenerator();

public:
    void Invalidate()
    {
        bHasGenerated = false;
    }

    float GetPreferredWidth(FString Text, FTextGenerationSettings Settings, const UObject* TextObj);
    float GetPreferredHeight(FString Text, FTextGenerationSettings Settings, const UObject* TextObj);

private:
    void GetPreferredSize(FString Text, FTextGenerationSettings Settings, const UObject* TextObj);

public:
    bool Populate(FString Text, FTextGenerationSettings Settings, const UObject* TextObj);
	
private:
    static bool IsWhitespaceWithNonBreakingSpace(TCHAR Char)
    {
        return FChar::IsWhitespace(Char) || Char == 160;
    }

    static bool IsWhitespaceWithoutNonBreakingSpace(TCHAR Char)
    {
        return FChar::IsWhitespace(Char) && Char != 160;
    }
    
	static bool ValidatedSettings(FTextGenerationSettings& Settings, const UObject* TextObj)
	{
        if (!Settings.Font.IsValid())
            return false;

	    if (Settings.ImportFontSize <= 1)
	    {
	        Settings.ImportFontSize = 1;
	    }
	    
        return true;
    }

    static bool IsLeftPivot(ETextAnchor Anchor)
    {
        return Anchor == ETextAnchor::TextAnchor_LowerLeft ||
               Anchor == ETextAnchor::TextAnchor_MiddleLeft || 
               Anchor == ETextAnchor::TextAnchor_UpperLeft;
    }
	
    static FVector2D GetTextAnchorPivot(ETextAnchor Anchor)
    {
        switch (Anchor)
        {
        case ETextAnchor::TextAnchor_LowerLeft:
            return FVector2D(0, 0);
        case ETextAnchor::TextAnchor_LowerCenter:
            return FVector2D(0.5f, 0);
        case ETextAnchor::TextAnchor_LowerRight:
            return FVector2D(1, 0);
        case ETextAnchor::TextAnchor_MiddleLeft:
            return FVector2D(0, 0.5f);
        case ETextAnchor::TextAnchor_MiddleCenter:
            return FVector2D(0.5f, 0.5f);
        case ETextAnchor::TextAnchor_MiddleRight:
            return FVector2D(1, 0.5f);
        case ETextAnchor::TextAnchor_UpperLeft:
            return FVector2D(0, 1);
        case ETextAnchor::TextAnchor_UpperCenter:
            return FVector2D(0.5f, 1);
        case ETextAnchor::TextAnchor_UpperRight:
            return FVector2D(1, 1);
        default:
            return FVector2D::ZeroVector;
        }
    }

	EFontStyle GetGlyphFontStyle(bool bItalic, bool bBold) const
    {
        EFontStyle FontStyle = LastSettings.FontStyle;
        if (bItalic && bBold)
        {
            FontStyle = EFontStyle::FontStyle_BoldAndItalic;
        }
        else
        {
            if (bItalic)
            {
                FontStyle = EFontStyle::FontStyle_Italic;
            }
            else if (bBold)
            {
                FontStyle = EFontStyle::FontStyle_Bold;
            }
        }
    	return FontStyle;
    }

    bool WrapText(FString& Text, int32 FontSize, int32 RegionWidth, int32 RegionHeight);

	static void RemoveLine(int32 StartIndex);

	float GetLineHeight(float MaxTextFontSize, float MaxNonTextFontSize) const;
    float GetFontDescenderFromBottom(float MaxTextFontSize) const;
    void UpdateBaseLine(float MaxTextFontSize, float MaxNonTextFontSize) const;
	
	float GetGlyphWidth(TCHAR Char, float FontSize, int32 Sub, float Kerning, EFontStyle FontStyle) const;
    float GetGlyphWidth(TCHAR Char, float FontSize, EFontStyle FontStyle) const;
    int32 UpdateGlyph(int32 Char, float FontSize, float LineMaxFontSize, EFontStyle FontStyle, const FVector2D& InvTextureSize, bool bIsUnderline, FVector2D& ScreenPxRange) const;

	static int32 CheckRemoveLine(int32 LineCount, int32 EndIndex);

    FVector2D CalculatePrintedSize() const;
	
    bool Print();

    void Align(int32 IndexOffset, int32 HypertextIndexOffset, int32 EmojiIndexOffset, int32 WidgetIndexOffset, float PrintedWidth, int32 TextElementIndexOffset, int32 MaxTextElementIndex);
	
    static bool ParseSymbol(const FString& Text, const FTextGenerationSettings& Setting, int32& Index, int32& CurFontSize, int32 DefaultFontSize, TArray<FLinearColor>& Colors,
							bool& bColorChanged, int8& Sub, bool& bBold, bool& bItalic, bool& bUnderline, bool& bStrike, FString& Hypertext, int32& HypertextIndex, bool& bEmoji, FName& EmojiSymbol, bool& bWidget, FName& WidgetSymbol);
	
public:
    FVector2D Extents;
    
    TArray<FUIVertex> Vertices;
    TArray<FUICharInfo> Characters;
    TArray<FUILineInfo> Lines;

    TArray<FHypertextRegion> HypertextList;
    TArray<FEmojiRegion> EmojiList;
    TArray<FWidgetRegion> WidgetList;
    TArray<FWidgetRegion> AttachWidgetList;

    FORCEINLINE int32 CharacterCountVisible() const { return Characters.Num() - 1; }
	
private:
    FTextGenerationSettings LastSettings;

    FString LastString; 
    FVector2D PrintedSize;

	uint8 bHasGenerated : 1;

public:
    uint8 bGenerateCharacterAndLineInfo : 1; 

};
