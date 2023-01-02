#include "FontAtlasGenerator/FontTextureAtlas.h"

namespace FontAtlasGenerator
{
	FFontTextureAtlas::~FFontTextureAtlas()
	{
		EmptyAtlasData();
	}

	void FFontTextureAtlas::EmptyAtlasData()
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
		
		// Clear all raw data
		AtlasData.Empty();
	}
	
	const FAtlasedTextureSlot* FFontTextureAtlas::AddTexture(uint32 TextureWidth, uint32 TextureHeight, const TArray<uint8>& Data)
	{
		// Find a spot for the character in the texture
		const FAtlasedTextureSlot* NewSlot = FindSlotForTexture(TextureWidth, TextureHeight);

		// handle cases like space, where the size of the glyph is zero. The copy data code doesn't handle zero sized source data with a padding
		// so make sure to skip this call.
		if (NewSlot && TextureWidth > 0 && TextureHeight > 0)
		{
			CopyDataIntoSlot(NewSlot, Data);
		}

		return NewSlot;
	}
	
	void FFontTextureAtlas::InitAtlasData()
	{
		check(AtlasEmptySlots == NULL && AtlasData.Num() == 0);

		FAtlasedTextureSlot* RootSlot = new FAtlasedTextureSlot(0, 0, AtlasWidth, AtlasHeight, GetPaddingAmount());

		RootSlot->LinkHead(AtlasEmptySlots);

		AtlasData.Reserve(AtlasWidth * AtlasHeight * BytesPerPixel);
		AtlasData.AddZeroed(AtlasWidth * AtlasHeight * BytesPerPixel);
	}
	
	void FFontTextureAtlas::CopyRow(const FCopyRowData& CopyRowData) const
	{
		const uint8* Data = CopyRowData.SrcData;
		uint8* Start = CopyRowData.DestData;
		const uint32 SourceWidth = CopyRowData.SrcTextureWidth;
		const uint32 DestWidth = CopyRowData.DestTextureWidth;
		const uint32 SrcRow = CopyRowData.SrcRow;
		const uint32 DestRow = CopyRowData.DestRow;
		// this can only be one or zero
		const uint32 Padding = GetPaddingAmount();

		const uint8* SourceDataAddress = &Data[(SrcRow * SourceWidth) * BytesPerPixel];
		uint8* DestDataAddress = &Start[(DestRow * DestWidth + Padding) * BytesPerPixel];
		FMemory::Memcpy(DestDataAddress, SourceDataAddress, SourceWidth * BytesPerPixel);

		if (Padding > 0)
		{
			uint8* DestPaddingPixelLeft = &Start[(DestRow * DestWidth) * BytesPerPixel];
			uint8* DestPaddingPixelRight = DestPaddingPixelLeft + ((CopyRowData.RowWidth - 1) * BytesPerPixel);
			FMemory::Memzero(DestPaddingPixelLeft, BytesPerPixel);
			FMemory::Memzero(DestPaddingPixelRight, BytesPerPixel);
		}
	}

	void FFontTextureAtlas::ZeroRow(const FCopyRowData& CopyRowData) const
	{
		const uint32 DestWidth = CopyRowData.DestTextureWidth;
		const uint32 DestRow = CopyRowData.DestRow;

		uint8* DestDataAddress = &CopyRowData.DestData[DestRow * DestWidth * BytesPerPixel];
		FMemory::Memzero(DestDataAddress, CopyRowData.RowWidth * BytesPerPixel);
	}

	void FFontTextureAtlas::CopyDataIntoSlot(const FAtlasedTextureSlot* SlotToCopyTo, const TArray<uint8>& Data)
	{
		// Copy pixel data to the texture
		uint8* Start = &AtlasData[SlotToCopyTo->Y * AtlasWidth * BytesPerPixel + SlotToCopyTo->X * BytesPerPixel];

		// Account for same padding on each sides
		const uint32 Padding = GetPaddingAmount();
		// const uint32 AllPadding = Padding * 2;
		constexpr uint32 AllPadding = 0;

		// Make sure the actual slot is not zero-area (otherwise padding could corrupt other images in the atlas)
		check(SlotToCopyTo->Width > AllPadding);
		check(SlotToCopyTo->Height > AllPadding);

		// The width of the source texture without padding (actual width)
		const uint32 SourceWidth = SlotToCopyTo->Width - AllPadding - 1;
		const uint32 SourceHeight = SlotToCopyTo->Height - AllPadding - 1;

		FCopyRowData CopyRowData;
		CopyRowData.DestData = Start;
		CopyRowData.SrcData = Data.GetData();
		CopyRowData.DestTextureWidth = AtlasWidth;
		CopyRowData.SrcTextureWidth = SourceWidth;
		CopyRowData.RowWidth = SlotToCopyTo->Width;

		// Apply the padding for bilinear filtering. 
		// Not used if no padding (assumes sampling outside boundaries of the sub texture is not possible)
		if (Padding > 0)
		{
			// Copy first color row into padding.  
			CopyRowData.SrcRow = 0;
			CopyRowData.DestRow = 0;

			ZeroRow(CopyRowData);
		}

		// Copy each row of the texture
		for (uint32 Row = Padding; Row < SlotToCopyTo->Height - Padding - 1; ++Row)
		{
			CopyRowData.SrcRow = Row - Padding;
			CopyRowData.DestRow = Row;

			CopyRow(CopyRowData);
		}

		if (Padding > 0)
		{
			// Copy last color row into padding row for bilinear filtering
			CopyRowData.SrcRow = SourceHeight - 1;
			CopyRowData.DestRow = SlotToCopyTo->Height - Padding;

			ZeroRow(CopyRowData);
		}
	}
	
	const FAtlasedTextureSlot* FFontTextureAtlas::FindSlotForTexture(uint32 InWidth, uint32 InHeight)
	{
		FAtlasedTextureSlot* ReturnVal = nullptr;

		// Account for padding on both sides
		const uint32 Padding = GetPaddingAmount();
		// const uint32 TotalPadding = Padding * 2;
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
}
