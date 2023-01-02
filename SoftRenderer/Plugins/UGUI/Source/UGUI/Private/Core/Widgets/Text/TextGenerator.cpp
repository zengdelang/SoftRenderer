#include "Core/Widgets/Text/TextGenerator.h"
#include "UGUISettings.h"
#include "Core/Widgets/Text/TextElementInterface.h"
#include "Core/Widgets/Text/TextEmojiSheet.h"

/////////////////////////////////////////////////////
// FTextGenerator

struct FTextElement
{
    int32 LineNumber;
    TCHAR Char;
    
    float FontSize;
    
	int8 SubscriptMode;
	FLinearColor Color;

    FVector2D CursorPos;
    float CharWidth;
    
    uint8 bItalic : 1;
    uint8 bBold : 1;
    uint8 bUnderline : 1;
    uint8 bStrikeThrough : 1;
    uint8 bEmoji : 1;
    uint8 bWidget : 1;
    
    FString Hypertext;
    int32 HypertextIndex;

    FName EmojiSymbol;
    FName WidgetSymbol;

public:
	FTextElement()
	{
        LineNumber = 1;
        Char = 0;
        FontSize = 0;
        SubscriptMode = 0;
        Color = FColor::White;
	    CursorPos = FVector2D::ZeroVector;
	    CharWidth = 0;
        bItalic = false;
        bBold = false;
        bUnderline = false;
        bStrikeThrough = false;
	    bEmoji = false;
	    bWidget = false;
	    Hypertext = TEXT("");
	    HypertextIndex = 0;
	}
    
    bool IsChar() const
	{
	    return bEmoji == false && bWidget == false;
	}
};

struct FLineElement
{
    int32 StartCharIdx;
    float MaxTextFontSize;
    float MaxNonTextFontSize;
    float Height;
    float TopY;

public:
    FLineElement()
	{
        StartCharIdx = 0;
        MaxTextFontSize = -1;
        MaxNonTextFontSize = -1;
        Height = 0;
        TopY = 0;
    }

    FLineElement(float InMaxTextFontSize, float InMaxNonTextFontSize)
    {
        StartCharIdx = 0;
        MaxTextFontSize = InMaxTextFontSize;
        MaxNonTextFontSize = InMaxNonTextFontSize;
        Height = 0;
        TopY = 0;
    }
};

struct FGlyphInfo
{
    FVector2D V0;
    FVector2D V1;
	
    FVector2D U0;
    FVector2D U1;
    FVector2D U2;
    FVector2D U3;
	
    float Advance;
	
public:
    FGlyphInfo()
    {
        Advance = 0;
    }
};

static TArray<FTextElement> TextElementList;
static TArray<FLineElement> LineElementList;

static TArray<FLinearColor> TextColors;

static float LineFactor = 1;
static float FinalKerning = 1;

static FGlyphInfo GlyphInfo;

static float SizeShrinkage = 0.67f;

static float Baseline = 0;
static float BaselineOffset = 0;
static float NonTextBaselineOffset = 0;

FTextGenerator::FTextGenerator()
{
    LastSettings = FTextGenerationSettings();
	bHasGenerated = false;
    bGenerateCharacterAndLineInfo = false;
}

float FTextGenerator::GetPreferredWidth(FString Text, FTextGenerationSettings Settings, const UObject* TextObj)
{
    GetPreferredSize(Text, MoveTemp(Settings), TextObj);
    return PrintedSize.X;
}

float FTextGenerator::GetPreferredHeight(FString Text, FTextGenerationSettings Settings, const UObject* TextObj)
{
    GetPreferredSize(Text, MoveTemp(Settings), TextObj);
    return PrintedSize.Y;
}

void FTextGenerator::GetPreferredSize(FString Text, FTextGenerationSettings Settings, const UObject* TextObj)
{
    if (bHasGenerated && Text.Equals(LastString) && Settings.Equals(LastSettings) && ValidatedSettings(Settings, TextObj))
    {
        return;
    }

    LastSettings = MoveTemp(Settings);
    if (!LastSettings.Font.IsValid())
    {
        return;
    }
    
    bHasGenerated = true;

    LastString = Text;
    Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

    int32 MaxFontSize = LastSettings.FontSize;
    if (LastSettings.bResizeTextForBestFit)
    {
        MaxFontSize = LastSettings.ResizeTextMaxSize;
    }
    
    int32 RegionWidth = FMath::RoundToInt(LastSettings.GenerationExtents.X);
    LineFactor = LastSettings.LineSpacing;
    FinalKerning = (LastSettings.Font->Kerning + LastSettings.Kerning);

    if (LastSettings.GenerationExtents.X < 1)
    {
        RegionWidth = 1e8;
    }

    WrapText(Text, MaxFontSize, RegionWidth, 1e8);
    PrintedSize = CalculatePrintedSize();
}

bool FTextGenerator::Populate(FString Text, FTextGenerationSettings Settings, const UObject* TextObj)
{
    if (bHasGenerated && Text.Equals(LastString) && Settings.Equals(LastSettings) && ValidatedSettings(Settings, TextObj))
        return false;
	
    bHasGenerated = true;

    Extents = Settings.GenerationExtents;

    LastString = Text;
    Text.ReplaceInline(TEXT("\r\n"), TEXT("\n"));

    LastSettings = MoveTemp(Settings);

    Vertices.Empty();
    HypertextList.Empty();
    WidgetList.Empty();
    AttachWidgetList.Empty();

    int32 MinFontSize = LastSettings.FontSize;
    int32 MaxFontSize = LastSettings.FontSize;
    if (LastSettings.bResizeTextForBestFit)
    {
        MinFontSize = LastSettings.ResizeTextMinSize;
        MaxFontSize = LastSettings.ResizeTextMaxSize;
    }

    int32 RegionWidth = FMath::RoundToInt(LastSettings.GenerationExtents.X);
    int32 RegionHeight = FMath::RoundToInt(LastSettings.GenerationExtents.Y);

    LineFactor = LastSettings.LineSpacing;
    FinalKerning = (LastSettings.Font->Kerning + LastSettings.Kerning);

    if (LastSettings.HorizontalOverflow == EHorizontalWrapMode::HorizontalWrapMode_Overflow)
    {
        RegionWidth = 1e8;
    }

    if (LastSettings.VerticalOverflow == EVerticalWrapMode::VerticalWrapMode_Overflow)
    {
        RegionHeight = 1e8;
    }

    for (int32 Index = MaxFontSize; Index >= MinFontSize; --Index)
    {
        const bool bBestFit = WrapText(Text, Index, RegionWidth, RegionHeight);
        if (!bBestFit && LastSettings.bResizeTextForBestFit && Index != MinFontSize)
        {
            continue;
        }

        PrintedSize = CalculatePrintedSize();
        return Print();
    }

    return false;
}

bool FTextGenerator::WrapText(FString& Text, int32 FontSize, int32 RegionWidth, int32 RegionHeight)
{
    TextElementList.Reset();
	EmojiList.Empty();
	
    if (RegionWidth < 1 || RegionHeight < 1)
    {
        return false;
    }

    if (Text.IsEmpty())
    {
        Text = TEXT(" ");
    }

    float RemainingWidth = RegionWidth;
    float RemainingHeight = RegionHeight;

    TextColors.Reset();
    FLinearColor CurColor = LastSettings.Color;

    int8 Sub = 0;
    bool bBold = LastSettings.FontStyle == EFontStyle::FontStyle_Bold || LastSettings.FontStyle == EFontStyle::FontStyle_BoldAndItalic;
    bool bItalic = LastSettings.FontStyle == EFontStyle::FontStyle_Italic || LastSettings.FontStyle == EFontStyle::FontStyle_BoldAndItalic;
    bool bUnderline = false;
    bool bStrikeThrough = false;
    
    FString Hypertext = TEXT("");
    int32 HypertextIndex = 0;

    bool bEmoji = false;
    FName EmojiSymbol = TEXT("");

    bool bWidget = false;
    FName WidgetSymbol = TEXT("");
    
	int32 CurFontSize = FontSize;

    const TArray<TCHAR>& Chars = Text.GetCharArray();
    const int32 TextLen = Text.Len();
    TextElementList.Reserve(TextLen);

    FPlatformMisc::Prefetch(TextElementList.GetData());
	
    for (int32 Index = 0; Index < TextLen; ++Index)
    {         	
        bool bColorChanged = false;
        if (LastSettings.bRichText
            && ParseSymbol(Text, LastSettings, Index, CurFontSize, LastSettings.FontSize, TextColors, bColorChanged,
                Sub, bBold, bItalic, bUnderline, bStrikeThrough, Hypertext, HypertextIndex, bEmoji, EmojiSymbol, bWidget, WidgetSymbol))
        {
            --Index;

            if (bEmoji)
            {
                bEmoji = false;
                bWidget = false;
                
                const auto TextElementInterface = Cast<ITextElementInterface>(LastSettings.TextComponent.Get());   
                if (!TextElementInterface)
                    continue;

                FTextEmoji* TextEmoji = TextElementInterface->GetTextEmoji(EmojiSymbol);
                if (TextEmoji == nullptr)
                    continue;

                const float EmojiScale = TextEmoji->GetScale(CurFontSize, LastSettings.NonTextScale * LastSettings.Font->NonTextScale);

                FTextElement TextElement;
                TextElement.Char = TEXT('X');
                TextElement.FontSize = TextEmoji->GetHeight(EmojiScale);
                TextElement.CharWidth = FinalKerning + TextEmoji->GetWidth(EmojiScale);
                TextElement.CursorPos.X = EmojiScale;
                TextElement.bEmoji = true;
                TextElement.EmojiSymbol = EmojiSymbol;
                TextElementList.Emplace(TextElement);

            	FEmojiRegion EmojiRegion = FEmojiRegion();
            	EmojiRegion.EmojiSymbol = TextElement.EmojiSymbol;
            	EmojiList.Emplace(EmojiRegion);
            	
                continue;
            }

            if (bWidget)
            {
                bEmoji = false;
                bWidget = false;

                const auto TextElementInterface = Cast<ITextElementInterface>(LastSettings.TextComponent.Get());   
                if (!TextElementInterface)
                    continue;

                FTextWidget* TextWidget = TextElementInterface->GetTextWidget(WidgetSymbol);
                FAttachWidget* AttachWidget = TextElementInterface->GetAttachWidget(WidgetSymbol);
                if (TextWidget == nullptr && AttachWidget == nullptr)
                    continue;

                FTextElement TextElement;
                TextElement.Char = TEXT('X');
                TextElement.bWidget = true;
                TextElement.WidgetSymbol = WidgetSymbol;
                
                if (TextWidget)
                {
                    const float WidgetScale = TextWidget->GetScale(CurFontSize, LastSettings.NonTextScale * LastSettings.Font->NonTextScale);
                    TextElement.FontSize = TextWidget->GetHeight(WidgetScale);
                    TextElement.CharWidth = FinalKerning + TextWidget->GetWidth(WidgetScale);
                    TextElement.CursorPos.X = WidgetScale;
                    TextElementList.Emplace(TextElement);
                }
                else if (AttachWidget)
                {
                    TextElement.FontSize = AttachWidget->GetHeight();
                    TextElement.CharWidth = FinalKerning + AttachWidget->GetWidth();
                    TextElement.CursorPos.X = 1;
                    TextElementList.Emplace(TextElement);
                }
            }  
            
        	if (bColorChanged)
        	{
                if (TextColors.Num() == 0)
                {
                    CurColor = LastSettings.Color;
                }
                else
                {
                    if (LastSettings.bBlendComponentColor)
                    {
                        CurColor = LastSettings.Color * TextColors[TextColors.Num() - 1];
                    }
                    else
                    {
                        CurColor = TextColors[TextColors.Num() - 1];
                    }
                }
        	}      
            continue;
        }

        const TCHAR Char = Chars[Index];
        
        if (FChar::IsLinebreak(Char))
        {
            FTextElement TextElement;
            TextElement.Char = Char;
            TextElement.FontSize = CurFontSize;
            TextElementList.Emplace(TextElement);
            continue;
        }

        if (Char < TEXT(' ') || Char == 8203)  // 8203 : ZERO WIDTH SPACE
        {
            continue;
        }

        const auto FontStyle = GetGlyphFontStyle(bItalic, bBold);
        bool bUseNormalStyle = true;
        const int32 CharIndex = LastSettings.Font->RemapChar(Chars[Index], bUseNormalStyle, FontStyle);
        
        // Skip invalid characters.
        if (!LastSettings.Font->Characters.IsValidIndex(CharIndex))
        {
            continue;
        }
        
        const float Width = GetGlyphWidth(Char, CurFontSize, Sub, FinalKerning, FontStyle);
        if (Width <= 0)
        {
            continue;
        }

        FTextElement TextElement;
        TextElement.bBold = bUseNormalStyle ? bBold : false;
        TextElement.bItalic = bUseNormalStyle ? bItalic : false;
        TextElement.bStrikeThrough = bStrikeThrough;
        TextElement.SubscriptMode = Sub;
        TextElement.CharWidth = Width;
        TextElement.bUnderline = bUnderline;
        TextElement.Color = CurColor;
        TextElement.FontSize = CurFontSize;
        TextElement.Char = Char;
        if (Hypertext != TEXT(""))
        {
            TextElement.Hypertext = Hypertext;
            TextElement.HypertextIndex = HypertextIndex;
        }
        TextElementList.Emplace(TextElement);
    }

    int32 Offset = 0;
    const int32 TextCount = TextElementList.Num();

    bool bEastern = false;
    bool bHasSpace = false;
    bool bLineIsEmpty = true;
	
    int32 Start = 0;
    int32 LineCount = 1;

    float MaxTextHeight = -1;
    float MaxNonTextHeight = -1;
    float SpaceBeforeMaxTextHeight = -1;
    float SpaceBeforeMaxNonTextHeight = -1;
 
    for (; Offset < TextCount; ++Offset)
    {
        auto& TextElement = TextElementList[Offset];
        if (TextElement.bEmoji)
        {
            bEastern = true;
        }
        else if(TextElement.bWidget)
        {
            bEastern = true;
        }
        else
        {
            const int32 Char = TextElement.Char;
    	
            if (Char > 12287)
                bEastern = true;

            if (FChar::IsLinebreak(Char))
            {
                TextElement.LineNumber = LineCount;
 
                if (MaxTextHeight < 0)
                {
                    MaxTextHeight = TextElement.FontSize;
                }

                const float Height = GetLineHeight(MaxTextHeight, MaxNonTextHeight);
                RemainingHeight -= Height;
                if (FMath::RoundToInt(RemainingHeight) < 0)
                {
                    Start = CheckRemoveLine(LineCount, Offset);
                    break;
                }
        	
                ++LineCount;
                RemainingWidth = RegionWidth;
                MaxTextHeight = -1;
                MaxNonTextHeight = -1;
                bEastern = false;
                bHasSpace = false;
                continue;
            }
        }

        const float GlyphWidth = TextElement.CharWidth;
        if (FMath::RoundToInt(GlyphWidth) > RegionWidth)
        {
            Start = CheckRemoveLine(LineCount, Offset - 1);
            RemainingHeight = -MAX_flt;
            break;
        }

        RemainingWidth -= GlyphWidth;
        TextElement.LineNumber = LineCount;

        const bool bIsTextElement = !(TextElement.bEmoji || TextElement.bWidget);
        if (bIsTextElement && IsWhitespaceWithoutNonBreakingSpace(TextElement.Char))
        {
            Start = Offset;
            bHasSpace = true;
            bEastern = false;

            SpaceBeforeMaxTextHeight = MaxTextHeight;
            SpaceBeforeMaxNonTextHeight = MaxNonTextHeight;
            
            if (bIsTextElement && TextElement.FontSize > SpaceBeforeMaxTextHeight)
            {
                SpaceBeforeMaxTextHeight = TextElement.FontSize;
            }

            if (!bIsTextElement && TextElement.FontSize > SpaceBeforeMaxNonTextHeight)
            {
                SpaceBeforeMaxNonTextHeight = TextElement.FontSize;
            }
        }

        if (FMath::RoundToInt(RemainingWidth) >= 0)
        {
            if (bIsTextElement && TextElement.FontSize > MaxTextHeight)
            {
                MaxTextHeight = TextElement.FontSize;
            }

            if (!bIsTextElement && TextElement.FontSize > MaxNonTextHeight)
            {
                MaxNonTextHeight = TextElement.FontSize;
            }
        }
        else
        {
            const bool bIsWhitespace = IsWhitespaceWithoutNonBreakingSpace(TextElement.Char);
            if (bIsTextElement && bIsWhitespace)
            {
                bHasSpace = false;
                bEastern = true;
            }
            else
            {
                if (bHasSpace)
                {
                    if (bEastern)
                    {
                        bEastern = !bIsTextElement || TextElement.Char > 12287;
                    }
                    else
                    {
                        bLineIsEmpty = false;
                        for (int32 Index = Offset + 1; Index < TextCount; ++Index)
                        {
                            const int32 TempChar = TextElementList[Index].Char;
                            if (TempChar > 12287)
                            {
                                bLineIsEmpty = true;
                                break;
                            }

                            if (IsWhitespaceWithoutNonBreakingSpace(TempChar))
                            {
                                break;
                            }
                        }
                    }
                }
                
                bHasSpace = false;
            }

            if (bLineIsEmpty)
            { 
                const float Height = GetLineHeight(MaxTextHeight, MaxNonTextHeight);
 
                RemainingHeight -= Height;
                if (FMath::RoundToInt(RemainingHeight) < 0)
                {
                    Start = CheckRemoveLine(LineCount, Offset);
                    break;
                }

                // If it is a white space character, it will not go back to the previous character even if it exceeds the RegionWidth
            	if (!bIsWhitespace)
            	    --Offset;

            	++LineCount;
                bLineIsEmpty = true;
                MaxNonTextHeight = -1;
                MaxTextHeight = -1;
                RemainingWidth = RegionWidth;
                bEastern = false;
            	bHasSpace = false;
            }
            else
            {
                const float Height = GetLineHeight(SpaceBeforeMaxTextHeight, SpaceBeforeMaxNonTextHeight);

                RemainingHeight -= Height;
                if (FMath::RoundToInt(RemainingHeight) < 0)
                {
                    Start = CheckRemoveLine(LineCount, Offset);
                    break;
                }

                Offset = Start;
                ++LineCount;
                bLineIsEmpty = true;
                MaxNonTextHeight = -1;
                MaxTextHeight = -1;
                RemainingWidth = RegionWidth;
                bEastern = false;
            	bHasSpace = false;
            }
        }
    }

    if (FMath::RoundToInt(RemainingHeight) >= 0)
    {
        float Height = GetLineHeight(MaxTextHeight, MaxNonTextHeight);
        RemainingHeight -= Height;
        Start = TextElementList.Num();
        if (FMath::RoundToInt(RemainingHeight) < 0)
        {
            Start = CheckRemoveLine(LineCount, Offset);
        }
    }

    RemoveLine(Start);

    if (Offset == TextCount && TextElementList.Num() == TextCount)
    {
        return true;
    }

    if (LastSettings.bElipsizeEnd && TextElementList.Num() > 0 )
    {
        int32 TotalCount = TextElementList.Num();
        int32 StartIndex = TotalCount;
        
        const int32 LastLineNumber = TextElementList[StartIndex - 1].LineNumber;
        int32 LastLineStartIndex = -2;

        if (LastLineNumber == 1)
        {
            LastLineStartIndex = -1;
        }
        else
        {
            for (int32 Index = TotalCount - 1; Index >= 0; --Index)
            {
                const auto& TextElementInfo = TextElementList[Index];
                if (LastLineNumber != TextElementInfo.LineNumber)
                {
                    LastLineStartIndex = Index - 1;
                    break;
                }
            }
        }

        if (LastLineStartIndex >= -1)
        {
            float CurDotFontSize = 0;
            EFontStyle CurDotStyle;
            uint8 CurDotSubscriptMode = 0;
            
            if (LastLineStartIndex == -1)
            {
                CurDotFontSize = LastSettings.FontSize;
                CurDotStyle = LastSettings.FontStyle;
            }
            else
            {
                const FTextElement& TextElementInfo = TextElementList[LastLineStartIndex];
                CurDotFontSize = TextElementInfo.FontSize;
                CurDotSubscriptMode = TextElementInfo.SubscriptMode;
                CurDotStyle = GetGlyphFontStyle(TextElementInfo.bItalic, TextElementInfo.bBold);
            }

            float DotWidth = GetGlyphWidth(TEXT('.'), CurDotFontSize, CurDotSubscriptMode, FinalKerning, CurDotStyle);
            float TotalDotWidth =(DotWidth + FinalKerning) * 3;

            int32 ValidDotIndex = -1;  
            if (FMath::RoundToInt(TotalDotWidth) <= RegionWidth)
            {
                ValidDotIndex = LastLineStartIndex;
            }
            
            float LastLineWidth = 0.0f;
            for (int32 Index = LastLineStartIndex + 1; Index < TotalCount; ++Index)
            {
                const FTextElement& TextElementInfo = TextElementList[Index];
                if (TextElementInfo.IsChar())
                {
                    float CurCharFontSize = TextElementInfo.FontSize;
                    EFontStyle CurCharStyle = GetGlyphFontStyle(TextElementInfo.bItalic, TextElementInfo.bBold);
                    uint8 CurCharSubscriptMode = TextElementInfo.SubscriptMode;
                    
                    if (CurDotStyle != CurCharStyle || CurDotSubscriptMode != CurCharSubscriptMode
                        || !FMath::IsNearlyEqual(CurDotFontSize, CurCharFontSize))
                    {
                        CurDotFontSize = CurCharFontSize;
                        CurDotStyle = CurCharStyle;
                        CurDotSubscriptMode = CurCharSubscriptMode;

                        DotWidth = GetGlyphWidth(TEXT('.'), CurDotFontSize, CurDotSubscriptMode, FinalKerning, CurDotStyle);
                        TotalDotWidth = DotWidth * 3;
                    }
                }
                
                float TempLastLineWidth = LastLineWidth + TextElementInfo.CharWidth;
                if (FMath::RoundToInt(TempLastLineWidth + TotalDotWidth) <= RegionWidth)
                {
                    LastLineWidth = TempLastLineWidth;
                    ValidDotIndex = Index;
                }
                else
                {
                    break;
                }
            }

            if (ValidDotIndex > -1)
            {
                RemoveLine(ValidDotIndex + 1);
                TotalCount = TextElementList.Num();
                FTextElement ElipsizeElement = FTextElement();
                ElipsizeElement.FontSize = LastSettings.FontSize;
                ElipsizeElement.Char = '.';
                ElipsizeElement.LineNumber = TextElementList[TotalCount -1].LineNumber;
                ElipsizeElement.Color = LastSettings.Color;
                TextElementList.Emplace(ElipsizeElement);
                TextElementList.Emplace(ElipsizeElement);
                TextElementList.Emplace(ElipsizeElement);
            }
        }
    }

    return false;
}

void FTextGenerator::RemoveLine(int32 StartIndex)
{
    for (int32 I = TextElementList.Num() - 1; I >= StartIndex; --I)
    {
        TextElementList.RemoveAt(I, 1, false);
    }
}

float FTextGenerator::GetLineHeight(float MaxTextFontSize, float MaxNonTextFontSize) const
{
    if (MaxTextFontSize < 0)
    {
        MaxTextFontSize = LastSettings.FontSize;
    }
    
    const float MaxTextLineHeight = MaxTextFontSize / LastSettings.Font->ImportFontSize * LastSettings.Font->LineHeight * LineFactor;
    if (MaxNonTextFontSize < 0)
    {
        return MaxTextLineHeight;
    }
   
    const float MaxNonTextLineHeight = (MaxNonTextFontSize + GetFontDescenderFromBottom(MaxTextFontSize)) * LineFactor;
    if (MaxTextLineHeight > MaxNonTextLineHeight)
    {
        return MaxTextLineHeight;
    }    
    return MaxNonTextLineHeight;
}

float FTextGenerator::GetFontDescenderFromBottom(float MaxTextFontSize) const
{
    if (MaxTextFontSize < 0)
    {
        MaxTextFontSize = LastSettings.FontSize;
    }

    const float ScaleFactor = MaxTextFontSize / LastSettings.ImportFontSize;
    return (LastSettings.Font->LineHeight - LastSettings.Font->Baseline) * ScaleFactor;
}

void FTextGenerator::UpdateBaseLine(float MaxTextFontSize, float MaxNonTextFontSize) const
{
    if (MaxTextFontSize < 0)
    {
        MaxTextFontSize = LastSettings.FontSize;
    }
    
    const float ScaleFactor = MaxTextFontSize / LastSettings.ImportFontSize;
    //TODO   Sup  Sub 算法重做 
    Baseline = LastSettings.Font->Baseline * ScaleFactor;

    NonTextBaselineOffset = ScaleFactor * LastSettings.Font->BaselineOffset;
    
    if (MaxNonTextFontSize < 0)
    {
        return;
    }
    
    if (MaxNonTextFontSize > Baseline)
    {
        Baseline = MaxNonTextFontSize;
    }    
}

float FTextGenerator::GetGlyphWidth(TCHAR Char, float FontSize, int32 Sub, float Kerning, EFontStyle FontStyle) const
{
    const float Width = GetGlyphWidth(Char, FontSize, FontStyle);
    if (Width <= 0)
    {
        return -MAX_flt;
    }

    float GlyphWidth = Kerning + Width;
    if (Sub != 0)
    {
        GlyphWidth *= SizeShrinkage;
    }
    return GlyphWidth;
}

float FTextGenerator::GetGlyphWidth(TCHAR Char, float FontSize, EFontStyle FontStyle) const
{
    if (LastSettings.Font.IsValid())
    {
        bool bUseNormalStyle = true;
        const int32 CharIndex = LastSettings.Font->RemapChar(Char, bUseNormalStyle, FontStyle);
        if (LastSettings.Font->Characters.IsValidIndex(CharIndex))
        {
            float BoldSpacingScale = 0;
            if (bUseNormalStyle && (FontStyle == EFontStyle::FontStyle_Bold || FontStyle == EFontStyle::FontStyle_BoldAndItalic))
            {
                BoldSpacingScale = LastSettings.Font->BoldSpacingScale;
            }
            
            const auto& FontChar = LastSettings.Font->Characters[CharIndex];
            return FontChar.Advance * (1 + BoldSpacingScale) * (FontSize / LastSettings.ImportFontSize) * FontChar.Scale;
        }   
    }
    return 0;
}

int32 FTextGenerator::UpdateGlyph(int32 Char, float FontSize, float LineMaxFontSize, EFontStyle FontStyle, const FVector2D& InvTextureSize, bool bIsUnderline, FVector2D& ScreenPxRange) const
{
    bool bUseNormalStyle = true;
    const int32 CharIndex = LastSettings.Font->RemapChar(Char, bUseNormalStyle, FontStyle);
    if (!LastSettings.Font->Characters.IsValidIndex(CharIndex))
    {
        GlyphInfo.Advance = 0;
        GlyphInfo.V0.X = 0;
        GlyphInfo.V0.Y = 0;
        GlyphInfo.V1.X = 0;
        GlyphInfo.V1.Y = 0;
        GlyphInfo.U0.X = 0;
        GlyphInfo.U0.Y = 0;
        GlyphInfo.U1.X = 0;
        GlyphInfo.U1.Y = 0;
        GlyphInfo.U2.X = 0;
        GlyphInfo.U2.Y = 0;
        GlyphInfo.U3.X = 0;
        GlyphInfo.U3.Y = 0;
        return 0;
    }

    float BoldSpacingScale = 0;
    if (bUseNormalStyle && (FontStyle == EFontStyle::FontStyle_Bold || FontStyle == EFontStyle::FontStyle_BoldAndItalic))
    {
        BoldSpacingScale = LastSettings.Font->BoldSpacingScale;
    }
	
    const auto& FontChar = LastSettings.Font->Characters[CharIndex];
    const float SizeScale = FontSize / LastSettings.ImportFontSize;
    const float FontScale = SizeScale * FontChar.Scale;
    const float GlyphWidth = FontChar.USize * FontScale;
    const float GlyphHeight = FontChar.VSize * FontScale;

    const float FinalHorizontalOffset = FontChar.HorizontalOffset * FontScale;
    GlyphInfo.V0.X = 0 + FinalHorizontalOffset;       
    GlyphInfo.V1.X = GlyphWidth + FinalHorizontalOffset;

    if (bIsUnderline)
    {
        GlyphInfo.V1.Y = LastSettings.Font->UnderlineY * SizeScale - Baseline;
        GlyphInfo.V0.Y = GlyphInfo.V1.Y - LastSettings.Font->UnderlineThickness * SizeScale;
    }
    else
    {
        GlyphInfo.V0.Y = -Baseline + FontChar.AscenderY * FontScale;
        GlyphInfo.V1.Y = GlyphInfo.V0.Y - GlyphHeight;
    }

    // 0.5 to avoid bilinear bleeding
    const float U = (FontChar.StartU + 0.5) * InvTextureSize.X;
    const float V = (FontChar.StartV + 0.5) * InvTextureSize.Y;
    const float SizeU = (FontChar.USize - 1) * InvTextureSize.X;
    const float SizeV = (FontChar.VSize - 1) * InvTextureSize.Y;

    GlyphInfo.U0.X = U;
    GlyphInfo.U0.Y = V;
    GlyphInfo.U1.X = U;
    GlyphInfo.U1.Y = V + SizeV;
    GlyphInfo.U2.X = U + SizeU;
    GlyphInfo.U2.Y = V + SizeV;
    GlyphInfo.U3.X = U + SizeU;
    GlyphInfo.U3.Y = V;

    GlyphInfo.Advance = FontChar.Advance * (1 + BoldSpacingScale) * FontScale;
    LastSettings.Font->UpdateScreenPxRange(FontChar, ScreenPxRange);
    return FontChar.TextureIndex;
}

int32 FTextGenerator::CheckRemoveLine(int32 LineCount, int32 EndIndex)
{
    int32 Start = EndIndex < 0 ? 0 : EndIndex;
    for (int32 Index = TextElementList.Num() - 1; Index >= 0; --Index)
    {
        if (Index > EndIndex)
        {
            continue;
        }

        if (TextElementList[Index].LineNumber != LineCount)
        {
            break;
        }
    	
        Start = Index;
    }
	
    return Start;
}

FVector2D FTextGenerator::CalculatePrintedSize() const
{
    FVector2D Size = FVector2D::ZeroVector;
	
    if (TextElementList.Num() > 0)
    {
        float X = 0;
        float Y = 0;
    	float MaxX = 0;

        float CurrentLineMaxTextHeight = -1;
        float CurrentLineMaxNonTextHeight = -1;
        
        int32 LineNumber = 1;
    	
        for (int32 Index = 0, TextCount = TextElementList.Num(); Index < TextCount; ++Index)
        {
            const auto& TextElement = TextElementList[Index];
            if (TextElement.LineNumber != LineNumber)
            {
                if (X > MaxX)
                {
                    MaxX = X;
                }
            	
                X = 0;
                Y += GetLineHeight(CurrentLineMaxTextHeight, CurrentLineMaxNonTextHeight);

                CurrentLineMaxTextHeight = -1;
                CurrentLineMaxNonTextHeight = -1;
                LineNumber = TextElement.LineNumber;
            }

            if (TextElement.bEmoji || TextElement.bWidget)
            {
                X += TextElement.CharWidth;             
                if (TextElement.FontSize > CurrentLineMaxNonTextHeight)
                {
                    CurrentLineMaxNonTextHeight = TextElement.FontSize;

                }   
                continue;
            }
            
            if (FChar::IsLinebreak(TextElement.Char))
            {
                if (CurrentLineMaxTextHeight < 0)
                {
                    CurrentLineMaxTextHeight = TextElement.FontSize;
                }
                continue;
            }

            X += TextElement.CharWidth;
            if (TextElement.FontSize > CurrentLineMaxTextHeight)
            {
                CurrentLineMaxTextHeight = TextElement.FontSize;
            }
        }

        if (X > MaxX)
        {
            MaxX = X;
        }
    	
        Y += GetLineHeight(CurrentLineMaxTextHeight, CurrentLineMaxNonTextHeight);

        Size.X = MaxX;
        Size.Y = Y;
    }

    return Size;
}

static TArray<FVector2D> TempUVs = { FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector };
static TArray<FVector> TempVertices = { FVector::ZeroVector,FVector::ZeroVector,FVector::ZeroVector,FVector::ZeroVector };

bool FTextGenerator::Print()
{
    if (TextElementList.Num() == 0)
        return false;

    LineElementList.Reset();

    int32 TextCount = TextElementList.Num();
    if (TextCount > 0)
    {
        LineElementList.Reserve(TextElementList[TextCount - 1].LineNumber);
    }

    FPlatformMisc::Prefetch(LineElementList.GetData());
    FPlatformMisc::Prefetch(TextElementList.GetData());

    int32 CurrentLineIndex = 1;
    int32 StartCharIdx = 0;
    
    float CurrentLineMaxTextSize = -1;
    float CurrentLineMaxNonTextSize = -1;
	
    for (int32 Index = 0; Index < TextCount; ++Index)
    {
        auto& TextElement = TextElementList[Index];
        if (TextElement.LineNumber != CurrentLineIndex)
        {
            FLineElement LineElement;
            LineElement.MaxTextFontSize = CurrentLineMaxTextSize;
            LineElement.MaxNonTextFontSize = CurrentLineMaxNonTextSize;
            LineElement.StartCharIdx = StartCharIdx;
            StartCharIdx = Index;
            LineElementList.Emplace(LineElement);

            CurrentLineIndex = TextElement.LineNumber;

            const bool bIsTextElement = !(TextElement.bEmoji || TextElement.bWidget);
            
            CurrentLineMaxNonTextSize = !bIsTextElement ? TextElement.FontSize : -1;
            CurrentLineMaxTextSize = !bIsTextElement ? -1 : TextElement.FontSize;
        }
        else if (FChar::IsLinebreak(TextElement.Char))
        {
            if (CurrentLineMaxTextSize < 0)
                CurrentLineMaxTextSize = TextElement.FontSize;

            CurrentLineIndex = TextElement.LineNumber + 1;
            
            FLineElement LineElement;
            LineElement.MaxTextFontSize = CurrentLineMaxTextSize;
            LineElement.MaxNonTextFontSize = CurrentLineMaxNonTextSize;
            LineElement.StartCharIdx = StartCharIdx;
            StartCharIdx = Index + 1;
            LineElementList.Emplace(LineElement);

            CurrentLineMaxNonTextSize = -1;
            CurrentLineMaxTextSize = -1;
        }
        else
        {
            const bool bIsTextElement = !(TextElement.bEmoji || TextElement.bWidget);
            if (!bIsTextElement)
            {
                if (TextElement.FontSize > CurrentLineMaxNonTextSize)
                {
                    CurrentLineMaxNonTextSize = TextElement.FontSize;
                }
            }
            else
            {
                if (TextElement.FontSize > CurrentLineMaxTextSize)
                {
                    CurrentLineMaxTextSize = TextElement.FontSize;
                }
            }
        }
    }

    FLineElement LastLineElement;
    LastLineElement.MaxTextFontSize = CurrentLineMaxTextSize;
    LastLineElement.MaxNonTextFontSize = CurrentLineMaxNonTextSize;
    LastLineElement.StartCharIdx = StartCharIdx;
    LineElementList.Emplace(LastLineElement);

    float X = 0;
	float Y = 0;

    float PrevX = 0;

    int32 IndexOffset = 0;
    int32 HypertextIndexOffset = 0;
    int32 EmojiIndexOffset = 0;
    int32 WidgetIndexOffset = 0;
    int32 TextElementIndexOffset = 0;
    int32 CurrentLine = 1;
    FLineElement* LineElementPtr = &LineElementList[0];
    
	float CurrentLineHeight = GetLineHeight(LineElementPtr->MaxTextFontSize, LineElementPtr->MaxNonTextFontSize);
    UpdateBaseLine(LineElementPtr->MaxTextFontSize, LineElementPtr->MaxNonTextFontSize);
	
    LineElementPtr->Height = CurrentLineHeight;
    LineElementPtr->TopY = -Y;

    float FinalBoldStyle = 0;
    FVector2D InvTextureSize(1.0f, 1.0f);
	if (LastSettings.Font.IsValid())
	{
	    FinalBoldStyle = LastSettings.Font->BoldStyle;
	    InvTextureSize.X = 1.0f / LastSettings.Font->AtlasWidth;
	    InvTextureSize.Y = 1.0f / LastSettings.Font->AtlasHeight;
	}

    // add an invisible char into TextElementList 
    {
        int32 InvisibleLineNumber = 1;
        if (TextElementList.Num() > 0)
        {
            const auto& LastTextElement = TextElementList[TextElementList.Num() - 1];
            InvisibleLineNumber = LastTextElement.LineNumber;
            if (FChar::IsLinebreak(LastTextElement.Char))
            {
                ++InvisibleLineNumber;
            }
        }
    
        FTextElement TextElement;
        TextElement.CharWidth = 0;
        TextElement.Char = 0;
        TextElement.LineNumber = InvisibleLineNumber;
        TextElementList.Emplace(TextElement);  
    }

    TextCount = TextElementList.Num();

	int32 EmojiCount = 0;

    Vertices.Reserve(TextCount * 4);
    FPlatformMisc::Prefetch(Vertices.GetData());
    
    for (int32 Index = 0; Index < TextCount; ++Index)
    {
        auto& TextElement = TextElementList[Index];

        if (TextElement.LineNumber != CurrentLine)
        {
            if (!IsLeftPivot(LastSettings.TextAnchor))
            {
                Align(IndexOffset, HypertextIndexOffset, EmojiIndexOffset, WidgetIndexOffset, X - FinalKerning, TextElementIndexOffset, Index);
                IndexOffset = Vertices.Num();
                EmojiIndexOffset = EmojiCount;
                WidgetIndexOffset = WidgetList.Num() + AttachWidgetList.Num();
                HypertextIndexOffset = HypertextList.Num();
                TextElementIndexOffset = Index;
            }

            X = 0;
            Y += CurrentLineHeight;

            LineElementPtr = &LineElementList[CurrentLine];
            CurrentLineHeight = GetLineHeight(LineElementPtr->MaxTextFontSize, LineElementPtr->MaxNonTextFontSize);
            UpdateBaseLine(LineElementPtr->MaxTextFontSize, LineElementPtr->MaxNonTextFontSize);
       	
            CurrentLine = TextElement.LineNumber;

            LineElementPtr->Height = CurrentLineHeight;
            LineElementPtr->TopY = -Y;

            if (TextElement.Char == 0)
            {
                break;
            }
        }

        if (TextElement.Char == 0)
        {
            break;
        }
    	
        PrevX = X;

        const float ElementNonTextScale = TextElement.CursorPos.X;
        TextElement.CursorPos.X = X;
        TextElement.CursorPos.Y = -Y;
 
        if (TextElement.bEmoji)
        {
            float PaddingLeft = 0;
            float PaddingBottom = 0;
            float Width = 0;
            float Height = 0;

            ITextElementInterface* TextElementInterface = Cast<ITextElementInterface>(LastSettings.TextComponent.Get());
            if (TextElementInterface)
            {
                FTextEmoji* TextEmoji = TextElementInterface->GetTextEmoji(TextElement.EmojiSymbol);
                if (TextEmoji)
                {
                    PaddingLeft = TextEmoji->PaddingLeft * ElementNonTextScale;
                    PaddingBottom = TextEmoji->PaddingBottom * ElementNonTextScale;
                    Width = TextEmoji->Width * ElementNonTextScale;
                    Height = TextEmoji->Height * ElementNonTextScale;
                }
            }

        	if (EmojiList.IsValidIndex(EmojiCount))
        	{
        		FEmojiRegion& EmojiRegion = EmojiList[EmojiCount];
        		EmojiRegion.BottomLeft.X = X + PaddingLeft;
        		EmojiRegion.BottomLeft.Y = -Baseline - Y + PaddingBottom - NonTextBaselineOffset;
        		EmojiRegion.TopRight.X = EmojiRegion.BottomLeft.X + Width;
        		EmojiRegion.TopRight.Y = EmojiRegion.BottomLeft.Y + Height;
        		EmojiRegion.bVisible = true;
        	}

        	++EmojiCount;
            
            X += TextElement.CharWidth;
            continue;
        }
    	
    	if (TextElement.bWidget)
    	{
            bool bTextWidget = false;
            bool bAttachWidget = false;
    	    
    	    float PaddingLeft = 0;
    	    float PaddingBottom = 0;
    	    float Width = 0;
    	    float Height = 0;

    	    ITextElementInterface* TextElementInterface = Cast<ITextElementInterface>(LastSettings.TextComponent.Get());
    	    if (TextElementInterface)
    	    {
    	        if (FTextWidget* TextWidget = TextElementInterface->GetTextWidget(TextElement.WidgetSymbol))
    	        {
    	            PaddingLeft = TextWidget->PaddingLeft * ElementNonTextScale;
    	            PaddingBottom = TextWidget->PaddingBottom * ElementNonTextScale;
    	            Width = TextWidget->Width * ElementNonTextScale;
    	            Height = TextWidget->Height * ElementNonTextScale;
    	            bTextWidget = true;
    	        }
    	        else if (FAttachWidget* AttachWidget = TextElementInterface->GetAttachWidget(TextElement.WidgetSymbol))
    	        {
    	            PaddingLeft = AttachWidget->PaddingLeft * ElementNonTextScale;
    	            PaddingBottom = AttachWidget->PaddingBottom * ElementNonTextScale;
    	            if (IsValid(AttachWidget->AttachWidget))
    	            {
    	                Width = AttachWidget->AttachWidget->GetRect().Width * ElementNonTextScale;
    	                Height = AttachWidget->AttachWidget->GetRect().Height * ElementNonTextScale;
    	            }
    	            bAttachWidget = true;
    	        }
    	    }

    	    FWidgetRegion WidgetRegion = FWidgetRegion();
    	    WidgetRegion.BottomLeft.X = X + PaddingLeft;
    	    WidgetRegion.BottomLeft.Y = -Baseline - Y + PaddingBottom - NonTextBaselineOffset;
    	    WidgetRegion.TopRight.X = WidgetRegion.BottomLeft.X + Width;
    	    WidgetRegion.TopRight.Y = WidgetRegion.BottomLeft.Y + Height;
    	    WidgetRegion.WidgetSymbol = TextElement.WidgetSymbol;
    	    if (bTextWidget)
    	    {
    	        WidgetList.Emplace(WidgetRegion);
    	    }
    	    else if (bAttachWidget)
    	    {
    	        AttachWidgetList.Emplace(WidgetRegion);
    	    }
            
    	    X += TextElement.CharWidth;
            continue;
    	}
    	
        if (FChar::IsLinebreak(TextElement.Char))
        {
            continue;
        }

        const auto FontStyle = GetGlyphFontStyle(TextElement.bItalic, TextElement.bBold);
        FVector2D ScreenPxRange = FVector2D::ZeroVector;
        const int32 TextureIndex = UpdateGlyph(TextElement.Char, TextElement.FontSize, LineElementPtr->MaxTextFontSize, FontStyle, InvTextureSize, false, ScreenPxRange);

        if (TextElement.SubscriptMode != 0)
        {
            GlyphInfo.V0.X *= SizeShrinkage;
            GlyphInfo.V0.Y *= SizeShrinkage;
            GlyphInfo.V1.X *= SizeShrinkage;
            GlyphInfo.V1.Y *= SizeShrinkage;

            if (TextElement.SubscriptMode == 1)
            {
                float YMoveDelta = (Baseline - BaselineOffset) * 0.33f + TextElement.FontSize * 0.075f;
                GlyphInfo.V0.Y -= YMoveDelta;
                GlyphInfo.V1.Y -= YMoveDelta;
            }
            else
            {
                float YMoveDelta = ((Baseline - BaselineOffset) - TextElement.FontSize) * 0.33f;
                GlyphInfo.V0.Y -= YMoveDelta;
                GlyphInfo.V1.Y -= YMoveDelta;
            }
        }

        float V0X = GlyphInfo.V0.X + X;
        float V0Y = GlyphInfo.V0.Y - Y;
        float V1X = GlyphInfo.V1.X + X;
        float V1Y = GlyphInfo.V1.Y - Y;

        X += (TextElement.SubscriptMode == 0)
            ? FinalKerning + GlyphInfo.Advance
            : (FinalKerning + GlyphInfo.Advance) * SizeShrinkage;

        if (!IsWhitespaceWithNonBreakingSpace(TextElement.Char))
        {
            TempUVs[0].X = GlyphInfo.U0.X;
            TempUVs[0].Y = GlyphInfo.U0.Y;
            TempUVs[1].X = GlyphInfo.U1.X;
            TempUVs[1].Y = GlyphInfo.U1.Y;
            TempUVs[2].X = GlyphInfo.U2.X;
            TempUVs[2].Y = GlyphInfo.U2.Y;
            TempUVs[3].X = GlyphInfo.U3.X;
            TempUVs[3].Y = GlyphInfo.U3.Y;
        	
            if (!TextElement.bItalic)
            {
                TempVertices[0].X = V0X;
                TempVertices[0].Y = V0Y;
                TempVertices[1].X = V0X;
                TempVertices[1].Y = V1Y;
                TempVertices[2].X = V1X;
                TempVertices[2].Y = V1Y;
                TempVertices[3].X = V1X;
                TempVertices[3].Y = V0Y;
            }
            else // Italic
            {
                float Slant = 0.2f * (V1Y - V0Y);
                TempVertices[0].X = V0X - Slant;
                TempVertices[0].Y = V0Y;
                TempVertices[1].X = V0X;
                TempVertices[1].Y = V1Y;
                TempVertices[2].X = V1X;
                TempVertices[2].Y = V1Y;
                TempVertices[3].X = V1X - Slant;
                TempVertices[3].Y = V0Y;
            }

            for (int32 VertexIndex = 0; VertexIndex < 4; ++VertexIndex)
            {
                FUIVertex UIVertex = FUIVertex::SimpleVertex;
                UIVertex.UV0 = TempUVs[VertexIndex];
                UIVertex.UV2.X = TextElement.bBold ? FinalBoldStyle : 0;
                UIVertex.UV2.Y = TextureIndex;
                UIVertex.UV4.X = ScreenPxRange.X;
                UIVertex.UV4.Y = ScreenPxRange.Y;
                FastLinearColorToFColor(TextElement.Color, &UIVertex.Color);
                UIVertex.Position = TempVertices[VertexIndex];
                Vertices.Emplace(UIVertex);
            }
        }

        if (TextElement.Hypertext != TEXT(""))
        {
            if (HypertextList.Num() != 0)
            {
                FHypertextRegion& LastRegion = HypertextList[HypertextList.Num() - 1];
                if (FMathUtility::Approximately(LastRegion.TopRight.Y, LineElementPtr->TopY) && LastRegion.HypertextIndex == TextElement.HypertextIndex && LastRegion.Hypertext == TextElement.Hypertext)
                {
                    LastRegion.TopRight.X = X;
                }
                else
                {
                    FHypertextRegion HypertextRegion = FHypertextRegion();
                    HypertextRegion.BottomLeft.X = PrevX;
                    HypertextRegion.BottomLeft.Y = LineElementPtr->TopY - LineElementPtr->Height;
                    HypertextRegion.TopRight.X = X;
                    HypertextRegion.TopRight.Y = LineElementPtr->TopY;
                    HypertextRegion.Hypertext = TextElement.Hypertext;
                    HypertextRegion.HypertextIndex = TextElement.HypertextIndex;
                    HypertextList.Emplace(HypertextRegion);
                }
            }
            else
            {
                FHypertextRegion HypertextRegion = FHypertextRegion();
                HypertextRegion.BottomLeft.X = PrevX;
                HypertextRegion.BottomLeft.Y = LineElementPtr->TopY - LineElementPtr->Height;
                HypertextRegion.TopRight.X = X;
                HypertextRegion.TopRight.Y = LineElementPtr->TopY;
                HypertextRegion.Hypertext = TextElement.Hypertext;
                HypertextRegion.HypertextIndex = TextElement.HypertextIndex;
                HypertextList.Emplace(HypertextRegion);
            }
        }

        if (TextElement.bStrikeThrough)
        {
            FVector2D XScreenPxRange = FVector2D::ZeroVector;
            UpdateGlyph(TEXT('x'), TextElement.FontSize, LineElementPtr->MaxTextFontSize,
                EFontStyle::FontStyle_Normal, InvTextureSize, false, XScreenPxRange);

            const float CY = (GlyphInfo.V0.Y + GlyphInfo.V1.Y) * 0.5f;
            
            FVector2D DashScreenPxRange = FVector2D::ZeroVector;
            const int32 DashTextureIndex = UpdateGlyph(TEXT('_'), TextElement.FontSize, LineElementPtr->MaxTextFontSize,
                EFontStyle::FontStyle_Normal, InvTextureSize, false, DashScreenPxRange);

            const auto& Dash = GlyphInfo;

            const float CX = (Dash.U0.X + Dash.U2.X) * 0.5f;
            TempUVs[0].X = CX;
            TempUVs[0].Y = Dash.U0.Y;
            TempUVs[1].X = CX;
            TempUVs[1].Y = Dash.U2.Y;
            TempUVs[2].X = CX;
            TempUVs[2].Y = Dash.U2.Y;
            TempUVs[3].X = CX;
            TempUVs[3].Y = Dash.U0.Y;

            V0Y = CY + (Dash.V0.Y - Dash.V1.Y) * 0.5f;
            V1Y = CY + (Dash.V1.Y - Dash.V0.Y);

            if (TextElement.SubscriptMode == 1)
            {
                V0Y *= SizeShrinkage;
                V1Y *= SizeShrinkage;
                float YMoveDelta = (Baseline - BaselineOffset) * 0.33f + TextElement.FontSize * 0.075f;
                V0Y -= YMoveDelta;
                V1Y -= YMoveDelta;
            }
        	
            V0Y = -Y + V0Y;
            V1Y = -Y + V1Y;

            TempVertices[0].X = PrevX;
            TempVertices[0].Y = V0Y;
            TempVertices[1].X = PrevX;
            TempVertices[1].Y = V1Y;
            TempVertices[2].X = X;
            TempVertices[2].Y = V1Y;
            TempVertices[3].X = X;
            TempVertices[3].Y = V0Y;

            for (int32 VertexIndex = 0; VertexIndex < 4; ++VertexIndex)
            {
                FUIVertex UIVertex = FUIVertex::SimpleVertex;
                UIVertex.UV0 = TempUVs[VertexIndex];
                UIVertex.UV2.X = TextElement.bBold ? FinalBoldStyle : 0;
                UIVertex.UV2.Y = DashTextureIndex;
                UIVertex.UV4.X = DashScreenPxRange.X;
                UIVertex.UV4.Y = DashScreenPxRange.Y;
                FastLinearColorToFColor(TextElement.Color, &UIVertex.Color);
                UIVertex.Position = TempVertices[VertexIndex];
                Vertices.Emplace(UIVertex);
            }
        }
 
        if (TextElement.bUnderline)
        {
            int32 DashSize = LineElementPtr->MaxTextFontSize;
            if (TextElement.SubscriptMode == 1)
            {
                DashSize = TextElement.FontSize;
            }
            
            FVector2D DashScreenPxRange = FVector2D::ZeroVector;
            const int32 DashTextureIndex = UpdateGlyph(TEXT('_'), DashSize, LineElementPtr->MaxTextFontSize,
                EFontStyle::FontStyle_Normal, InvTextureSize, true, DashScreenPxRange);

        	const auto& Dash = GlyphInfo;
        	
            const float CX = (Dash.U0.X + Dash.U2.X) * 0.5f;
            TempUVs[0].X = CX;
            TempUVs[0].Y = Dash.U0.Y;
            TempUVs[1].X = CX;
            TempUVs[1].Y = Dash.U2.Y;
            TempUVs[2].X = CX;
            TempUVs[2].Y = Dash.U2.Y;
            TempUVs[3].X = CX;
            TempUVs[3].Y = Dash.U0.Y;

            const float OriginalUnderlineThickness = Dash.V0.Y - Dash.V1.Y;
            float UnderlineThickness = (Dash.V0.Y - Dash.V1.Y) * LastSettings.Font->UnderlineScale * LastSettings.UnderlineScale;
            V0Y = Dash.V0.Y + (UnderlineThickness - OriginalUnderlineThickness) * 0.5;
            V1Y = V0Y - UnderlineThickness;

            if (TextElement.SubscriptMode == 1)
            {
                V0Y *= SizeShrinkage;
                V1Y *= SizeShrinkage;
                float YMoveDelta = (Baseline - BaselineOffset) * 0.33f + TextElement.FontSize * 0.075f;
                V0Y -= YMoveDelta;
                V1Y -= YMoveDelta;
            }
        	
			V0Y = -Y + V0Y;
			V1Y = -Y + V1Y;

			TempVertices[0].X = PrevX;
			TempVertices[0].Y = V0Y;
			TempVertices[1].X = PrevX;
			TempVertices[1].Y = V1Y;
			TempVertices[2].X = X;
			TempVertices[2].Y = V1Y;
			TempVertices[3].X = X;
			TempVertices[3].Y = V0Y;

            for (int32 VertexIndex = 0; VertexIndex < 4; ++VertexIndex)
            {
                FUIVertex UIVertex = FUIVertex::SimpleVertex;
                UIVertex.UV0 = TempUVs[VertexIndex];
                UIVertex.UV2.X = TextElement.bBold ? FinalBoldStyle : 0;
                UIVertex.UV2.Y = DashTextureIndex;
                UIVertex.UV4.X = DashScreenPxRange.X;
                UIVertex.UV4.Y = DashScreenPxRange.Y;
                FastLinearColorToFColor(TextElement.Color, &UIVertex.Color);
                UIVertex.Position = TempVertices[VertexIndex];
                Vertices.Emplace(UIVertex);
            }
        }
    }

    // update invisible text element
    if (TextElementList.Num() > 0)
    {
        auto& LastTextElement = TextElementList[TextElementList.Num() - 1];
        LastTextElement.CursorPos.X = X;
        LastTextElement.CursorPos.Y = Y;
    }

    if (!IsLeftPivot(LastSettings.TextAnchor) && IndexOffset < Vertices.Num())
    {
        Align(IndexOffset, HypertextIndexOffset, EmojiIndexOffset, WidgetIndexOffset, X - FinalKerning, TextElementIndexOffset, TextElementList.Num());
    }

    const float WidthOffset = LastSettings.GenerationExtents.X * LastSettings.Pivot.X;
    const float HeightOffset = LastSettings.GenerationExtents.Y -
        LastSettings.GenerationExtents.Y * LastSettings.Pivot.Y;

	const FVector2D AnchorPivot = GetTextAnchorPivot(LastSettings.TextAnchor);
    const float PivotOffsetY = FMath::Lerp(PrintedSize.Y - LastSettings.GenerationExtents.Y,
        0.0f, AnchorPivot.Y);
        
    for (int32 Index = 0, Count = Vertices.Num(); Index < Count; ++Index)
    {
        FUIVertex& UIVertex = Vertices[Index];
        UIVertex.Position.X -= WidthOffset;
        UIVertex.Position.Y += HeightOffset + PivotOffsetY;
    }

    for (int32 Index = 0, Count = HypertextList.Num(); Index < Count; ++Index)
    {
        FHypertextRegion& Region = HypertextList[Index];
        Region.BottomLeft.X -= WidthOffset;
        Region.BottomLeft.Y += HeightOffset + PivotOffsetY;

        Region.TopRight.X -= WidthOffset;
        Region.TopRight.Y += HeightOffset + PivotOffsetY;
    }

    for (int32 Index = 0, Count = EmojiList.Num(); Index < Count; ++Index)
    {
        FEmojiRegion& Region = EmojiList[Index];
        Region.BottomLeft.X -= WidthOffset;
        Region.BottomLeft.Y += HeightOffset + PivotOffsetY;

        Region.TopRight.X -= WidthOffset;
        Region.TopRight.Y += HeightOffset + PivotOffsetY;
    }

    for (int32 Index = 0, Count = WidgetList.Num(); Index < Count; ++Index)
    {
        FWidgetRegion& Region = WidgetList[Index];
        Region.BottomLeft.X -= WidthOffset;
        Region.BottomLeft.Y += HeightOffset + PivotOffsetY;

        Region.TopRight.X -= WidthOffset;
        Region.TopRight.Y += HeightOffset + PivotOffsetY;
    }

    for (int32 Index = 0, Count = AttachWidgetList.Num(); Index < Count; ++Index)
    {
        FWidgetRegion& Region = AttachWidgetList[Index];
        Region.BottomLeft.X -= WidthOffset;
        Region.BottomLeft.Y += HeightOffset + PivotOffsetY;

        Region.TopRight.X -= WidthOffset;
        Region.TopRight.Y += HeightOffset + PivotOffsetY;
    }

    Characters.Empty();
    if (bGenerateCharacterAndLineInfo)
    {
        Characters.Reserve(TextElementList.Num());
        FPlatformMisc::Prefetch(Characters.GetData());
        
        for (int32 Index = 0, Count = TextElementList.Num(); Index < Count; ++Index)
        {
            const FTextElement& TextElem = TextElementList[Index];

            FUICharInfo CharInfo;
            CharInfo.CharWidth = TextElem.CharWidth;
            CharInfo.CursorPos.X = TextElem.CursorPos.X - WidthOffset;
            CharInfo.CursorPos.Y = TextElem.CursorPos.Y + HeightOffset + PivotOffsetY;
            Characters.Emplace(CharInfo);
        }
    }

    Lines.Empty();
    if (bGenerateCharacterAndLineInfo)
    {
        Lines.Reserve(LineElementList.Num());
        FPlatformMisc::Prefetch(Lines.GetData());
        
        for (int32 Index = 0, Count = LineElementList.Num(); Index < Count; ++Index)
        {
            const FLineElement& LineElem = LineElementList[Index];

            FUILineInfo LineInfo;
            LineInfo.StartCharIdx = LineElem.StartCharIdx;
            LineInfo.Height = LineElem.Height;
            LineInfo.TopY = LineElem.TopY + HeightOffset + PivotOffsetY;
            Lines.Emplace(LineInfo);
        }
    }

    TextElementList.Reset();
    LineElementList.Reset();

	return true;
}

void FTextGenerator::Align(int32 IndexOffset, int32 HypertextIndexOffset, int32 EmojiIndexOffset, int32 WidgetIndexOffset, float PrintedWidth, int32 TextElementIndexOffset, int32 MaxTextElementIndex)
{
    float Padding = 0;
    
    const float RectWidth = LastSettings.GenerationExtents.X;
    switch (LastSettings.TextAnchor)
    {
    case ETextAnchor::TextAnchor_LowerRight:
    case ETextAnchor::TextAnchor_MiddleRight:
    case ETextAnchor::TextAnchor_UpperRight:
        {
            Padding = RectWidth - PrintedWidth;
            for (int32 Index = IndexOffset, Count = Vertices.Num(); Index < Count; ++Index)
            {
                FUIVertex& UIVertex = Vertices[Index];
                UIVertex.Position.X += Padding;
            }

            for (int32 Index = HypertextIndexOffset, Count = HypertextList.Num(); Index < Count; ++Index)
            {
                FHypertextRegion& Region = HypertextList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = EmojiIndexOffset, Count = EmojiList.Num(); Index < Count; ++Index)
            {
                FEmojiRegion& Region = EmojiList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = WidgetIndexOffset, Count = WidgetList.Num(); Index < Count; ++Index)
            {
                FWidgetRegion& Region = WidgetList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = WidgetIndexOffset, Count = AttachWidgetList.Num(); Index < Count; ++Index)
            {
                FWidgetRegion& Region = AttachWidgetList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }
            break;
        }
    case ETextAnchor::TextAnchor_LowerCenter:
    case ETextAnchor::TextAnchor_MiddleCenter:
    case ETextAnchor::TextAnchor_UpperCenter:
        {
            Padding = (RectWidth - PrintedWidth) * 0.5f;
            const int32 Diff = FMath::RoundToInt(RectWidth - PrintedWidth);
            const int32 IntWidth = FMath::RoundToInt(RectWidth);

            const bool bOddDiff = (Diff & 1) == 1;
            const bool bOddWidth = (IntWidth & 1) == 1;
            if ((bOddDiff && !bOddWidth) || (!bOddDiff && bOddWidth))
                Padding += 0.5f;

            for (int32 Index = IndexOffset, Count = Vertices.Num(); Index < Count; ++Index)
            {
                FUIVertex& UIVertex = Vertices[Index];
                UIVertex.Position.X += Padding;
            }

            for (int32 Index = HypertextIndexOffset, Count = HypertextList.Num(); Index < Count; ++Index)
            {
                FHypertextRegion& Region = HypertextList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = EmojiIndexOffset, Count = EmojiList.Num(); Index < Count; ++Index)
            {
                FEmojiRegion& Region = EmojiList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = WidgetIndexOffset, Count = WidgetList.Num(); Index < Count; ++Index)
            {
                FWidgetRegion& Region = WidgetList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }

            for (int32 Index = WidgetIndexOffset, Count = AttachWidgetList.Num(); Index < Count; ++Index)
            {
                FWidgetRegion& Region = AttachWidgetList[Index];
                Region.BottomLeft.X += Padding;
                Region.TopRight.X += Padding;
            }    
            break;
        }
    default:
        ;
    }

    if (bGenerateCharacterAndLineInfo)
    {
        for (int32 Index = TextElementIndexOffset; Index < MaxTextElementIndex; ++Index)
        {
            FTextElement& TextElement = TextElementList[Index];
            TextElement.CursorPos.X += Padding;   
        }
    }
}

bool FTextGenerator::ParseSymbol(const FString& Text, const FTextGenerationSettings& Setting, int32& Index, int32& CurFontSize, int32 DefaultFontSize, TArray<FLinearColor>& Colors, 
	bool& bColorChanged, int8& Sub, bool& bBold, bool& bItalic, bool& bUnderline, bool& bStrike, FString& Hypertext, int32& HypertextIndex, bool& bEmoji, FName& EmojiSymbol, bool& bWidget, FName& WidgetSymbol)
{
    const int32 Length = Text.Len();

    if (Index + 3 > Length || Text[Index] != TEXT('<')) return false;

    if (Text[Index + 2] == TEXT('>'))
    {
        const FString Sub3 = Text.Mid(Index, 3);
        if (Sub3 == TEXT("<b>"))
        {
            bBold = true;
            Index += 3;
            return true;
        }

        if (Sub3 == TEXT("<i>"))
        {
            bItalic = true;
            Index += 3;
            return true;
        }

        if (Sub3 == TEXT("<u>"))
        {
            bUnderline = true;
            Index += 3;
            return true;
        }

        if (Sub3 == TEXT("<s>"))
        {
            bStrike = true;
            Index += 3;
            return true;
        }
    }

    if (Index + 4 > Length) return false;

    if (Text[Index + 3] == '>')
    {
        const FString Sub4 = Text.Mid(Index, 4);
        if (Sub4 == TEXT("</b>"))
        {
            bBold = false;
            Index += 4;
            return true;
        }

        if (Sub4 == TEXT("</i>"))
        {
            bItalic = false;
            Index += 4;
            return true;
        }

        if (Sub4 == TEXT("</u>"))
        {
            bUnderline = false;
            Index += 4;
            return true;
        }

        if (Sub4 == TEXT("</s>"))
        {
            bStrike = false;
            Index += 4;
            return true;
        }

        if (Sub4 == TEXT("</a>"))
        {
            Hypertext = TEXT("");
            Index += 4;
            return true;
        }
    }

    const auto Href = Text.Mid(Index, 3);
    if (Href == TEXT("<a="))
    {
        FString HypertextStr;
        for (int32 SuffixIndex = Index + 3; SuffixIndex < Length; ++SuffixIndex)
        {
            if (Text[SuffixIndex] == TEXT('>'))
            {
                break;
            }

            HypertextStr.AppendChar(Text[SuffixIndex]);
        }
        ++HypertextIndex;
        Hypertext = HypertextStr;
        Index += 4 + HypertextStr.Len();
        return true;
    }

    if (Index + 5 > Length) return false;

    if (Text[Index + 4] == TEXT('>'))
    {
        const FString Sub5 = Text.Mid(Index, 5);
        if (Sub5 == TEXT("<sub>"))
        {
            Sub = 1;
            Index += 5;
            return true;
        }

        if (Sub5 == TEXT("<sup>"))
        {
            Sub = 2;
            Index += 5;
            return true;
        }
    }

    if (Index + 6 > Length) return false;

    if (Text[Index + 5] == TEXT('>'))
    {
        const FString Sub6 = Text.Mid(Index, 6);
        if (Sub6 == TEXT("</sub>"))
        {
            if (Sub == 1)
                Sub = 0;
            Index += 6;
            return true;
        }
    	
    	if(Sub6 == TEXT("</sup>"))
        {
            if (Sub == 2)
                Sub = 0;
            Index += 6;
            return true;
        }
    }
	
    if (Index + 7 > Length) return false;

    if (Text[Index + 6] == TEXT('>'))
    {
        const FString Sub7 = Text.Mid(Index, 7);
    	if (Sub7 == TEXT("</size>"))
    	{
            Index += 7;
            CurFontSize = DefaultFontSize;
            return true;
    	}
    }

    if (Index + 8 > Length) return false;

    if (Text[Index + 7] == TEXT('>'))
    {
        const FString Sub8 = Text.Mid(Index, 8);
        if (Sub8 == TEXT("</color>"))
        {
            Index += 8;
            if (Colors.Num() > 0)
            {
                bColorChanged = true;
                Colors.RemoveAt(Colors.Num() - 1, 1, false);
            }
            return true;
        }
    }

    const auto SubSize = Text.Mid(Index, 6);
    if (SubSize == TEXT("<size="))
    {
        FString SizeStr;
        for (int32 SuffixIndex = Index + 6; SuffixIndex < Length; ++SuffixIndex)
        {
            if (Text[SuffixIndex] == TEXT('>'))
            {
                break;
            }
            SizeStr.AppendChar(Text[SuffixIndex]);
        }
	
        const int32 FontSize = FCString::Atoi(*SizeStr);
        if (FontSize > 0)
        {
            CurFontSize = FontSize;
            Index += 7 + SizeStr.Len();
            return true;
        }
    }

    if (Index + 9 > Length) return false;

    const auto SubString = Text.Mid(Index, 7);
    if (SubString == TEXT("<emoji="))
    {
        FString EmojiStr;
        bool bIsEnd = false;
        for (int32 SuffixIndex = Index + 7; SuffixIndex < Length; ++SuffixIndex)
        {
            if (Text[SuffixIndex] == TEXT('>'))
            {
                bIsEnd = true;
                break;
            }
            EmojiStr.AppendChar(Text[SuffixIndex]);
        }
	    
        if (bIsEnd == true)
        {
            bEmoji = true;
            EmojiSymbol = FName(EmojiStr);
            Index += 8 + EmojiStr.Len();
        }
        else
        {
            return false;
        }
	    
        return true;
    }
    
    const auto WidgetString = Text.Mid(Index, 8);
    if (WidgetString == TEXT("<widget="))
    {
        FString WidgetStr;
        bool bIsEnd = false;
        for (int32 SuffixIndex = Index + 8; SuffixIndex < Length; ++SuffixIndex)
        {
            if (Text[SuffixIndex] == TEXT('>'))
            {
                bIsEnd = true;
                break;
            }
            WidgetStr.AppendChar(Text[SuffixIndex]);
        }
	    
        if (bIsEnd == true)
        {
            bWidget = true;
            WidgetSymbol = FName(WidgetStr);
            Index += 9 + WidgetStr.Len();
        }
        else
        {
            return false;
        }
	    
        return true;
    }

    if (SubString == TEXT("<color="))
    {
        FString ColorStr;
        bool bIsEnd = false;
        for (int32 SuffixIndex = Index + 7; SuffixIndex < Length; ++SuffixIndex)
        {
            if (Text[SuffixIndex] == TEXT('>'))
            {
                bIsEnd = true;
                break;
            }
            ColorStr.AppendChar(Text[SuffixIndex]);
        }

    	if (bIsEnd)
    	{
    	    bColorChanged = true;        
    	    const auto ColorHex = UUGUISettings::Get()->TextColorTags.Find(FName(ColorStr));
    	    if (ColorHex != nullptr)
    	    {
    	        Colors.Emplace(*ColorHex);
    	    }
    	    else
    	    {
    	        Colors.Emplace(FColor::FromHex(ColorStr).ReinterpretAsLinear());
    	    }        
    	    Index += 8 + ColorStr.Len();
    	}
    	else
    	{
    	    return false;
    	}

        return true;
    }

    return false;
}

/////////////////////////////////////////////////////
