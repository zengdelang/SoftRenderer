#include "FontAtlasGenerator/GlyphGeometry.h"
#include "SignedDistance/ShapeDistanceFinder.h"
#include <cmath>

namespace FontAtlasGenerator
{
	FGlyphGeometry::FGlyphGeometry() : TextureIndex(0), FontStyle(EFontStyle::Normal), Index(), Codepoint(), GeometryScale(), Bounds(),
	                                   Advance(), Box()
	{
	}

	bool FGlyphGeometry::Load(const SDFGenerator::FFontHandle* Font, double InGeometryScale, SDFGenerator::FGlyphIndex InIndex)
	{
		if (Font && SDFGenerator::LoadGlyph(Shape, Font, InIndex, &Advance, &HoriBearingX, &HoriBearingY, &Width, &Height) && Shape.Validate())
		{
			this->Index = InIndex.GetIndex();
			this->GeometryScale = InGeometryScale;
			Codepoint = 0;
			Advance *= InGeometryScale;
			HoriBearingX *= InGeometryScale;
			HoriBearingY *= InGeometryScale;

			Width *= InGeometryScale;
			Height *= InGeometryScale;

			Shape.Normalize();
			Bounds = Shape.GetBounds();

			{
				// Determine if shape is winded incorrectly and reverse it in that case
				const SDFGenerator::FPoint2D OuterPoint(Bounds.Left - (Bounds.Right - Bounds.Left) - 1,
				                                        Bounds.Bottom - (Bounds.Top - Bounds.Bottom) - 1);
				if (SDFGenerator::FSimpleTrueShapeDistanceFinder::OneShotDistance(Shape, OuterPoint) > 0)
				{
					for (SDFGenerator::FContour& Contour : Shape.Contours)
						Contour.Reverse();
				}
			}
			return true;
		}
		return false;
	}

	bool FGlyphGeometry::Load(const SDFGenerator::FFontHandle* Font, double InGeometryScale, unicode_t InCodepoint)
	{
		SDFGenerator::FGlyphIndex GlyphIndex;
		if (SDFGenerator::GetGlyphIndex(GlyphIndex, Font, InCodepoint))
		{
			if (Load(Font, InGeometryScale, GlyphIndex))
			{
				this->Codepoint = InCodepoint;
				return true;
			}
		}
		return false;
	}
	
	void FGlyphGeometry::WrapBox(double Scale, double Range, double MiterLimit)
	{
		Scale *= GeometryScale;
		Range /= GeometryScale;
		Box.Range = Range;
		Box.Scale = Scale;
		if (Bounds.Left < Bounds.Right && Bounds.Bottom < Bounds.Top)
		{
			double L = Bounds.Left, B = Bounds.Bottom, R = Bounds.Right, T = Bounds.Top;
			L -= .5 * Range, B -= .5 * Range;
			R += .5 * Range, T += .5 * Range;

			if (MiterLimit > 0)
				Shape.BoundMiters(L, B, R, T, .5 * Range, MiterLimit, 1);
			
			const double W = Scale * (R - L);
			const double H = Scale * (T - B);
			Box.Rect.W = static_cast<int32>(ceil(W)) + 1;
			Box.Rect.H = static_cast<int32>(ceil(H)) + 1;
			Box.Translate.X = -L + .5 * (Box.Rect.W - W) / Scale;
			Box.Translate.Y = -B + .5 * (Box.Rect.H - H) / Scale;
		}
		else
		{
			Box.Rect.W = 0, Box.Rect.H = 0;
			Box.Translate = SDFGenerator::FVector2();
		}
	}

	void FGlyphGeometry::PlaceBox(int32 X, int32 Y)
	{
		Box.Rect.X = X, Box.Rect.Y = Y;
	}

	void FGlyphGeometry::SetBoxRect(const FRectangle& Rect)
	{
		Box.Rect = Rect;
	}

	int32 FGlyphGeometry::GetIndex() const
	{
		return Index;
	}

	SDFGenerator::FGlyphIndex FGlyphGeometry::GetGlyphIndex() const
	{
		return SDFGenerator::FGlyphIndex(Index);
	}

	unicode_t FGlyphGeometry::GetCodepoint() const
	{
		return Codepoint;
	}

	int32 FGlyphGeometry::GetIdentifier(EGlyphIdentifierType Type) const
	{
		switch (Type)
		{
		case EGlyphIdentifierType::GLYPH_INDEX:
			return Index;
		case EGlyphIdentifierType::UNICODE_CODEPOINT:
			return static_cast<int32>(Codepoint);
		}
		return 0;
	}

	const SDFGenerator::FShape& FGlyphGeometry::GetShape() const
	{
		return Shape;
	}

	double FGlyphGeometry::GetAdvance() const
	{
		return Advance;
	}
	
	double FGlyphGeometry::GetHoriBearingX() const
	{
		return HoriBearingX;
	}

	double FGlyphGeometry::GetHoriBearingY() const
	{
		return HoriBearingY;
	}

	double FGlyphGeometry::GetWidth() const
	{
		return Width;
	}

	double FGlyphGeometry::GetHeight() const
	{
		return Height;
	}

	FRectangle FGlyphGeometry::GetBoxRect() const
	{
		return Box.Rect;
	}

	void FGlyphGeometry::GetBoxRect(int32& X, int32& Y, int32& W, int32& H) const
	{
		X = Box.Rect.X, Y = Box.Rect.Y;
		W = Box.Rect.W, H = Box.Rect.H;
	}

	void FGlyphGeometry::GetBoxSize(int32& W, int32& H) const
	{
		W = Box.Rect.W, H = Box.Rect.H;
	}

	double FGlyphGeometry::GetBoxRange() const
	{
		return Box.Range;
	}

	SDFGenerator::FProjection FGlyphGeometry::GetBoxProjection() const
	{
		return SDFGenerator::FProjection(SDFGenerator::FVector2(Box.Scale), Box.Translate);
	}

	double FGlyphGeometry::GetBoxScale() const
	{
		return Box.Scale;
	}

	SDFGenerator::FVector2 FGlyphGeometry::GetBoxTranslate() const
	{
		return Box.Translate;
	}

	void FGlyphGeometry::GetQuadPlaneBounds(double& Left, double& Bottom, double& Right, double& Top) const
	{
		if (Box.Rect.W > 0 && Box.Rect.H > 0)
		{
			const double InvBoxScale = 1 / Box.Scale;
			Left = GeometryScale * (-Box.Translate.X + .5 * InvBoxScale);
			Bottom = GeometryScale * (-Box.Translate.Y + .5 * InvBoxScale);
			Right = GeometryScale * (-Box.Translate.X + (Box.Rect.W - .5) * InvBoxScale);
			Top = GeometryScale * (-Box.Translate.Y + (Box.Rect.H - .5) * InvBoxScale);
		}
		else
		{
			Left = 0, Bottom = 0, Right = 0, Top = 0;
		}
	}

	void FGlyphGeometry::GetQuadAtlasBounds(double& Left, double& Bottom, double& Right, double& Top) const
	{
		if (Box.Rect.W > 0 && Box.Rect.H > 0)
		{
			Left = Box.Rect.X + .5;
			Bottom = Box.Rect.Y + .5;
			Right = Box.Rect.X + Box.Rect.W - .5;
			Top = Box.Rect.Y + Box.Rect.H - .5;
		}
		else
		{
			Left = 0, Bottom = 0, Right = 0, Top = 0;
		}
	}

	bool FGlyphGeometry::IsWhitespace() const
	{
		return Shape.Contours.Num() == 0;
	}

	FGlyphGeometry::operator FGlyphBox() const
	{
		FGlyphBox GlyphBox;
		GlyphBox.Index = Index;
		GlyphBox.Advance = Advance;
		GetQuadPlaneBounds(GlyphBox.Bounds.Left, GlyphBox.Bounds.Bottom, GlyphBox.Bounds.Right, GlyphBox.Bounds.Top);
		GlyphBox.Rect.X = this->Box.Rect.X, GlyphBox.Rect.Y = this->Box.Rect.Y, GlyphBox.Rect.W = this->Box.Rect.W, GlyphBox.Rect.H = this->Box.Rect.H;
		return GlyphBox;
	}
}
