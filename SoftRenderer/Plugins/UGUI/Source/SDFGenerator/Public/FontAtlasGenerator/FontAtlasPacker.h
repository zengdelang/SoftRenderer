#pragma once

#include "GlyphGeometry.h"
#include "FontAtlasGenerator/FontTextureAtlas.h"

namespace FontAtlasGenerator
{
	class SDFGENERATOR_API FFontAtlasPacker
	{
	public:
		FFontAtlasPacker()
			: AtlasUsedSlots(nullptr)
			, AtlasEmptySlots(nullptr)
			, AtlasPadding(0)
		{
			InitAtlasData();
		}

		virtual ~FFontAtlasPacker();
		
		void EmptyAtlasData();

		/**
		 * Finds the optimal slot for a texture in the atlas
		 * 
		 * @param InWidth The width of the texture we are adding
		 * @param InHeight The height of the texture we are adding
		 */
		const FAtlasedTextureSlot* FindSlotForTexture(uint32 InWidth, uint32 InHeight);
		
		void InitAtlasData();

		void Pack(const TArray<FGlyphGeometry*>& GlyphGeometries, bool bOverlapSupport, bool bScanlinePass, FVector2D MaxAtlasSize, int32& CurIndex, int32 Count, class ISDFGeneratorLogListener* LogListener = nullptr);

	private:
		FORCEINLINE int32 GetPaddingAmount() const
		{
			return AtlasPadding;
		}

	public:
		uint32 GetAtlasWidth() const { return AtlasWidth; }
		uint32 GetAtlasHeight() const { return AtlasHeight; }
		
	public:
		TArray<TArray<uint8>> AtlasDataList;

	protected:
		/** The list of atlas slots pointing to used texture data in the atlas */
		FAtlasedTextureSlot* AtlasUsedSlots;
		/** The list of atlas slots pointing to empty texture data in the atlas */
		FAtlasedTextureSlot* AtlasEmptySlots;

		/** Width of the atlas */
		uint32 AtlasWidth;
		/** Height of the atlas */
		uint32 AtlasHeight;

		uint32 AtlasPadding;
	};
}
