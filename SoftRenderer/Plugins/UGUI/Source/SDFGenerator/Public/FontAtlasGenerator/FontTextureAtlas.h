#pragma once

namespace FontAtlasGenerator
{
	/**
	 * Structure holding information about where a texture is located in the atlas. Inherits a linked-list interface.
	 *
	 * When a slot is occupied by texture data, the remaining space in the slot (if big enough) is split off into two new (smaller) slots,
	 * building a tree of texture rectangles which, instead of being stored as a tree, are flattened into two linked-lists:
	 *	- AtlasEmptySlots:	A linked-list of empty slots ready for texture data - iterates in same order as a depth-first-search on a tree
	 *	- AtlasUsedSlots:	An unordered linked-list of slots containing texture data
	 */
	struct FAtlasedTextureSlot : public TIntrusiveLinkedList<FAtlasedTextureSlot>
	{
		/** The X position of the character in the texture */
		uint32 X;
		/** The Y position of the character in the texture */
		uint32 Y;
		/** The width of the character */
		uint32 Width;
		/** The height of the character */
		uint32 Height;
		/** Uniform Padding. can only be zero or one. See ESlateTextureAtlasPaddingStyle. */
		uint8 Padding;

		FAtlasedTextureSlot( uint32 InX, uint32 InY, uint32 InWidth, uint32 InHeight, uint8 InPadding )
			: TIntrusiveLinkedList<FAtlasedTextureSlot>()
			, X(InX)
			, Y(InY)
			, Width(InWidth)
			, Height(InHeight)
			, Padding(InPadding)
		{
		}
	};
	
	class FFontTextureAtlas
	{
	public:
		FFontTextureAtlas(uint32 InWidth, uint32 InHeight, uint32 InBytesPerPixel)
		      : AtlasData()
			  , AtlasUsedSlots(nullptr)
			  , AtlasEmptySlots(nullptr)
			  , AtlasWidth(InWidth)
			  , AtlasHeight(InHeight)
			  , BytesPerPixel(InBytesPerPixel)
			  , AtlasPadding(0)
		{
			InitAtlasData();
		}

		virtual ~FFontTextureAtlas();

		/**
		 * Clears atlas cpu data.  It does not clear rendering data
		 */
		void EmptyAtlasData();

		/**
		 * Adds a texture to the atlas
		 *
		 * @param TextureWidth	Width of the texture
		 * @param TextureHeight	Height of the texture
		 * @param Data			Raw texture data
		 */
		const FAtlasedTextureSlot* AddTexture(uint32 TextureWidth, uint32 TextureHeight, const TArray<uint8>& Data);

		/** @return the width of the atlas */
		uint32 GetWidth() const { return AtlasWidth; }
		/** @return the height of the atlas */
		uint32 GetHeight() const { return AtlasHeight; }

	public:
		/**
		 * Finds the optimal slot for a texture in the atlas
		 * 
		 * @param InWidth The width of the texture we are adding
		 * @param InHeight The height of the texture we are adding
		 */
		const FAtlasedTextureSlot* FindSlotForTexture(uint32 InWidth, uint32 InHeight);
		
		/**
		 * Creates enough space for a single texture the width and height of the atlas
		 */
		void InitAtlasData();

		struct FCopyRowData
		{
			/** Source data to copy */
			const uint8* SrcData;
			/** Place to copy data to */
			uint8* DestData;
			/** The row number to copy */
			uint32 SrcRow;
			/** The row number to copy to */
			uint32 DestRow;
			/** The width of a source row */
			uint32 RowWidth;
			/** The width of the source texture */
			uint32 SrcTextureWidth;
			/** The width of the dest texture */
			uint32 DestTextureWidth;
		};

		/**
		 * Copies a single row from a source texture to a dest texture,
		 * respecting the padding.
		 *
		 * @param CopyRowData	Information for how to copy a row
		 */
		void CopyRow(const FCopyRowData& CopyRowData) const;

		/**
		 * Zeros out a row in the dest texture (used with PaddingStyle == PadWithZero).
		 * respecting the padding.
		 *
		 * @param CopyRowData	Information for how to copy a row
		 */
		void ZeroRow(const FCopyRowData& CopyRowData) const;

		/** 
		 * Copies texture data into the atlas at a given slot
		 *
		 * @param SlotToCopyTo	The occupied slot in the atlas where texture data should be copied to
		 * @param Data			The data to copy into the atlas
		 */
		void CopyDataIntoSlot(const FAtlasedTextureSlot* SlotToCopyTo, const TArray<uint8>& Data);

	private:
		/** Returns the amount of padding needed for the current padding style */
		FORCEINLINE int32 GetPaddingAmount() const
		{
			return AtlasPadding;
		}

	public:
		/** Actual texture data contained in the atlas */
		TArray<uint8> AtlasData;

	protected:
		/** The list of atlas slots pointing to used texture data in the atlas */
		FAtlasedTextureSlot* AtlasUsedSlots;

		/** The list of atlas slots pointing to empty texture data in the atlas */
		FAtlasedTextureSlot* AtlasEmptySlots;

		/** Width of the atlas */
		uint32 AtlasWidth;

		/** Height of the atlas */
		uint32 AtlasHeight;

		/** Bytes per pixel in the atlas */
		uint32 BytesPerPixel;

		uint32 AtlasPadding;
	};
}
