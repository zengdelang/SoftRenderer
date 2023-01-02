#include "ShapeGenerator/FontShapeGenerator.h"

#ifndef WITH_FREETYPE
	#define WITH_FREETYPE	0
#endif // WITH_FREETYPE

#if WITH_FREETYPE
#include "ft2build.h"
// Freetype style include
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif // USE_FREETYPE

namespace SDFGenerator
{
#if WITH_FREETYPE
	
	#define REQUIRE(cond) { if (!(cond)) return false; }
	#define F26DOT6_TO_DOUBLE(x) (1/64.*double(x))

	class FFreetypeHandle
	{
		friend FFreetypeHandle* InitializeFreetype();
		friend void DeinitializeFreetype(const FFreetypeHandle* Library);
		friend int32 GetFaceNum(const FFreetypeHandle* Library, const char* Filename);
		friend FFontHandle* GetFaceName(const FFreetypeHandle* Library, const char* Filename, FString& FamilyName, FString& StyleName, int32 FaceIndex);
		friend FFontHandle* LoadFont(const FFreetypeHandle* Library, const char* Filename, int32 FaceIndex);
		friend FFontHandle* LoadFontData(const FFreetypeHandle* Library, const byte* Data, int32 Length, int32 FaceIndex);

		FT_Library Library;
	};

	class FFontHandle
	{
		friend FFontHandle* AdoptFreetypeFont(FT_Face FTFace);
		friend int32 GetFaceNum(const FFreetypeHandle* Library, const char* Filename);
		friend FFontHandle* GetFaceName(const FFreetypeHandle* Library, const char* Filename, FString& FamilyName, FString& StyleName, int32 FaceIndex);
		friend FFontHandle* LoadFont(const FFreetypeHandle* Library, const char* Filename, int32 FaceIndex);
		friend FFontHandle* LoadFontData(const FFreetypeHandle* Library, const byte* Data, int32 Length, int32 FaceIndex);
		friend void DestroyFont(const FFontHandle* Font);
		friend bool GetFontMetrics(FFontMetrics& Metrics, const FFontHandle* Font);
		friend bool GetFontWhitespaceWidth(double& SpaceAdvance, double& TabAdvance, const FFontHandle* Font);
		friend bool GetGlyphIndex(FGlyphIndex& GlyphIndex, const FFontHandle* Font, unicode_t Unicode);
		friend bool LoadGlyph(FShape& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex, double* Advance, double* HoriBearingX, double* HoriBearingY, double* Width, double* Height);
		friend bool LoadGlyph(FShape& Output, const FFontHandle* Font, unicode_t Unicode, double* Advance, double* HoriBearingX, double* HoriBearingY, double* Width, double* Height);
		friend bool GetKerning(double& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex1, FGlyphIndex GlyphIndex2);
		friend bool GetKerning(double& Output, const FFontHandle* Font, unicode_t Unicode1, unicode_t Unicode2);

		FT_Face Face;
		bool bOwnership;
	};

	struct FtContext
	{
		FPoint2D Position;
		FShape* Shape;
		FContour* Contour;
	};

	static FPoint2D FTPoint2(const FT_Vector& Vector)
	{
		return FPoint2D(F26DOT6_TO_DOUBLE(Vector.x), F26DOT6_TO_DOUBLE(Vector.y));
	}

	static int32 FTMoveTo(const FT_Vector* To, void* User)
	{
		FtContext* Context = reinterpret_cast<FtContext*>(User);
		if (!(Context->Contour && Context->Contour->Edges.Num() == 0))
			Context->Contour = &Context->Shape->AddContour();
		Context->Position = FTPoint2(*To);
		return 0;
	}

	static int32 FTLineTo(const FT_Vector* To, void* User)
	{
		FtContext* Context = reinterpret_cast<FtContext*>(User);
		const FPoint2D Endpoint = FTPoint2(*To);
		if (Endpoint != Context->Position)
		{
			Context->Contour->AddEdge(new FLinearSegment(Context->Position, Endpoint));
			Context->Position = Endpoint;
		}
		return 0;
	}

	static int32 FTConicTo(const FT_Vector* Control, const FT_Vector* To, void* User)
	{
		FtContext* Context = reinterpret_cast<FtContext*>(User);
		Context->Contour->AddEdge(new FQuadraticSegment(Context->Position, FTPoint2(*Control), FTPoint2(*To)));
		Context->Position = FTPoint2(*To);
		return 0;
	}

	static int32 FTCubicTo(const FT_Vector* Control1, const FT_Vector* Control2, const FT_Vector* To, void* User)
	{
		FtContext* Context = reinterpret_cast<FtContext*>(User);
		Context->Contour->AddEdge(new FCubicSegment(Context->Position, FTPoint2(*Control1), FTPoint2(*Control2),
		                                            FTPoint2(*To)));
		Context->Position = FTPoint2(*To);
		return 0;
	}

	FGlyphIndex::FGlyphIndex(unsigned InIndex) : Index(InIndex)
	{
	}

	unsigned FGlyphIndex::GetIndex() const
	{
		return Index;
	}

	bool FGlyphIndex::operator!() const
	{
		return Index == 0;
	}

	FFreetypeHandle* InitializeFreetype()
	{
		FFreetypeHandle* Handle = new FFreetypeHandle;
		const FT_Error Error = FT_Init_FreeType(&Handle->Library);
		if (Error)
		{
			delete Handle;
			return nullptr;
		}
		return Handle;
	}

	void DeinitializeFreetype(const FFreetypeHandle* Library)
	{
		FT_Done_FreeType(Library->Library);
		delete Library;
	}

	FFontHandle* AdoptFreetypeFont(FT_Face FTFace)
	{
		FFontHandle* Handle = new FFontHandle;
		Handle->Face = FTFace;
		Handle->bOwnership = false;
		return Handle;
	}

	int32 GetFaceNum(const FFreetypeHandle* Library, const char* Filename)
	{
		if (!Library)
			return 0;
		
		FFontHandle* Handle = new FFontHandle;
		const FT_Error Error = FT_New_Face(Library->Library, Filename, 0, &Handle->Face);
		if (Error)
		{
			delete Handle;
			return 0;
		}

		int32 FaceNum = Handle->Face->num_faces - 1;
		
		Handle->bOwnership = true;
		if (Handle->bOwnership)
			FT_Done_Face(Handle->Face);
		delete Handle;
		
		return FaceNum;
	}

	FFontHandle* LoadFont(const FFreetypeHandle* Library, const char* Filename, int32 FaceIndex)
	{
		if (!Library)
			return nullptr;
		
		FFontHandle* Handle = new FFontHandle;
		const FT_Error Error = FT_New_Face(Library->Library, Filename, FaceIndex, &Handle->Face);
		if (Error)
		{
			delete Handle;
			return nullptr;
		}
		Handle->bOwnership = true;
		return Handle;
	}

	FFontHandle* GetFaceName(const FFreetypeHandle* Library, const char* Filename, FString& FamilyName, FString& StyleName, int32 FaceIndex)
	{
		if (!Library)
			return nullptr;
		
		FFontHandle* Handle = new FFontHandle;
		const FT_Error Error = FT_New_Face(Library->Library, Filename, FaceIndex, &Handle->Face);
		if (Error)
		{
			delete Handle;
			return nullptr;
		}

		FamilyName = TCHAR_TO_UTF8(ANSI_TO_TCHAR(Handle->Face->family_name));
		StyleName = TCHAR_TO_UTF8(ANSI_TO_TCHAR(Handle->Face->style_name));
		
		Handle->bOwnership = true;
		return Handle;
	}

	FFontHandle* LoadFontData(const FFreetypeHandle* Library, const byte* Data, int32 Length, int32 FaceIndex)
	{
		if (!Library)
			return nullptr;
		
		FFontHandle* Handle = new FFontHandle;
		const FT_Error Error = FT_New_Memory_Face(Library->Library, Data, Length, FaceIndex, &Handle->Face);
		if (Error)
		{
			delete Handle;
			return nullptr;
		}
		Handle->bOwnership = true;
		return Handle;
	}

	void DestroyFont(const FFontHandle* Font)
	{
		if (Font->bOwnership)
			FT_Done_Face(Font->Face);
		delete Font;
	}

	bool GetFontMetrics(FFontMetrics& Metrics, const FFontHandle* Font)
	{
		Metrics.EmSize = F26DOT6_TO_DOUBLE(Font->Face->units_per_EM);
		Metrics.AscenderY = F26DOT6_TO_DOUBLE(Font->Face->ascender);
		Metrics.DescenderY = F26DOT6_TO_DOUBLE(Font->Face->descender);
		Metrics.LineHeight = F26DOT6_TO_DOUBLE(Font->Face->height);
		Metrics.UnderlineY = F26DOT6_TO_DOUBLE(Font->Face->underline_position);
		Metrics.UnderlineThickness = F26DOT6_TO_DOUBLE(Font->Face->underline_thickness);
		return true;
	}

	bool GetFontWhitespaceWidth(double& SpaceAdvance, double& TabAdvance, const FFontHandle* Font)
	{
		FT_Error error = FT_Load_Char(Font->Face, ' ', FT_LOAD_NO_SCALE);
		if (error)
			return false;
		
		SpaceAdvance = F26DOT6_TO_DOUBLE(Font->Face->glyph->advance.x);
		
		error = FT_Load_Char(Font->Face, '\t', FT_LOAD_NO_SCALE);
		if (error)
			return false;
		
		TabAdvance = F26DOT6_TO_DOUBLE(Font->Face->glyph->advance.x);
		return true;
	}

	bool GetGlyphIndex(FGlyphIndex& GlyphIndex, const FFontHandle* Font, unicode_t Unicode)
	{
		GlyphIndex = FGlyphIndex(FT_Get_Char_Index(Font->Face, Unicode));
		return GlyphIndex.GetIndex() != 0 || Unicode == 127;
	}

	bool LoadGlyph(FShape& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex, double* Advance, double* HoriBearingX, double* HoriBearingY, double* Width, double* Height)
	{
		if (!Font)
			return false;
		
		FT_Error Error = FT_Load_Glyph(Font->Face, GlyphIndex.GetIndex(), FT_LOAD_NO_SCALE);
		if (Error)
			return false;
		
		Output.Contours.Empty();
		Output.bInverseYAxis = false;
		
		if (Advance)
			*Advance = F26DOT6_TO_DOUBLE(Font->Face->glyph->advance.x);

		if (HoriBearingX)
			*HoriBearingX = F26DOT6_TO_DOUBLE(Font->Face->glyph->metrics.horiBearingX);
		
		if (HoriBearingY)
			*HoriBearingY = F26DOT6_TO_DOUBLE(Font->Face->glyph->metrics.horiBearingY);

		if (Width)
			*Width = F26DOT6_TO_DOUBLE(Font->Face->glyph->metrics.width);

		if (Height)
			*Height = F26DOT6_TO_DOUBLE(Font->Face->glyph->metrics.height);
		
		FtContext Context = {};
		Context.Shape = &Output;
		FT_Outline_Funcs ftFunctions;
		ftFunctions.move_to = &FTMoveTo;
		ftFunctions.line_to = &FTLineTo;
		ftFunctions.conic_to = &FTConicTo;
		ftFunctions.cubic_to = &FTCubicTo;
		ftFunctions.shift = 0;
		ftFunctions.delta = 0;
		
		Error = FT_Outline_Decompose(&Font->Face->glyph->outline, &ftFunctions, &Context);
		if (Error)
			return false;
		
		if (Output.Contours.Num() > 0 && Output.Contours.Last().Edges.Num() == 0)
			Output.Contours.Pop();
		return true;
	}

	bool LoadGlyph(FShape& Output, const FFontHandle* Font, unicode_t Unicode, double* Advance, double* HoriBearingX, double* HoriBearingY, double* Width, double* Height)
	{
		return LoadGlyph(Output, Font, FGlyphIndex(FT_Get_Char_Index(Font->Face, Unicode)), Advance, HoriBearingX, HoriBearingY);
	}

	bool GetKerning(double& Output, const FFontHandle* Font, FGlyphIndex GlyphIndex1, FGlyphIndex GlyphIndex2)
	{
		FT_Vector Kerning;
		if (FT_Get_Kerning(Font->Face, GlyphIndex1.GetIndex(), GlyphIndex2.GetIndex(), FT_KERNING_UNSCALED, &Kerning))
		{
			Output = 0;
			return false;
		}
		Output = F26DOT6_TO_DOUBLE(Kerning.x);
		return true;
	}

	bool GetKerning(double& Output, const FFontHandle* Font, unicode_t Unicode1, unicode_t Unicode2)
	{
		return GetKerning(Output, Font, FGlyphIndex(FT_Get_Char_Index(Font->Face, Unicode1)),FGlyphIndex(FT_Get_Char_Index(Font->Face, Unicode2)));
	}

#endif // USE_FREETYPE
}
