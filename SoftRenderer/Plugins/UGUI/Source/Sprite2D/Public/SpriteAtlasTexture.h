#pragma once

#include "CoreMinimal.h"

class USprite2D;

/**
 * Structure holding information about where a texture is located in the atlas. Inherits a linked-list interface.
 *
 * When a slot is occupied by texture data, the remaining space in the slot (if big enough) is split off into two new (smaller) slots,
 * building a tree of texture rectangles which, instead of being stored as a tree, are flattened into two linked-lists:
 *	- AtlasEmptySlots:	A linked-list of empty slots ready for texture data - iterates in same order as a depth-first-search on a tree
 *	- AtlasUsedSlots:	An unordered linked-list of slots containing texture data
 */
struct FSpriteAtlasTextureSlot : public TIntrusiveLinkedList<FSpriteAtlasTextureSlot>
{
	/** The X position of the sprite in the texture */
	uint32 X;
	/** The Y position of the sprite in the texture */
	uint32 Y;
	/** The width of the sprite */
	uint32 Width;
	/** The height of the sprite */
	uint32 Height;

	FSpriteAtlasTextureSlot( uint32 InX, uint32 InY, uint32 InWidth, uint32 InHeight)
		: TIntrusiveLinkedList<FSpriteAtlasTextureSlot>()
		, X(InX)
		, Y(InY)
		, Width(InWidth)
		, Height(InHeight)
	{
		
	}
};

struct FSpriteMergeAction
{
	TWeakObjectPtr<USprite2D> Sprite;

	uint32 X;
	uint32 Y;
	uint32 Width;
	uint32 Height;
};

class SPRITE2D_API FSpriteAtlasTexture : public TSharedFromThis<FSpriteAtlasTexture>
{
public:
	FSpriteAtlasTexture();
	virtual ~FSpriteAtlasTexture();
	
public:
	TWeakObjectPtr<UTexture2D> Texture;
	
    /** The list of atlas slots pointing to used texture data in the atlas */
    FSpriteAtlasTextureSlot* AtlasUsedSlots;
    /** The list of atlas slots pointing to empty texture data in the atlas */
    FSpriteAtlasTextureSlot* AtlasEmptySlots;
	
    uint32 AtlasWidth;
    uint32 AtlasHeight;
    	
    uint32 BlockSizeX;
    uint32 BlockSizeY;

	FName AtlasName;
	
    EPixelFormat PixelFormat;
	TextureCompressionSettings CompressionSettings;
	
    TArray<FSpriteMergeAction> MergeActions;
	
	TSet<TWeakObjectPtr<USprite2D>> Sprites;
	
	uint32 ValidSpriteCount;
	int32 InvalidSpriteCount;
	
	uint8 bSRGB : 1;
	uint8 bNeedUpdate : 1;
	uint8 bCPUMergeMode : 1;

public:
	static TSharedPtr<FSpriteAtlasTexture> CreateSpriteAtlasTexture(UTexture2D*& OutTargetTexture, FName InAtlasName, uint32 InWidth, uint32 InHeight, EPixelFormat InPixelFormat,
		uint32 InBlockSizeX, uint32 InBlockSizeY, TextureCompressionSettings InCompressionSettings, bool bInSRGB);

	FORCEINLINE bool IsInvalid() const
	{
		return Sprites.Num() > 0 && InvalidSpriteCount >= Sprites.Num();
	}
	
	void IncrementInvalidSpriteCount();

	FORCEINLINE void IncrementValidSpriteCount()
	{
		++ValidSpriteCount;
	}
	
	void DecrementValidSpriteCount();

#if WITH_EDITOR
	void RemoveSprite(USprite2D* Sprite2D);
#endif

protected:
	/**
	 * Clears atlas cpu data.  It does not clear rendering data
	 */
	void EmptyAtlasData();
	
	/**
	 * Creates enough space for a single texture the width and height of the atlas
	 */
	void InitAtlasData();

public:
	/**
	 * Finds the optimal slot for a texture in the atlas
	 * 
	 * @param InWidth The width of the texture we are adding
	 * @param InHeight The height of the texture we are adding
	 */
	const FSpriteAtlasTextureSlot* FindSlotForTexture(uint32 InWidth, uint32 InHeight);

	void AddMergeAction(USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height);

protected:
	void InternalAddMergeAction(USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height);

public:
	void MergeSpriteTextures(double& TimeLimit, uint64& StartTime);
	void MergeSpriteTextureImmediate(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height);

protected:
	void MergeSpriteTextureForCPU(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height, uint8* RawData) const;
	void MergeSpriteTextureForRHI(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height) const;
	
};
