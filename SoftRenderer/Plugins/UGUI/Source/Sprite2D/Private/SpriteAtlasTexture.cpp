#include "SpriteAtlasTexture.h"
#include "Sprite2D.h"
#include "Sprite2DModule.h"
#include "SpriteMergeSubsystem.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

#ifndef OPENGL_USE_DYNAMIC_UI_ALTAS
#define OPENGL_USE_DYNAMIC_UI_ALTAS 0
#endif

TAutoConsoleVariable<int32> CVarSpriteMergeMode(
	TEXT("Sprite2D.MergeMode"),
	0,
	TEXT("0 - GPU")
	TEXT("1 - CPU"), ECVF_Scalability);

TAutoConsoleVariable<int32> CVarDisableSprite2DMerge(
	TEXT("Sprite2D.DisableMerge"),
	0,
	TEXT("0 - Enable merge.")
	TEXT("1 - Disable merge."));

TAutoConsoleVariable<int32> CVarGPUMergeCount(
	TEXT("Sprite2D.GPUMergeCount"),
	15,
	TEXT("Frame interval for attempted merging."));

/////////////////////////////////////////////////////
// FSpriteAtlasTexture

static uint8 ASTC_4x4_TransparentBlock[] = { 252, 253, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0 };

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- FillTransparentBlocksIntoTexture"), STATGROUP_Sprite2D_FillTransparentBlocksIntoTexture, STATGROUP_Sprite2D);
void FillTransparentBlocksIntoTexture(EPixelFormat Format, uint8* RawData, const int32 NumBlocksX, const int32 NumBlocksY, const int32 BlockBytes)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_FillTransparentBlocksIntoTexture);
	
	if (Format == PF_ASTC_4x4)
	{
		const uint32 SrcPitch = NumBlocksX * BlockBytes; // Num-of bytes per row in the source data;
		uint8* ASTC_4x4_RowTransparentBlocks = new uint8[SrcPitch];
		uint8* RowRawData = ASTC_4x4_RowTransparentBlocks;
		
		for (int32 Index = 0; Index < NumBlocksX; ++Index)
		{
			FMemory::Memcpy(RowRawData, ASTC_4x4_TransparentBlock, BlockBytes);
			RowRawData += BlockBytes;
		}

		for (int32 Index = 0; Index < NumBlocksY; ++Index)
		{
			FMemory::Memcpy(RawData, ASTC_4x4_RowTransparentBlocks, SrcPitch);
			RawData += SrcPitch;
		}

		delete[] ASTC_4x4_RowTransparentBlocks;
	}
	else
	{
		FMemory::Memset(RawData, 0, NumBlocksX * NumBlocksY * BlockBytes);
	}
}

FSpriteAtlasTexture::FSpriteAtlasTexture()
{
	Texture = nullptr;
	AtlasUsedSlots = nullptr;
	AtlasEmptySlots = nullptr;

	AtlasWidth = 2048;
	AtlasHeight = 2048;

	BlockSizeX = 1;
	BlockSizeY = 1;

	AtlasName = NAME_None;
	
	PixelFormat = EPixelFormat::PF_Unknown;
	CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	
	ValidSpriteCount = 0;
	InvalidSpriteCount = 0;
	
	bSRGB = true;
	bNeedUpdate = false;
	bCPUMergeMode = false;
}

FSpriteAtlasTexture::~FSpriteAtlasTexture()
{
	EmptyAtlasData();

	MergeActions.Empty();
	
	for (auto& Sprite : Sprites)
	{
		if (Sprite.IsValid())
		{
			Sprite->DynamicSpriteTexture = nullptr;
			Sprite->SpriteAtlasTexture = nullptr;
			Sprite->NotifySpriteTextureChanged(true, true);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- CreateSpriteAtlasTexture"), STATGROUP_Sprite2D_CreateSpriteAtlasTexture, STATGROUP_Sprite2D);
TSharedPtr<FSpriteAtlasTexture> FSpriteAtlasTexture::CreateSpriteAtlasTexture(UTexture2D*& OutTargetTexture, FName InAtlasName, uint32 InWidth, uint32 InHeight,
                                                                              EPixelFormat InPixelFormat, uint32 InBlockSizeX, uint32 InBlockSizeY, TextureCompressionSettings InCompressionSettings, bool bInSRGB)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_CreateSpriteAtlasTexture);
	
	LLM_SCOPE(ELLMTag::Textures);

	const bool bIsCPUMergeMode = CVarSpriteMergeMode.GetValueOnGameThread() == 1;

	UTexture2D* NewTexture = nullptr;
	if (InWidth > 0 && InHeight > 0 && InBlockSizeX > 0 && InBlockSizeY > 0)
	{
		NewTexture = NewObject<UTexture2D>(
			GetTransientPackage(),
			NAME_None,
			RF_Transient
			);

		NewTexture->SRGB = bInSRGB;
		NewTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		NewTexture->MipLoadOptions = ETextureMipLoadOptions::OnlyFirstMip;
		NewTexture->NeverStream = true;

		NewTexture->PlatformData = new FTexturePlatformData();
		NewTexture->PlatformData->SizeX = InWidth;
		NewTexture->PlatformData->SizeY = InHeight;
		NewTexture->PlatformData->PixelFormat = InPixelFormat;
		
		FTexture2DMipMap* Mip = new FTexture2DMipMap();
		NewTexture->PlatformData->Mips.Add(Mip);
		
		Mip->SizeX = InWidth;
		Mip->SizeY = InHeight;
		
		Mip->BulkData.Lock(LOCK_READ_WRITE);

		const int32 NumBlocksX = (InWidth + InBlockSizeX - 1) / InBlockSizeX;
		const int32 NumBlocksY = (InHeight + InBlockSizeY - 1) / InBlockSizeY;

		uint8* RawData = (uint8*)Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InPixelFormat].BlockBytes);

		FillTransparentBlocksIntoTexture(InPixelFormat, RawData, NumBlocksX, NumBlocksY, GPixelFormats[InPixelFormat].BlockBytes);
		
#if WITH_EDITORONLY_DATA
		if (bIsCPUMergeMode)
		{
			NewTexture->Source.Init(InWidth, InHeight, 1, 1, ETextureSourceFormat::TSF_BGRA8, RawData);
		}
#endif
        
		Mip->BulkData.Unlock();
		
		NewTexture->UpdateResource();

		if (!bIsCPUMergeMode)
		{
			Mip->BulkData.RemoveBulkData();
		}
	}
	else
	{
		UE_LOG(LogSprite2D, Warning, TEXT("Invalid parameters specified for FSpriteAtlasTexture::CreateSpriteAtlasTexture()"));
	}

	if (IsValid(NewTexture))
	{
		OutTargetTexture = NewTexture;
		
		TSharedPtr<FSpriteAtlasTexture> NewAtlasTexture = MakeShareable(new FSpriteAtlasTexture());
		NewAtlasTexture->Texture = NewTexture;
		NewAtlasTexture->AtlasName = InAtlasName;
		NewAtlasTexture->AtlasWidth = InWidth;
		NewAtlasTexture->AtlasHeight = InHeight;
		NewAtlasTexture->BlockSizeX = InBlockSizeX;
		NewAtlasTexture->BlockSizeY = InBlockSizeY;
		NewAtlasTexture->PixelFormat = InPixelFormat;
		NewAtlasTexture->CompressionSettings = InCompressionSettings;
        NewAtlasTexture->bSRGB = bInSRGB;
		NewAtlasTexture->bCPUMergeMode = bIsCPUMergeMode;
		NewAtlasTexture->InitAtlasData();
		return NewAtlasTexture;
	}
	
	return nullptr;
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- IncrementInvalidSpriteCount"), STATGROUP_Sprite2D_IncrementInvalidSpriteCount, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::IncrementInvalidSpriteCount()
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_IncrementInvalidSpriteCount);
	
	++InvalidSpriteCount;

	if (InvalidSpriteCount >= Sprites.Num() && GEngine)
	{
		USpriteMergeSubsystem* Subsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
		if (IsValid(Subsystem))
		{
			Subsystem->RemoveSpriteTexture(SharedThis(this));
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- DecrementValidSpriteCount"), STATGROUP_Sprite2D_DecrementValidSpriteCount, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::DecrementValidSpriteCount()
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_DecrementValidSpriteCount);
	
	--ValidSpriteCount;

	if (ValidSpriteCount <= 0 && GEngine)
	{
		USpriteMergeSubsystem* Subsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
		if (IsValid(Subsystem) && (Subsystem->IsDiscardUnreferencedTexture() || AtlasName != NAME_None))
		{
			Subsystem->RemoveSpriteTexture(SharedThis(this));
		}
	}
}

#if WITH_EDITOR

void FSpriteAtlasTexture::RemoveSprite(USprite2D* Sprite2D)
{
	for (auto& Sprite : Sprites)
	{
		if (Sprite.IsValid())
		{
			if (Sprite.Get() == Sprite2D)
			{
				Sprites.Remove(Sprite);
				if (Sprite2D->ReferenceCount > 0)
				{
					DecrementValidSpriteCount();
				}
				IncrementInvalidSpriteCount();
				return;
			}
		}
	}
}

#endif

void FSpriteAtlasTexture::EmptyAtlasData()
{
	FMemMark Mark(FMemStack::Get());

	// Remove all nodes
	TArray<FSpriteAtlasTextureSlot*, TMemStackAllocator<>> DeleteSlots;

	for (FSpriteAtlasTextureSlot::TIterator SlotIt(AtlasUsedSlots); SlotIt; SlotIt.Next())
	{
		FSpriteAtlasTextureSlot& CurSlot = *SlotIt;
		DeleteSlots.Add(&CurSlot);
	}

	for (FSpriteAtlasTextureSlot::TIterator SlotIt(AtlasEmptySlots); SlotIt; SlotIt.Next())
	{
		FSpriteAtlasTextureSlot& CurSlot = *SlotIt;
		DeleteSlots.Add(&CurSlot);
	}

	AtlasUsedSlots = nullptr;
	AtlasEmptySlots = nullptr;

	for (const FSpriteAtlasTextureSlot* CurSlot : DeleteSlots)
	{
		delete CurSlot;
	}

	DeleteSlots.Empty();
}

void FSpriteAtlasTexture::InitAtlasData()
{
	FSpriteAtlasTextureSlot* RootSlot = new FSpriteAtlasTextureSlot(0, 0, AtlasWidth, AtlasHeight);
	RootSlot->LinkHead(AtlasEmptySlots);
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- FindSlotForTexture"), STATGROUP_Sprite2D_FindSlotForTexture, STATGROUP_Sprite2D);
const FSpriteAtlasTextureSlot* FSpriteAtlasTexture::FindSlotForTexture(uint32 InWidth, uint32 InHeight)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_FindSlotForTexture);
	
	FSpriteAtlasTextureSlot* ReturnVal = nullptr;

	// Account for padding on both sides
	int32 Columns = InWidth / BlockSizeX + 1;
	if (AtlasUsedSlots == nullptr && InWidth == AtlasWidth)
	{
		--Columns;
	}
	
	int32 Rows = InHeight / BlockSizeY + 1;
	if (AtlasUsedSlots == nullptr && InHeight == AtlasHeight)
	{
		--Rows;
	}
	
	const uint32 PaddedWidth = Columns * BlockSizeX;
	const uint32 PaddedHeight = Rows * BlockSizeY;

	// Previously, slots were stored as a binary tree - this has been replaced with a linked-list of slots on the edge of the tree
	// (slots on the edge of the tree represent empty slots); this iterates empty slots in same order as a binary depth-first-search,
	// except much faster.
	for (FSpriteAtlasTextureSlot::TIterator SlotIt(AtlasEmptySlots); SlotIt; ++SlotIt)
	{
		FSpriteAtlasTextureSlot& CurSlot = *SlotIt;
		if (PaddedWidth <= CurSlot.Width && PaddedHeight <= CurSlot.Height)
		{
			ReturnVal = &CurSlot;
			break;
		}
	}
	
	if (ReturnVal != nullptr)
	{
		// The width and height of the new child node
		const uint32 RemainingWidth =  FMath::Max<int32>(0, ReturnVal->Width - PaddedWidth);
		const uint32 RemainingHeight = FMath::Max<int32>(0, ReturnVal->Height - PaddedHeight);

		// New slots must have a minimum width/height, to avoid excessive slots i.e. excessive memory usage and iteration.
		// No glyphs seem to use slots this small, and cutting these slots out improves performance/memory-usage a fair bit
		constexpr uint32 MinSlotDim = 2;

		// Split the remaining area around this slot into two children.
		if (RemainingHeight >= MinSlotDim || RemainingWidth >= MinSlotDim)
		{
			FSpriteAtlasTextureSlot* LeftSlot;
			FSpriteAtlasTextureSlot* RightSlot;

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
				LeftSlot = new FSpriteAtlasTextureSlot(ReturnVal->X, ReturnVal->Y + PaddedHeight, PaddedWidth, RemainingHeight);
				RightSlot = new FSpriteAtlasTextureSlot(ReturnVal->X + PaddedWidth, ReturnVal->Y, RemainingWidth, ReturnVal->Height);
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
				LeftSlot = new FSpriteAtlasTextureSlot(ReturnVal->X + PaddedWidth, ReturnVal->Y, RemainingWidth, PaddedHeight);
				RightSlot = new FSpriteAtlasTextureSlot(ReturnVal->X, ReturnVal->Y + PaddedHeight, ReturnVal->Width, RemainingHeight);
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
		ReturnVal->Width = InWidth;
		ReturnVal->Height = InHeight;
	}

	return ReturnVal;
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- AddMergeAction"), STATGROUP_Sprite2D_AddMergeAction, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::AddMergeAction(USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_AddMergeAction);
	
	Sprites.Add(Sprite);
	InternalAddMergeAction(Sprite, X, Y, Width, Height);
}

void FSpriteAtlasTexture::InternalAddMergeAction(USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height)
{
	if (USpriteMergeSubsystem::LoadingScreenCounter > 0)
	{
		if (Sprite->GetReferenceCount() > 0)
		{
			++ValidSpriteCount;
		}

		MergeSpriteTextureImmediate(Sprite, X, Y,  Width, Height);
	}
	else
	{
		bNeedUpdate = true;

		FSpriteMergeAction Action;
		Action.Sprite = Sprite;
		Action.X = X;
		Action.Y = Y;
		Action.Width = Width;
		Action.Height = Height;
	
		MergeActions.Emplace(Action);
	
		if (Sprite->GetReferenceCount() > 0)
		{
			++ValidSpriteCount;
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- MergeSpriteTextures"), STATGROUP_Sprite2D_MergeSpriteTextures, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::MergeSpriteTextures(double& TimeLimit, uint64& StartTime)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_MergeSpriteTextures);
	
	if (!bNeedUpdate)
		return;
	
	if (!Texture.IsValid())
		return;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (CVarDisableSprite2DMerge.GetValueOnGameThread() > 0)
	{
		return;
	}
#endif
	
	if (bCPUMergeMode)
	{
		uint8* RawData = (uint8*)Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		
		int32 Index = 0;
		for (const int32 Count = MergeActions.Num(); Index < Count; ++Index)
		{
			FSpriteMergeAction& SpriteMergeAction = MergeActions[Index];
			MergeSpriteTextureForCPU(SpriteMergeAction.Sprite.Get(), SpriteMergeAction.X, SpriteMergeAction.Y, SpriteMergeAction.Width, SpriteMergeAction.Height, RawData);

			{
				const uint64 EndTime = FPlatformTime::Cycles64();

				TimeLimit -= FPlatformTime::ToSeconds64(EndTime - StartTime);
				if (TimeLimit <= 0 && USpriteMergeSubsystem::LoadingScreenCounter == 0)
				{
					++Index;
					Texture->PlatformData->Mips[0].BulkData.Unlock();
					MergeActions.RemoveAt(0, Index, false);
					return;
				}
			
				StartTime = EndTime;
			}
		}
		
		Texture->PlatformData->Mips[0].BulkData.Unlock();

		if (Index > 0)
		{
			MergeActions.RemoveAt(0, Index, false);
		}

#if USE_DYNAMIC_UI_ALTAS
		Texture->UpdateSprite2DTexture();
#else
		Texture->UpdateResource();
#endif
	}
	else
	{
		const int32 GPUMergeCount = CVarGPUMergeCount.GetValueOnGameThread();

		int32 Index = 0;
		for (const int32 Count = GPUMergeCount > 0 ? FMath::Min(GPUMergeCount, MergeActions.Num()) : MergeActions.Num(); Index < Count; ++Index)
		{
			FSpriteMergeAction& SpriteMergeAction = MergeActions[Index];
			MergeSpriteTextureForRHI(SpriteMergeAction.Sprite.Get(), SpriteMergeAction.X, SpriteMergeAction.Y, SpriteMergeAction.Width, SpriteMergeAction.Height);

			{
				const uint64 EndTime = FPlatformTime::Cycles64();

				TimeLimit -= FPlatformTime::ToSeconds64(EndTime - StartTime);
				if (TimeLimit <= 0 && USpriteMergeSubsystem::LoadingScreenCounter == 0)
				{
					++Index;
					MergeActions.RemoveAt(0, Index, false);
					return;
				}
			
				StartTime = EndTime;
			}
		}

		if (Index > 0)
		{
			MergeActions.RemoveAt(0, Index, false);
		}
	}

	bNeedUpdate = MergeActions.Num() > 0;

	{
		const uint64 EndTime = FPlatformTime::Cycles64();
		TimeLimit -= FPlatformTime::ToSeconds64(EndTime - StartTime);
		StartTime = EndTime;
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- MergeSpriteTextureImmediate"), STATGROUP_Sprite2D_MergeSpriteTextureImmediate, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::MergeSpriteTextureImmediate(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_MergeSpriteTextureImmediate);
	if (!Texture.IsValid())
		return;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (CVarDisableSprite2DMerge.GetValueOnGameThread() > 0)
	{
		return;
	}
#endif
	
	if (bCPUMergeMode)
	{
		uint8* RawData = (uint8*)Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		MergeSpriteTextureForCPU(Sprite, X, Y, Width, Height, RawData);
		Texture->PlatformData->Mips[0].BulkData.Unlock();
#if USE_DYNAMIC_UI_ALTAS
		Texture->UpdateSprite2DTexture();
#else
		Texture->UpdateResource();
#endif
	}
	else
	{
		MergeSpriteTextureForRHI(Sprite, X, Y, Width, Height);
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- MergeSpriteTextureForCPU"), STATGROUP_Sprite2D_MergeSpriteTextureForCPU, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::MergeSpriteTextureForCPU(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height, uint8* RawData) const
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_MergeSpriteTextureForCPU);
	
	if (!IsValid(Sprite))
		return;
	
	UTexture2D* SpriteSourceTexture = Sprite->GetSourceTexture();
	if (!IsValid(SpriteSourceTexture))
		return;

	const int32 BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
	constexpr int32 MipIndex = 0;

	uint32 AtlasWidthInBlocks;
	{
		const uint32 WidthInTexels = FMath::Max<uint32>(AtlasWidth >> MipIndex, 1);
		AtlasWidthInBlocks = (WidthInTexels + BlockSizeX - 1) / BlockSizeX;
	}
	
	uint32 WidthInBlocks;
	{
		const uint32 WidthInTexels = FMath::Max<uint32>(Width >> MipIndex, 1);
		WidthInBlocks = (WidthInTexels + BlockSizeX - 1) / BlockSizeX;
	}

	uint32 HeightInBlocks;
	{
		const uint32 HeightInTexels = FMath::Max<uint32>(Height >> MipIndex, 1);
		HeightInBlocks = (HeightInTexels + BlockSizeY - 1) / BlockSizeY;
	}

#if USE_DYNAMIC_UI_ALTAS
	if (SpriteSourceTexture->Sprite2DTextureType != GPU)
	{
		uint8* SourceData = (uint8*)SpriteSourceTexture->GetFirstMipData();
		check(SourceData);

		uint8* DestData = RawData + (Y / BlockSizeY * AtlasWidthInBlocks + X / BlockSizeX) * BlockBytes;
	
		const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
		const uint32 AtlasSrcPitch = AtlasWidthInBlocks * BlockBytes;

		for (uint32 Index = 0; Index < HeightInBlocks; ++Index)
		{
			FMemory::Memcpy(DestData, SourceData, SrcPitch);
			DestData += AtlasSrcPitch;
			SourceData += SrcPitch;
		}
	}
	else
	{
		const int32 TextureMipMapSize = WidthInBlocks * HeightInBlocks * BlockBytes;
		uint8** MipData = new uint8*[1];
		MipData[0] = new uint8[TextureMipMapSize];
	
		SpriteSourceTexture->GetMipData(0, (void**)MipData);
		uint8* SourceData = MipData[0];

		uint8* DestData = RawData + (Y / BlockSizeY * AtlasWidthInBlocks + X / BlockSizeX) * BlockBytes;
	
		const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
		const uint32 AtlasSrcPitch = AtlasWidthInBlocks * BlockBytes;

		for (uint32 Index = 0; Index < HeightInBlocks; ++Index)
		{
			FMemory::Memcpy(DestData, SourceData, SrcPitch);
			DestData += AtlasSrcPitch;
			SourceData += SrcPitch;
		}

		delete[] MipData[0];
		delete[] MipData;
	}
#else
	const int32 TextureMipMapSize = WidthInBlocks * HeightInBlocks * BlockBytes;
	uint8** MipData = new uint8*[1];
	MipData[0] = new uint8[TextureMipMapSize];
	
	SpriteSourceTexture->GetMipData(0, (void**)MipData);
	uint8* SourceData = MipData[0];

	uint8* DestData = RawData + (Y / BlockSizeY * AtlasWidthInBlocks + X / BlockSizeX) * BlockBytes;
	
	const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
	const uint32 AtlasSrcPitch = AtlasWidthInBlocks * BlockBytes;
	
	for (uint32 Index = 0; Index < HeightInBlocks; ++Index)
	{
		FMemory::Memcpy(DestData, SourceData, SrcPitch);
		DestData += AtlasSrcPitch;
		SourceData += SrcPitch;
	}
	
	delete[] MipData[0];
	delete[] MipData;
#endif
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- MergeSpriteTextureForRHI"), STATGROUP_Sprite2D_MergeSpriteTextureForRHI, STATGROUP_Sprite2D);
void FSpriteAtlasTexture::MergeSpriteTextureForRHI(const USprite2D* Sprite, uint32 X, uint32 Y, uint32 Width, uint32 Height) const
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_MergeSpriteTextureForRHI);
	
	if (!IsValid(Sprite))
		return;
	
	UTexture2D* SpriteSourceTexture = Sprite->GetSourceTexture();
	if (!IsValid(SpriteSourceTexture))
		return;

	const int32 BlockBytes = GPixelFormats[PixelFormat].BlockBytes;
	constexpr int32 MipIndex = 0;
	
	uint32 WidthInBlocks;
	{
		const uint32 WidthInTexels = FMath::Max<uint32>(Width >> MipIndex, 1);
		WidthInBlocks = (WidthInTexels + BlockSizeX - 1) / BlockSizeX;
	}

	uint32 HeightInBlocks;
	{
		const uint32 HeightInTexels = FMath::Max<uint32>(Height >> MipIndex, 1);
		HeightInBlocks = (HeightInTexels + BlockSizeY - 1) / BlockSizeY;
	}
	
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D;
	Region->DestX = X;
	Region->DestY = Y;
	Region->SrcX = 0;
	Region->SrcY = 0;
	
#if PLATFORM_IOS
	Region->Width = WidthInBlocks * BlockSizeX;
	Region->Height = HeightInBlocks * BlockSizeY;
#else
	Region->Width = Width;
	Region->Height = Height;
#endif

	// for OpenGL
#if OPENGL_USE_DYNAMIC_UI_ALTAS 
	Region->DataSize = WidthInBlocks * BlockBytes * HeightInBlocks;
#endif

#if USE_DYNAMIC_UI_ALTAS
	if (SpriteSourceTexture->Sprite2DTextureType != GPU)
	{
		uint8* FirstMipData = (uint8*)SpriteSourceTexture->GetFirstMipData();
		check(FirstMipData);
	
		const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
	
		// SrcBpp has no effect on compressed pictures, SrcX and SrcY must be 0
		Texture->UpdateTextureRegions(0, 1, Region, SrcPitch, 0, FirstMipData,
		[this](uint8* SrcData, const FUpdateTextureRegion2D* Region)
		{
			delete Region;
		});
	}
	else
	{
		const int32 TextureMipMapSize = WidthInBlocks * HeightInBlocks * BlockBytes;
		uint8** MipData = new uint8*[1];
		MipData[0] = new uint8[TextureMipMapSize];
	
		SpriteSourceTexture->GetMipData(0, (void**)MipData);
	
		const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
	
		// SrcBpp has no effect on compressed pictures, SrcX and SrcY must be 0
		Texture->UpdateTextureRegions(0, 1, Region, SrcPitch, 0, MipData[0],
		[this, MipData](uint8* SrcData, const FUpdateTextureRegion2D* Region)
		{
			delete[] MipData[0];
			delete[] MipData;
			
			delete Region;
		});
	}
#else
	const int32 TextureMipMapSize = WidthInBlocks * HeightInBlocks * BlockBytes;
	uint8** MipData = new uint8*[1];
	MipData[0] = new uint8[TextureMipMapSize];
	
	SpriteSourceTexture->GetMipData(0, (void**)MipData);
	
	const uint32 SrcPitch = WidthInBlocks * BlockBytes; // Num-of bytes per row in the source data;
	
	// SrcBpp has no effect on compressed pictures, SrcX and SrcY must be 0
	Texture->UpdateTextureRegions(0, 1, Region, SrcPitch, 0, MipData[0],
	[this, MipData](uint8* SrcData, const FUpdateTextureRegion2D* Region)
	{
		delete[] MipData[0];
		delete[] MipData;
			
		delete Region;
	});
#endif
}

/////////////////////////////////////////////////////
