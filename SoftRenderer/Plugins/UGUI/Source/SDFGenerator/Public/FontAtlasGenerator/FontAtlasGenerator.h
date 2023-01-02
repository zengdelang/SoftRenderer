#pragma once

#include "ShapeGenerator/FontShapeGenerator.h"

#if WITH_FREETYPE

namespace FontAtlasGenerator
{
	class FFontHolder
	{
		SDFGenerator::FFreetypeHandle* FreeType;
		SDFGenerator::FFontHandle* Font;
		FString FontFilename;
		int32 FontFaceIndex;

	public:
		SDFGENERATOR_API FFontHolder();

		SDFGENERATOR_API ~FFontHolder();

		bool Load(const FString& InFontFilename, int32 FaceIndex);

		SDFGENERATOR_API bool GetFaceName(const FString& InFontFilename, FString& FamilyName, FString& StyleName, int32 FaceIndex);

		SDFGENERATOR_API int32 GetFaceCount(const FString& InFontFilename) const;
		
		operator SDFGenerator::FFontHandle*() const
		{
			return Font;
		}
	};

	struct SDFGENERATOR_API FFontCharacter
	{
	public:
		float StartU;
		
		float StartV;
		
		float USize;
		
		float VSize;
		
		float Advance;
		
		float HorizontalOffset;
		
		float AscenderY;
		
		float Scale;
		
		uint8 TextureIndex;
		
		uint32 ScreenPxRangeIndex;

	public:
		FFontCharacter()
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

	struct FFont
	{
	public:
		int32 AtlasWidth;
		
		int32 AtlasHeight;
		
		float ImportFontSize;

		float LineHeight;

		float Baseline;

		float BaselineOffset;
		
		float UnderlineY;

		float UnderlineThickness;
		
		int32 NotDefineCharIndex = -1;
		
		TArray<FFontCharacter> Characters;
		
		TArray<FVector2D> ScreenPxRanges;

	public:
		TMap<uint32, uint32> NormalCharRemap;
		TMap<uint32, uint32> BoldCharRemap;
		TMap<uint32, uint32> ItalicCharRemap;
		TMap<uint32, uint32> BoldAndItalicCharRemap;
		
	};
	
	enum class EGlyphIdentifierType : uint8
	{
		UNICODE_CODEPOINT,
		GLYPH_INDEX,
	};

	enum class EFontStyle : uint8
	{
		Normal,
		Bold,
		Italic,
		BoldAndItalic,
	};
	
	struct FFontInput
	{
		FString CharsetName;
		FString FontFilename;
		EGlyphIdentifierType GlyphIdentifierType;
		FString Charset;
		EFontStyle FontStyle;
		int32 FaceIndex;
		double FontScale;
		double PxRange;

		int32 PxRangeIndex;
		
	public:
		FFontInput() : GlyphIdentifierType(EGlyphIdentifierType::UNICODE_CODEPOINT), FontStyle(EFontStyle::Normal),
		               FaceIndex(0), FontScale(0), PxRange(1), PxRangeIndex(0)
		{
		}
	};

	class SDFGENERATOR_API ISDFGeneratorLogListener
	{
	public:
		virtual ~ISDFGeneratorLogListener() = default;
		
		virtual void AddWarningMessage(FText Msg) = 0;
		virtual void AddErrorMessage(FText Msg) = 0;
		
		virtual void BeginSlowTask(bool bShowCancelButton, bool bShowProgressDialog) = 0;
		virtual void UpdateProgress(int32 CurIndex, int32 Count) = 0;
		virtual void EndSlowTask() = 0;
	};

	bool SDFGENERATOR_API GenerateFontAtlas(TArray<FFontInput>& FontInputs, class FFontAtlasPacker& AtlasPacker, FFont& FontInfo, FVector2D MaxAtlasSize,
		bool bOverlapSupport = false, bool bScanlinePass = false, ISDFGeneratorLogListener* LogListener = nullptr, float CustomBaselineOffset = 0);

}

#endif