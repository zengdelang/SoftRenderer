#include "FontAtlasGenerator/FontAtlasGenerator.h"
#include "FontAtlasGenerator/Charset.h"
#include "FontAtlasGenerator/FontGeometry.h"
#include "FontAtlasGenerator/FontAtlasPacker.h"
#include "ShapeGenerator/FontShapeGenerator.h"

#define LOCTEXT_NAMESPACE "FontAtlasGenerator"

#if WITH_FREETYPE

namespace FontAtlasGenerator
{
	FFontHolder::FFontHolder() : FreeType(SDFGenerator::InitializeFreetype()), Font(nullptr), FontFaceIndex(0)
	{
		
	}

	FFontHolder::~FFontHolder()
	{
		if (FreeType)
		{
			if (Font)
			{
				SDFGenerator::DestroyFont(Font);
			}
			SDFGenerator::DeinitializeFreetype(FreeType);
		}
	}

	bool FFontHolder::Load(const FString& InFontFilename, int32 FaceIndex)
	{
		if (FreeType && !InFontFilename.IsEmpty())
		{
			if (FontFilename.Equals(InFontFilename) && FontFaceIndex == FaceIndex)
				return true;
				
			if (Font)
			{
				SDFGenerator::DestroyFont(Font);
			}

			if (FaceIndex > 0)
			{
				FaceIndex = FMath::Min(FaceIndex, SDFGenerator::GetFaceNum(FreeType, TCHAR_TO_UTF8(*InFontFilename)));
			}
				
			if ((Font = SDFGenerator::LoadFont(FreeType, TCHAR_TO_UTF8(*InFontFilename), FaceIndex)))
			{
				FontFaceIndex = FaceIndex;
				FontFilename = InFontFilename;
				return true;
			}

			FontFaceIndex = FaceIndex;
			FontFilename = InFontFilename;
		}
			
		return false;
	}
	
	bool FFontHolder::GetFaceName(const FString& InFontFilename, FString& FamilyName, FString& StyleName, int32 FaceIndex)
	{
		if (FreeType)
		{
			if (Font)
			{
				SDFGenerator::DestroyFont(Font);
			}
				
			if ((Font = SDFGenerator::GetFaceName(FreeType, TCHAR_TO_UTF8(*InFontFilename), FamilyName, StyleName, FaceIndex)))
			{
				return true;
			}
		}
			
		return false;
	}

	int32 FFontHolder::GetFaceCount(const FString& InFontFilename) const
	{
		return SDFGenerator::GetFaceNum(FreeType, TCHAR_TO_UTF8(*InFontFilename));
	}
	
	
	bool GenerateFontAtlas(TArray<FFontInput>& FontInputs, FFontAtlasPacker& AtlasPacker, FFont& FontInfo, FVector2D MaxAtlasSize, bool bOverlapSupport, bool bScanlinePass, ISDFGeneratorLogListener* LogListener, float CustomBaselineOffset)
	{
		FCharsetCollector CharsetCollector;
		FFontHolder FontHolder;
		
		TArray<FGlyphGeometry*> GlyphGeometries;
		TArray<FGlyphGeometry*> RectangleGlyphs;
		TArray<FFontGeometry> FontGeometries;
		FontGeometries.SetNum(FontInputs.Num());

		int32 Index = -1;
		for (auto& FontInput : FontInputs)
		{
			++Index;
			
			if (!FontHolder.Load(FontInput.FontFilename, FontInput.FaceIndex))
			{
				if (LogListener)
				{
					LogListener->AddWarningMessage(FText::Format(LOCTEXT("LoadFontError", "Failed to load specified font file. font filename : {0}."), FText::FromString(FontInput.FontFilename)));
				}
				continue;
			}

			if (FontInput.FontScale <= 0)
				FontInput.FontScale = 1;
			
			// Load character set
			FCharset Charset;
			Charset.FontStyle = FontInput.FontStyle;
			Charset.CharsetCollector = &CharsetCollector;
			if (!FontInput.Charset.IsEmpty())
			{
				if (!Charset.Load(FontInput.Charset, FontInput.GlyphIdentifierType != EGlyphIdentifierType::UNICODE_CODEPOINT))
				{
					if (LogListener)
					{
						if (FontInput.GlyphIdentifierType == EGlyphIdentifierType::GLYPH_INDEX)
						{
							LogListener->AddWarningMessage(FText::Format(LOCTEXT("LoadGlyphCharsetError", "Failed to load glyph set specification. Charset Name : {0}, CharsetStr : {1}"),
								FText::FromString(FontInput.CharsetName), FText::FromString(FontInput.Charset)));
						}
						else
						{
							LogListener->AddWarningMessage(FText::Format(LOCTEXT("LoadUnicodeCharsetError", "Failed to load character set specification. Charset Name : {0}, CharsetStr : {1}"),
								FText::FromString(FontInput.CharsetName), FText::FromString(FontInput.Charset)));
						}
					}
				}
			}
			else
			{
				Charset = FCharset::ASCII;
				FontInput.GlyphIdentifierType = EGlyphIdentifierType::UNICODE_CODEPOINT;
			}

			if (Charset.GetCharsetString().Len() == 0)
				continue;
			
			// Load glyphs
			FFontGeometry& FontGeometry = FontGeometries[Index];
			int32 GlyphsLoaded = 0;
			switch (FontInput.GlyphIdentifierType)
			{
			    case EGlyphIdentifierType::UNICODE_CODEPOINT:
			        GlyphsLoaded = FontGeometry.LoadCharset(FontHolder, FontInput.FontScale, Charset, FontInput.FontStyle);
			        break;
		        default:
		        	;
			}

			if (GlyphsLoaded > 0)
			{
				GlyphGeometries.Reserve(GlyphGeometries.Num() + FontGeometry.GetGlyphs().Num());
				RectangleGlyphs.Reserve(RectangleGlyphs.Num() + FontGeometry.GetGlyphs().Num());
				
				for (auto& Glyph : FontGeometry.GetGlyphs())
				{
					GlyphGeometries.Add(&Glyph);
					Glyph.FontStyle = FontInput.FontStyle;
					if (!Glyph.IsWhitespace())
					{
						RectangleGlyphs.Add(&Glyph);
						Glyph.WrapBox(1, FMath::Max(1.0, FontInput.PxRange), 0);
					}
				}
			}

			CharsetCollector.Charsets.Emplace(MoveTemp(Charset));
		}

		if (GlyphGeometries.Num() == 0)
		{
			if (LogListener)
			{
				LogListener->AddErrorMessage(LOCTEXT("NoGlyphs", "No glyphs loaded."));
			}
			return true;
		}

		if (LogListener)
		{
			LogListener->BeginSlowTask(false, RectangleGlyphs.Num() > 0);
		}

		int32 CurIndex= 0;
		AtlasPacker.Pack(RectangleGlyphs, bOverlapSupport, bScanlinePass, MaxAtlasSize, CurIndex, RectangleGlyphs.Num(), LogListener);

		if (LogListener)
		{
			LogListener->EndSlowTask();
		}
		
		FontInfo.AtlasWidth = AtlasPacker.GetAtlasWidth();
		FontInfo.AtlasHeight = AtlasPacker.GetAtlasHeight();

		TArray<FVector2D> ScreenPxRanges;
		TMap<int32, int32> PxRangMap;
		
		float MaxImportFontSize = 0;
		int32 FontIndex = 0;
		for (auto& FontGeometry : FontGeometries)
		{
			if (FontGeometry.GetGlyphs().Num() > 0)
			{
				int32 PxRangeInt = (int32)FontInputs[FontIndex].PxRange;
				int32* PxRangeIndexPtr = PxRangMap.Find(PxRangeInt);
				if (PxRangeIndexPtr)
				{
					FontInputs[FontIndex].PxRangeIndex = *PxRangeIndexPtr;
				}
				else
				{
					ScreenPxRanges.Add(FVector2D(FontInputs[FontIndex].PxRange / FontInfo.AtlasWidth, FontInputs[FontIndex].PxRange / FontInfo.AtlasHeight));
					PxRangMap.Add(PxRangeInt, ScreenPxRanges.Num() - 1);
					FontInputs[FontIndex].PxRangeIndex = ScreenPxRanges.Num() - 1;
				}
				
				auto& Metrics = FontGeometry.GetMetrics();
				if (MaxImportFontSize < Metrics.EmSize)
				{
					MaxImportFontSize = Metrics.EmSize;
				}
			}

			++FontIndex;
		}
		
		float MaxLineHeight = 0;
		float MaxBaseline = 0;
		float MaxUnderlineY = 0;
		float MaxUnderlineThickness = 0;
		for (auto& FontGeometry : FontGeometries)
		{
			if (FontGeometry.GetGlyphs().Num() > 0)
			{
				auto& Metrics = FontGeometry.GetMetrics();

				const float CurFontSize = Metrics.EmSize;
				
				const float CurLineHeight = MaxImportFontSize / CurFontSize * Metrics.LineHeight;
				if (MaxLineHeight < CurLineHeight)
				{
					MaxLineHeight = CurLineHeight;
				}

				const float CurAscenderY = MaxImportFontSize / CurFontSize * Metrics.AscenderY;
				const float CurDescenderY = MaxImportFontSize / CurFontSize * Metrics.DescenderY;
				float CurLineGap = CurLineHeight - CurAscenderY + CurDescenderY;

				const float CurBaseline = CurLineGap + CurAscenderY;
				if (MaxBaseline < CurBaseline)
				{
					MaxBaseline = CurBaseline;
				}

				const float CurUnderline = MaxImportFontSize / CurFontSize * Metrics.UnderlineY;
				if (MaxUnderlineY > CurUnderline)
				{
					MaxUnderlineY = CurUnderline;
				}
				
				const float CurUnderlineThickness = MaxImportFontSize / CurFontSize * Metrics.UnderlineThickness;
				if (MaxUnderlineThickness < CurUnderlineThickness)
				{
					MaxUnderlineThickness = CurUnderlineThickness;
				}
			}
		}

		FontInfo.ImportFontSize = MaxImportFontSize;
		FontInfo.LineHeight = MaxLineHeight;
		FontInfo.Baseline = MaxBaseline;
		FontInfo.BaselineOffset = 0;
		FontInfo.UnderlineY = MaxUnderlineY;
		FontInfo.UnderlineThickness = MaxUnderlineThickness;
		FontInfo.ScreenPxRanges = ScreenPxRanges;
		
		uint32 CharacterIndex = 0;
		FontIndex = 0;
		for (auto& FontGeometry : FontGeometries)
		{
			if (FontGeometry.GetGlyphs().Num() > 0)
			{
				auto& Metrics = FontGeometry.GetMetrics();
				
				for (auto& Glyph : FontGeometry.GetGlyphs())
				{
					int32 Unicode = Glyph.GetCodepoint();
					
					FFontCharacter FontCharacter;

					int32 X;
					int32 Y;
					int32 W;
					int32 H;
					Glyph.GetBoxRect(X, Y, W, H);
					FontCharacter.StartU = X;
					FontCharacter.StartV = Y;
					FontCharacter.USize = W;
					FontCharacter.VSize = H;
					
					FontCharacter.Scale = MaxImportFontSize / Metrics.EmSize;
					FontCharacter.TextureIndex = Glyph.TextureIndex;
					
					FontCharacter.HorizontalOffset = Glyph.GetHoriBearingX() - (W - Glyph.GetWidth()) * 0.5;
					FontCharacter.AscenderY = Glyph.GetHoriBearingY() + (H - Glyph.GetHeight()) * 0.5f;
					
					FontCharacter.Advance = Glyph.GetAdvance();
					
					FontCharacter.ScreenPxRangeIndex = FontInputs[FontIndex].PxRangeIndex;

					if (Unicode == 127)
					{
						FontInfo.NotDefineCharIndex = CharacterIndex;
					}
					else if (Unicode == 88) // X
					{
						FontInfo.BaselineOffset = (Glyph.GetHeight() - Glyph.GetHoriBearingY() + CustomBaselineOffset) / FontCharacter.Scale;
					}

					FontInfo.Characters.Emplace(FontCharacter);

					switch (Glyph.FontStyle)
					{
					case EFontStyle::Normal:
						FontInfo.NormalCharRemap.Add(Unicode, CharacterIndex);
						break;
					case EFontStyle::Bold:
						FontInfo.BoldCharRemap.Add(Unicode, CharacterIndex);
						break;
					case EFontStyle::Italic:
						FontInfo.ItalicCharRemap.Add(Unicode, CharacterIndex);
						break;
					case EFontStyle::BoldAndItalic:
						FontInfo.BoldAndItalicCharRemap.Add(Unicode, CharacterIndex);
						break;
					}
					
					++CharacterIndex;
				}
			}

			++FontIndex;
		}

		return false;
	}
}

#endif

#undef LOCTEXT_NAMESPACE