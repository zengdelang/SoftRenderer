#include "Core/Widgets/Text/SDFFont.h"

/////////////////////////////////////////////////////
// USDFFont

USDFFont::USDFFont(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AtlasWidth = 2048;
	AtlasHeight = 2048;
	Kerning = 0;
	ImportFontSize = 32;
	LineHeight = 36;
	Baseline = 33;
	BaselineOffset = 0;
	UnderlineThickness = 1.6;
	UnderlineY = -3.6;
	FontTextureArray = nullptr;
	BoldStyle = 0.23f;
	BoldSpacingScale = 0.1f;
	UnderlineScale = 1;
	NonTextScale = 1;
	NotDefineCharIndex = -1;
}

int32 USDFFont::RemapChar(TCHAR CharCode, bool& bUseNormalStyle, EFontStyle InFontStyle)
{
	const uint16 UCode = CharCast<UCS2CHAR>(CharCode);
	const uint32* CharIndexPtr = nullptr;

	switch (InFontStyle)
	{
	case EFontStyle::FontStyle_Normal:
		bUseNormalStyle = true;
		CharIndexPtr = NormalCharRemap.Find(UCode);
		break;
	case EFontStyle::FontStyle_Bold:
		bUseNormalStyle = false;
		CharIndexPtr = BoldCharRemap.Find(UCode);
		break;
	case EFontStyle::FontStyle_Italic:
		bUseNormalStyle = false;
		CharIndexPtr = ItalicCharRemap.Find(UCode);
		break;
	case EFontStyle::FontStyle_BoldAndItalic:
		bUseNormalStyle = false;
		CharIndexPtr = BoldAndItalicCharRemap.Find(UCode);
		break;
	}

	if (InFontStyle != EFontStyle::FontStyle_Normal && CharIndexPtr == nullptr)
	{
		bUseNormalStyle = true;
		CharIndexPtr = NormalCharRemap.Find(UCode);
	}
	
	if (CharIndexPtr == nullptr)
	{
		return NotDefineCharIndex;
	}
	return *CharIndexPtr;
}

void USDFFont::UpdateScreenPxRange(const FSDFFontCharacter& FontChar, FVector2D& ScreenPxRange)
{
	if (ScreenPxRanges.IsValidIndex(FontChar.ScreenPxRangeIndex))
	{
		ScreenPxRange = ScreenPxRanges[FontChar.ScreenPxRangeIndex];
		return;
	}

	ScreenPxRange.X = 0.001953f;
	ScreenPxRange.Y = 0.001953f;
}

#if WITH_EDITORONLY_DATA

void USDFFont::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(USDFFont, MaxAtlasSize))
	{
		MaxAtlasSize.X = FMath::RoundUpToPowerOfTwo(MaxAtlasSize.X);
		MaxAtlasSize.Y = FMath::RoundUpToPowerOfTwo(MaxAtlasSize.Y);
	}
	
	UObject::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

/////////////////////////////////////////////////////
