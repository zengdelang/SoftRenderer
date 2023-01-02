#include "FontAtlasGenerator/FontAtlasPacker.h"
#include "GeneratorConfig.h"
#include "SDFGenerator.h"
#include "Image/BitmapFileHelper.h"
#include "Rasterization/Rasterization.h"
#include "Utility/PixelUtility.h"

#define SDF_ATLAS_GLYPH_FILL_RULE SDFGenerator::EFillRule::FILL_NONZERO

namespace FontAtlasGenerator
{
	FFontAtlasPacker::~FFontAtlasPacker()
	{
		EmptyAtlasData();
	}

	void FFontAtlasPacker::EmptyAtlasData()
	{
		// Remove all nodes
		TArray<FAtlasedTextureSlot*, TMemStackAllocator<>> DeleteSlots;

		for (FAtlasedTextureSlot::TIterator SlotIt(AtlasUsedSlots); SlotIt; SlotIt.Next())
		{
			FAtlasedTextureSlot& CurSlot = *SlotIt;
			DeleteSlots.Add(&CurSlot);
		}

		for (FAtlasedTextureSlot::TIterator SlotIt(AtlasEmptySlots); SlotIt; SlotIt.Next())
		{
			FAtlasedTextureSlot& CurSlot = *SlotIt;
			DeleteSlots.Add(&CurSlot);
		}

		AtlasUsedSlots = nullptr;
		AtlasEmptySlots = nullptr;

		for (const FAtlasedTextureSlot* CurSlot : DeleteSlots)
		{
			delete CurSlot;
		}

		DeleteSlots.Empty();
	}

	const FAtlasedTextureSlot* FFontAtlasPacker::FindSlotForTexture(uint32 InWidth, uint32 InHeight)
	{
		FAtlasedTextureSlot* ReturnVal = nullptr;

		// Account for padding on both sides
		const uint32 Padding = GetPaddingAmount();
		//const uint32 TotalPadding = Padding * 2;
		constexpr uint32 TotalPadding = 1;
		const uint32 PaddedWidth = InWidth + TotalPadding;
		const uint32 PaddedHeight = InHeight + TotalPadding;

		// Previously, slots were stored as a binary tree - this has been replaced with a linked-list of slots on the edge of the tree
		// (slots on the edge of the tree represent empty slots); this iterates empty slots in same order as a binary depth-first-search,
		// except much faster.
		for (FAtlasedTextureSlot::TIterator SlotIt(AtlasEmptySlots); SlotIt; ++SlotIt)
		{
			FAtlasedTextureSlot& CurSlot = *SlotIt;

			if (PaddedWidth <= CurSlot.Width && PaddedHeight <= CurSlot.Height)
			{
				ReturnVal = &CurSlot;
				break;
			}
		}
		
		if (ReturnVal != nullptr)
		{
			// The width and height of the new child node
			const uint32 RemainingWidth = FMath::Max<int32>(0, ReturnVal->Width - PaddedWidth);
			const uint32 RemainingHeight = FMath::Max<int32>(0, ReturnVal->Height - PaddedHeight);

			// New slots must have a minimum width/height, to avoid excessive slots i.e. excessive memory usage and iteration.
			// No glyphs seem to use slots this small, and cutting these slots out improves performance/memory-usage a fair bit
			constexpr uint32 MinSlotDim = 2;

			// Split the remaining area around this slot into two children.
			if (RemainingHeight >= MinSlotDim || RemainingWidth >= MinSlotDim)
			{
				FAtlasedTextureSlot* LeftSlot;
				FAtlasedTextureSlot* RightSlot;

				if (RemainingHeight <= RemainingWidth)
				{
					// Split vertically
					// - - - - - - - - -
					// |       |       |
					// |  Slot |       |
					// |       |       |
					// | - - - | Right |
					// |       |       |
					// |  Left |       |
					// |       |       |
					// - - - - - - - - -
					LeftSlot = new FAtlasedTextureSlot(ReturnVal->X, ReturnVal->Y + PaddedHeight, PaddedWidth,
					                                   RemainingHeight, Padding);
					RightSlot = new FAtlasedTextureSlot(ReturnVal->X + PaddedWidth, ReturnVal->Y, RemainingWidth,
					                                    ReturnVal->Height, Padding);
				}
				else
				{
					// Split horizontally
					// - - - - - - - - -
					// |       |       |
					// |  Slot | Left  |
					// |       |       |
					// | - - - - - - - |
					// |               |
					// |     Right     |
					// |               |
					// - - - - - - - - -
					LeftSlot = new FAtlasedTextureSlot(ReturnVal->X + PaddedWidth, ReturnVal->Y, RemainingWidth,
					                                   PaddedHeight, Padding);
					RightSlot = new FAtlasedTextureSlot(ReturnVal->X, ReturnVal->Y + PaddedHeight, ReturnVal->Width,
					                                    RemainingHeight, Padding);
				}

				// Replace the old slot within AtlasEmptySlots, with the new Left and Right slot, then add the old slot to AtlasUsedSlots
				LeftSlot->LinkReplace(ReturnVal);
				RightSlot->LinkAfter(LeftSlot);

				ReturnVal->LinkHead(AtlasUsedSlots);
			}
			else
			{
				// Remove the old slot from AtlasEmptySlots, into AtlasUsedSlots
				ReturnVal->Unlink();
				ReturnVal->LinkHead(AtlasUsedSlots);
			}

			// Shrink the slot to the remaining area.
			ReturnVal->Width = PaddedWidth;
			ReturnVal->Height = PaddedHeight;
		}

		return ReturnVal;
	}

	void FFontAtlasPacker::InitAtlasData()
	{
		check(AtlasEmptySlots == nullptr);

		FAtlasedTextureSlot* RootSlot = new FAtlasedTextureSlot(0, 0, AtlasWidth, AtlasHeight, GetPaddingAmount());

		RootSlot->LinkHead(AtlasEmptySlots);
	}

	void FFontAtlasPacker::Pack(const TArray<FGlyphGeometry*>& GlyphGeometries, bool bOverlapSupport, bool bScanlinePass, FVector2D MaxAtlasSize,
		int32& CurIndex, int32 Count, ISDFGeneratorLogListener* LogListener)
	{
		int32 DesiredWidth = 4;
		int32 DesiredHeight = 4;

		AtlasWidth = DesiredWidth;
		AtlasHeight = DesiredHeight;
		EmptyAtlasData();
		InitAtlasData();

		while (DesiredWidth < MaxAtlasSize.X || DesiredHeight < MaxAtlasSize.Y)
		{
			bool bEnough = true;
			for (auto& Glyph : GlyphGeometries)
			{
				int32 GlyphWidth = 0;
				int32 GlyphHeight = 0;
				Glyph->GetBoxSize(GlyphWidth, GlyphHeight);
				if (GlyphWidth > 0 && GlyphHeight > 0)
				{
					if (FindSlotForTexture(GlyphWidth, GlyphHeight) == nullptr)
					{
						bEnough = false;
						break;
					}
				}	
			}

			if (bEnough)
				break;

			if (DesiredWidth <= DesiredHeight)
			{
				DesiredWidth = FMath::Min(static_cast<int32>(MaxAtlasSize.X), 2 * DesiredWidth);
			}
			else
			{
				DesiredHeight = FMath::Min(static_cast<int32>(MaxAtlasSize.Y), 2 * DesiredHeight);
			}

			AtlasWidth = DesiredWidth;
			AtlasHeight = DesiredHeight;
			EmptyAtlasData();
			InitAtlasData();
		}

		AtlasWidth = DesiredWidth;
		AtlasHeight = DesiredHeight;
		EmptyAtlasData();
		
		SDFGenerator::FGeneratorConfig GeneratorConfig;
		GeneratorConfig.bOverlapSupport = bOverlapSupport;

		TArray<float> BitmapData;
		for (auto& Glyph : GlyphGeometries)
		{
			if (LogListener)
			{
				LogListener->UpdateProgress(CurIndex, Count);
			}
			
			int32 Left, Bottom, Width, Height;
			Glyph->GetBoxRect(Left, Bottom, Width, Height);

			BitmapData.Empty();
			BitmapData.SetNumZeroed(Width * Height);
			SDFGenerator::FBitmapRef<float, 1> GlyphBitmap(BitmapData.GetData(), Width, Height);
			SDFGenerator::GenerateSDF(GlyphBitmap, Glyph->GetShape(), Glyph->GetBoxProjection(), Glyph->GetBoxRange(), GeneratorConfig);
			if (bScanlinePass)
			{
				SDFGenerator::DistanceSignCorrection(GlyphBitmap, Glyph->GetShape(), Glyph->GetBoxProjection(), SDF_ATLAS_GLYPH_FILL_RULE);
			}
			
			Glyph->GlyphBitmapData.SetNumZeroed(Width * Height);

			int32 Index = 0;
			for (int32 Y = Height - 1; Y >= 0; --Y)
			{
				for (int32 X = 0; X < Width; ++X)
				{
					Glyph->GlyphBitmapData[Index] = static_cast<const uint8>(SDFGenerator::FPixelUtility::PixelFloatToByte(GlyphBitmap.Pixels[X + Y * Width]));
					++Index;
				}
			}

			++CurIndex;
		}
		
		FFontTextureAtlas FontTextureAtlas(AtlasWidth, AtlasHeight, 1);
		int32 TextureIndex = 0;
		for (auto& Glyph : GlyphGeometries)
		{
			int32 GlyphWidth = 0;
			int32 GlyphHeight = 0;
			Glyph->GetBoxSize(GlyphWidth, GlyphHeight);
			if (GlyphWidth > 0 && GlyphHeight > 0)
			{
				auto TextureSlot = FontTextureAtlas.AddTexture(GlyphWidth, GlyphHeight, Glyph->GlyphBitmapData);
				if (TextureSlot == nullptr)
				{
					AtlasDataList.Emplace(FontTextureAtlas.AtlasData);
					FontTextureAtlas.EmptyAtlasData();
					FontTextureAtlas.InitAtlasData();
					TextureSlot = FontTextureAtlas.AddTexture(GlyphWidth, GlyphHeight, Glyph->GlyphBitmapData);
					
					Glyph->PlaceBox(TextureSlot->X, TextureSlot->Y);
					
					++TextureIndex;
				}
				else
				{
					Glyph->PlaceBox(TextureSlot->X,  TextureSlot->Y);
				}
				
				Glyph->TextureIndex = TextureIndex;
			}
		}
		
		AtlasDataList.Emplace(FontTextureAtlas.AtlasData);
	}
}
