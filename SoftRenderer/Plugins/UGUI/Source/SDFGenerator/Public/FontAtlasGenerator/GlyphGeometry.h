#pragma once

#include "FontAtlasGenerator/Charset.h"
#include "Shape/Projection.h"
#include "Shape/Shape.h"
#include "ShapeGenerator/FontShapeGenerator.h"

#if WITH_FREETYPE

namespace FontAtlasGenerator
{
	struct FRectangle
	{
		int32 X, Y, W, H;
	};

	/**
	 * The glyph box - its bounds in plane and atlas
	 */
	struct FGlyphBox
	{
		int32 Index;
		double Advance;

		struct
		{
			double Left, Bottom, Right, Top;
		} Bounds;

		FRectangle Rect;
	};

	/**
	 * Represents the shape geometry of a single glyph as well as its configuration
	 */
	class FGlyphGeometry
	{
	public:
		FGlyphGeometry();

		/**
		 * Loads glyph geometry from font
		 */
		bool Load(const SDFGenerator::FFontHandle* Font, double InGeometryScale, SDFGenerator::FGlyphIndex InIndex);
		
		bool Load(const SDFGenerator::FFontHandle* Font, double InGeometryScale, unicode_t InCodepoint);
		
		/**
		 * Computes the dimensions of the glyph's box as well as the transformation for the generator function
		 */
		void WrapBox(double Scale, double Range, double MiterLimit);
		
		/**
		 * Sets the glyph's box's position in the atlas
		 */
		void PlaceBox(int32 X, int32 Y);
		
		/**
		 * Sets the glyph's box's rectangle in the atlas
		 */
		void SetBoxRect(const FRectangle& Rect);

		/**
		 * Returns the glyph's index within the font
		 */
		int32 GetIndex() const;

		/**
		 * Returns the glyph's index as a SDFGenerator::GlyphIndex
		 */
		SDFGenerator::FGlyphIndex GetGlyphIndex() const;

		/**
		 * Returns the Unicode codepoint represented by the glyph or 0 if unknown
		 */
		unicode_t GetCodepoint() const;

		/**
		 * Returns the glyph's identifier specified by the supplied identifier type
		 */
		int32 GetIdentifier(EGlyphIdentifierType Type) const;

		/**
		 * Returns the glyph's shape
		 */
		const SDFGenerator::FShape& GetShape() const;

		/**
		 * Returns the glyph's advance
		 */
		double GetAdvance() const;

		double GetHoriBearingX() const;

		double GetHoriBearingY() const;

		double GetWidth() const;

		double GetHeight() const;

		/**
		 * Returns the glyph's box in the atlas
		 */
		FRectangle GetBoxRect() const;

		/**
		 * Outputs the position and dimensions of the glyph's box in the atlas
		 */
		void GetBoxRect(int32& X, int32& Y, int32& W, int32& H) const;

		/**
		 * Outputs the dimensions of the glyph's box in the atlas
		 */
		void GetBoxSize(int32& W, int32& H) const;

		/**
		 * Returns the range needed to generate the glyph's SDF
		 */
		double GetBoxRange() const;

		/**
		 * Returns the projection needed to generate the glyph's bitmap
		 */
		SDFGenerator::FProjection GetBoxProjection() const;

		/**
		 * Returns the scale needed to generate the glyph's bitmap
		 */
		double GetBoxScale() const;

		/**
		 * Returns the translation vector needed to generate the glyph's bitmap
		 */
		SDFGenerator::FVector2 GetBoxTranslate() const;

		/**
		 * Outputs the bounding box of the glyph as it should be placed on the baseline
		 */
		void GetQuadPlaneBounds(double& Left, double& Bottom, double& Right, double& Top) const;

		/**
		 * Outputs the bounding box of the glyph in the atlas
		 */
		void GetQuadAtlasBounds(double& Left, double& Bottom, double& Right, double& Top) const;

		/**
		 * Returns true if the glyph is a whitespace and has no geometry
		 */
		bool IsWhitespace() const;

		/**
		 * Simplifies to GlyphBox
		 */
		operator FGlyphBox() const;

	public:
		int32 TextureIndex;
		EFontStyle FontStyle;
		TArray<uint8> GlyphBitmapData;
		
	private:
		int32 Index;
		
		unicode_t Codepoint;
		double GeometryScale;
		
		SDFGenerator::FShape Shape;
		SDFGenerator::FShape::FBounds Bounds;

		double Advance, HoriBearingX, HoriBearingY, Width, Height;

		struct
		{
			FRectangle Rect;
			double Range;
			double Scale;
			SDFGenerator::FVector2 Translate;
		} Box;
	};
}

#endif
