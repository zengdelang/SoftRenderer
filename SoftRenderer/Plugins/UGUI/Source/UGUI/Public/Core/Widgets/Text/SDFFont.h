#pragma once

#include "CoreMinimal.h"
#include "SDFFontCharset.h"
#include "Core/Widgets/Text/TextFontStyle.h"
#include "Engine/Texture2DArray.h"
#include "SDFFont.generated.h"

USTRUCT()
struct FSDFFontCharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float StartU;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float StartV;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float USize;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float VSize;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float Advance;
	
	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float HorizontalOffset;
	
	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float AscenderY;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	float Scale;
	
	UPROPERTY(EditAnywhere, Category = FontCharacter)
	uint8 TextureIndex;

	UPROPERTY(EditAnywhere, Category = FontCharacter)
	uint16 ScreenPxRangeIndex;

public:
	FSDFFontCharacter()
		: StartU(0)
		, StartV(0)
		, USize(0)
		, VSize(0)
		, Advance(0)
		, HorizontalOffset(0)
		, AscenderY(0)
		, Scale(1)
		, TextureIndex(0)
		, ScreenPxRangeIndex(0)
	{
		
	}
};

UCLASS(BlueprintType)
class UGUI_API USDFFont : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = Font)
	int32 AtlasWidth;

	UPROPERTY(VisibleAnywhere, Category = Font)
	int32 AtlasHeight;

	UPROPERTY(EditAnywhere, Category = Font)
	int32 Kerning;

	UPROPERTY(EditAnywhere, Category = Font)
	float ImportFontSize;

	UPROPERTY(EditAnywhere, Category = Font)
	float LineHeight;

	UPROPERTY(EditAnywhere, Category = Font)
	float Baseline;

	UPROPERTY()
	float BaselineOffset;

	UPROPERTY(EditAnywhere,Category = Font)
	float UnderlineY;

	UPROPERTY(EditAnywhere, Category = Font)
	float UnderlineThickness;

	UPROPERTY(EditAnywhere, Category = Font, meta = (ClampMin = "0", ClampMax = "0.5", UIMin = "0", UIMax = "0.5"))
	float BoldStyle;
	
	UPROPERTY(EditAnywhere, Category = Font)
	float BoldSpacingScale;
	
	UPROPERTY(EditAnywhere, Category = Font, meta = (ClampMin = "1.0", UIMin = "1.0"))
	float UnderlineScale;

	UPROPERTY(EditAnywhere, Category = Font)
	float NonTextScale;
	
	UPROPERTY(VisibleAnywhere, Category = Font)
	int32 NotDefineCharIndex;
	
	UPROPERTY(/*EditAnywhere, Category = Font*/)
	TArray<FSDFFontCharacter> Characters;

	UPROPERTY(EditAnywhere, Category = Font)
	UTexture2DArray* FontTextureArray;

	UPROPERTY(EditAnywhere, Category = Font)
	TArray<FVector2D> ScreenPxRanges;

public:
	UPROPERTY(/*EditAnywhere, Category = Font*/)
	TMap<uint16, uint32> NormalCharRemap;

	UPROPERTY(/*EditAnywhere, Category = Font*/)
	TMap<uint16, uint32> BoldCharRemap;
	
	UPROPERTY(/*EditAnywhere, Category = Font*/)
	TMap<uint16, uint32> ItalicCharRemap;

	UPROPERTY(/*EditAnywhere, Category = Font*/)
	TMap<uint16, uint32> BoldAndItalicCharRemap;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced)
	TArray<USDFFontCharset*> Charsets;

	UPROPERTY(EditAnywhere, Category = "SDF Generator",  meta = (ClampMin = "32", ClampMax = "4096", UIMin = "32", UIMax = "4096"))
	FVector2D MaxAtlasSize = FVector2D(2048, 2048);

	UPROPERTY(EditAnywhere, Category = "SDF Generator")
	bool bOverlapSupport = true;

	UPROPERTY(EditAnywhere, Category = "SDF Generator")
	bool bScanlinePass = true;

	UPROPERTY(EditAnywhere, Category = "SDF Generator")
	float CustomBaselineOffset = 0;
#endif

public:
	int32 RemapChar(TCHAR CharCode, bool& bUseNormalStyle, EFontStyle InFontStyle = EFontStyle::FontStyle_Normal);
	void UpdateScreenPxRange(const FSDFFontCharacter& FontChar, FVector2D& ScreenPxRange);

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
