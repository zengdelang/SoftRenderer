#pragma once

#include "Shape/Shape.h"

#ifndef WITH_FREETYPE
	#define WITH_FREETYPE	0
#endif // WITH_FREETYPE

namespace SDFGenerator
{
#if WITH_FREETYPE
	typedef unsigned char byte;
	typedef uint32 unicode_t;

	class FFreetypeHandle;
	class FFontHandle;

	class FGlyphIndex
	{
	public:
		explicit FGlyphIndex(unsigned InIndex = 0);
		unsigned GetIndex() const;
		bool operator!() const;

	private:
		unsigned Index;
	};

	/**
	 * Global metrics of a typeface (in font units).
	 */
	struct FFontMetrics
	{
		/**
		 * The size of one EM.
		 */
		double EmSize;
		
		/**
		 * The vertical position of the ascender and descender relative to the baseline.
		 */
		double AscenderY, DescenderY;
		
		/**
		 * The vertical difference between consecutive baselines.
		 */
		double LineHeight;
		
		/**
		 * The vertical position and thickness of the underline.
		 */
		double UnderlineY, UnderlineThickness;
	};

	/**
	 * Initializes the FreeType library.
	 */
	FFreetypeHandle* InitializeFreetype();
	
	/**
	 * Deinitializes the FreeType library.
	 */
	void DeinitializeFreetype(const FFreetypeHandle* Library);

	int32 GetFaceNum(const FFreetypeHandle* Library, const char* Filename);
	
	/**
	 * Loads a font file and returns its handle.
	 */
	FFontHandle* LoadFont(const FFreetypeHandle* Library, const char* Filename, int32 FaceIndex = 0);

	FFontHandle* GetFaceName(const FFreetypeHandle* Library, const char* Filename, FString& FamilyName, FString& StyleName, int32 FaceIndex = 0);
	
	/**
	 * Loads a font from binary data and returns its handle.
	 */
	FFontHandle* LoadFontData(const FFreetypeHandle* Library, const byte* Data, int32 Length, int32 FaceIndex = 0);
	
	/**
	 * Unloads a font file.
	 */
	void DestroyFont(const FFontHandle* Font);
	
	/**
	 * Outputs the metrics of a font file.
	 */
	bool GetFontMetrics(FFontMetrics& Metrics, const FFontHandle* Font);
	
	/**
	 * Outputs the width of the space and tab characters.
	 * */
	bool GetFontWhitespaceWidth(double& SpaceAdvance, double& TabAdvance, const FFontHandle* Font);
	
	/**
	 * Outputs the glyph index corresponding to the specified Unicode character.
	 */
	bool GetGlyphIndex(FGlyphIndex& GlyphIndex, const FFontHandle* Font, unicode_t Unicode);
	
	/**
	 * Loads the geometry of a glyph from a font file.
	 */
	bool LoadGlyph(FShape& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex, double* Advance = nullptr, double* HoriBearingX = nullptr, double* HoriBearingY = nullptr, double* Width = nullptr, double* Height = nullptr);
	bool LoadGlyph(FShape& Output, const FFontHandle* Font, unicode_t Unicode, double* Advance = nullptr, double* HoriBearingX = nullptr, double* HoriBearingY = nullptr, double* Width = nullptr, double* Height = nullptr);
	
	/**
	 * Outputs the kerning distance adjustment between two specific glyphs.
	 */
	bool GetKerning(double& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex1, FGlyphIndex GlyphIndex2);
	bool GetKerning(double& Output, const FFontHandle* Font, unicode_t Unicode1, unicode_t Unicode2);

#endif // USE_FREETYPE
}
