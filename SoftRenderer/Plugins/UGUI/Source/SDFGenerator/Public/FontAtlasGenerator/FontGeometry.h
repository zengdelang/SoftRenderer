#pragma once

#include "GlyphGeometry.h"
#include "FontAtlasGenerator/Charset.h"

#define SDF_ATLAS_DEFAULT_EM_SIZE 32.0

#if WITH_FREETYPE

namespace FontAtlasGenerator
{
	/**
	 * Represents the geometry of all glyphs of a given font or font variant
	 */
	class FFontGeometry
	{
	public:
		FFontGeometry();
		
		/**
		 * Loads all glyphs in a charset (Charset elements are Unicode codepoints), returns the number of successfully loaded glyphs
		 */
		int32 LoadCharset(const SDFGenerator::FFontHandle* Font, double FontScale, const FCharset& Charset, EFontStyle InFontStyle);

		/**
		 * Only loads font metrics and geometry scale from font
		 */
		bool LoadMetrics(const SDFGenerator::FFontHandle* Font, double FontScale);

		/**
		 * Adds a loaded glyph
		 */
		bool AddGlyph(const FGlyphGeometry& Glyph);
		bool AddGlyph(FGlyphGeometry&& Glyph);

		/**
		 * Loads kerning pairs for all glyphs that are currently present, returns the number of loaded kerning pairs
		 */
		int32 LoadKerning(const SDFGenerator::FFontHandle* Font);
		
		/**
		 * Returns the geometry scale to be used when loading glyphs
		 */
		double GetGeometryScale() const;
		
		/**
		 * Returns the processed font metrics
		 */
		const SDFGenerator::FFontMetrics& GetMetrics() const;
		
		/**
		 * Returns the type of identifier that was used to load glyphs
		 */
		EGlyphIdentifierType GetPreferredIdentifierType() const;

		TArray<FGlyphGeometry>& GetGlyphs() { return Glyphs; }
		
		/**
		 * Finds a glyph by glyph index or Unicode codepoint, returns null if not found
		 */
		const FGlyphGeometry* GetGlyph(SDFGenerator::FGlyphIndex GlyphIndex) const;
		const FGlyphGeometry* GetGlyph(unicode_t Codepoint) const;

	private:
		double GeometryScale;
		SDFGenerator::FFontMetrics Metrics;
		EGlyphIdentifierType PreferredIdentifierType;
		EFontStyle FontStyle;

		TArray<FGlyphGeometry> Glyphs;
		int32 RangeStart, RangeEnd;
		
		TMap<int32, int32> GlyphsByIndex;
		TMap<unicode_t, int32> GlyphsByCodepoint;
	};
}

#endif
