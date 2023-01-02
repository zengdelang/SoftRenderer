#include "FontAtlasGenerator/FontGeometry.h"

#if WITH_FREETYPE

namespace FontAtlasGenerator
{
	FFontGeometry::FFontGeometry()
		: GeometryScale(1),
		  Metrics(),
		  PreferredIdentifierType(EGlyphIdentifierType::UNICODE_CODEPOINT),
	      FontStyle(EFontStyle::Normal),
		  RangeStart(Glyphs.Num()), RangeEnd(Glyphs.Num())
	{
	}

	int32 FFontGeometry::LoadCharset(const SDFGenerator::FFontHandle* Font, double FontScale, const FCharset& Charset, EFontStyle InFontStyle)
	{
		if (!(Glyphs.Num() == RangeEnd && LoadMetrics(Font, FontScale)))
			return -1;
		
		Glyphs.Reserve(Glyphs.Num() + Charset.Num());

		int32 Loaded = 0;

		const auto& CharsetString = Charset.GetCharsetString();
		for (int32 TextIndex = 0, Len = CharsetString.Len(); TextIndex < Len; ++TextIndex)
		{
			const unicode_t CodePoint = CharsetString[TextIndex];
			FGlyphGeometry Glyph;
			if (Glyph.Load(Font, GeometryScale, CodePoint))
			{
				AddGlyph(Glyph);
				++Loaded;
			}
		}
		
		PreferredIdentifierType = EGlyphIdentifierType::UNICODE_CODEPOINT;
		FontStyle = InFontStyle;
		return Loaded;
	}

	bool FFontGeometry::LoadMetrics(const SDFGenerator::FFontHandle* Font, double FontScale)
	{
		if (!SDFGenerator::GetFontMetrics(Metrics, Font))
			return false;

		if (Metrics.EmSize <= 0)
			Metrics.EmSize = SDF_ATLAS_DEFAULT_EM_SIZE;

		GeometryScale = FontScale / Metrics.EmSize;
		Metrics.EmSize *= GeometryScale;
		Metrics.AscenderY *= GeometryScale;
		Metrics.DescenderY *= GeometryScale;
		Metrics.LineHeight *= GeometryScale;
		Metrics.UnderlineY *= GeometryScale;
		Metrics.UnderlineThickness *= GeometryScale;
		return true;
	}

	bool FFontGeometry::AddGlyph(const FGlyphGeometry& Glyph)
	{
		if (Glyphs.Num() != RangeEnd)
			return false;

		GlyphsByIndex.Emplace(Glyph.GetIndex(),RangeEnd);
		if (Glyph.GetCodepoint())
		{
			GlyphsByCodepoint.Emplace(Glyph.GetCodepoint(), RangeEnd);
		}

		Glyphs.Add(Glyph);
		
		++RangeEnd;
		return true;
	}

	bool FFontGeometry::AddGlyph(FGlyphGeometry&& Glyph)
	{
		if (Glyphs.Num() != RangeEnd)
			return false;
		
		GlyphsByIndex.Emplace(Glyph.GetIndex(),RangeEnd);
		if (Glyph.GetCodepoint())
		{
			GlyphsByCodepoint.Emplace(Glyph.GetCodepoint(), RangeEnd);
		}
		
		Glyphs.Emplace(Glyph);
		
		++RangeEnd;
		return true;
	}
	
	double FFontGeometry::GetGeometryScale() const
	{
		return GeometryScale;
	}

	const SDFGenerator::FFontMetrics& FFontGeometry::GetMetrics() const
	{
		return Metrics;
	}

	EGlyphIdentifierType FFontGeometry::GetPreferredIdentifierType() const
	{
		return PreferredIdentifierType;
	}
	
	const FGlyphGeometry* FFontGeometry::GetGlyph(SDFGenerator::FGlyphIndex GlyphIndex) const
	{
		const auto IndexPtr = GlyphsByIndex.Find(GlyphIndex.GetIndex());
		if (IndexPtr)
			return &(Glyphs)[*IndexPtr];
		return nullptr;
	}

	const FGlyphGeometry* FFontGeometry::GetGlyph(unicode_t Codepoint) const
	{
		const auto IndexPtr = GlyphsByCodepoint.Find(Codepoint);
		if (IndexPtr)
			return &(Glyphs)[*IndexPtr];
		return nullptr;
	}
}

#endif
