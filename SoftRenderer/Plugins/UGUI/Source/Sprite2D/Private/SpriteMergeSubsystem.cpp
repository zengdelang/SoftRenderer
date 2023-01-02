#include "SpriteMergeSubsystem.h"
#include "Sprite2D.h"
#include "Sprite2DModule.h"
#include "SpriteAtlasTexture.h"

#ifndef USE_DYNAMIC_UI_ALTAS
#define USE_DYNAMIC_UI_ALTAS 0
#endif

TAutoConsoleVariable<int32> CVarMaxAtlasCountForTrimMemory(
	TEXT("Sprite2D.MaxAtlasCountForTrimMemory"),
	3,
	TEXT("Greater than this value, try to release the old texture."), ECVF_Scalability);

TAutoConsoleVariable<float> CVarMergeSpriteTimeLimit(
	TEXT("Sprite2D.MergeSpriteTimeLimit"),
	0.00015,
	TEXT("Set the maximum time of a single frame to modify the physical state."), ECVF_Scalability);

/////////////////////////////////////////////////////
// USpriteMergeSubsystem

FOnSpriteMergeTick USpriteMergeSubsystem::OnSpriteMergeTick;
FOnSpriteMergeTick USpriteMergeSubsystem::OnSpriteMergeTryAgainTick;

int32 USpriteMergeSubsystem::AtlasWidth = 2048;
int32 USpriteMergeSubsystem::AtlasHeight = 2048;
uint32 USpriteMergeSubsystem::LoadingScreenCounter = 0;

USpriteMergeSubsystem::USpriteMergeSubsystem()
{
	TimeLimit = 0;
	bDirty = true;
	bDiscardUnreferencedTexture = false;
}

bool USpriteMergeSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return FApp::CanEverRender();
}

void USpriteMergeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if USE_DYNAMIC_UI_ALTAS
	UTexture2D::SetAtlasTextureSize(AtlasWidth, AtlasHeight);
#endif

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USpriteMergeSubsystem::PreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USpriteMergeSubsystem::PostLoadMapWithWorld);
	
	OnSpriteMergeTick.AddUObject(this, &USpriteMergeSubsystem::TickInternal);
	OnSpriteMergeTryAgainTick.AddUObject(this, &USpriteMergeSubsystem::TryAgainTick);
}

void USpriteMergeSubsystem::Deinitialize()
{
	OnSpriteMergeTick.RemoveAll(this);
	OnSpriteMergeTryAgainTick.RemoveAll(this);
	
    SpriteTextures.Empty();
	Textures.Empty();
	SpritePendingActions.Empty();
	
	Super::Deinitialize();
}

void USpriteMergeSubsystem::PreLoadMap(const FString& MapName)
{
	bDiscardUnreferencedTexture = true;

	for (int32 Index = SpriteTextures.Num() - 1; Index >= 0; --Index)
	{
		const auto& SpriteTexture = SpriteTextures[Index];
		if (SpriteTexture.IsValid() && SpriteTexture->ValidSpriteCount <= 0)
		{
			RemoveSpriteTexture(SpriteTexture);
		}
	}
}

void USpriteMergeSubsystem::PostLoadMapWithWorld(UWorld* LoadedWorld)
{
	bDiscardUnreferencedTexture = false;
}

void USpriteMergeSubsystem::ClearSprite2DAtlas()
{
	for (int32 Index = SpriteTextures.Num() - 1; Index >= 0; --Index)
	{
		const auto& SpriteTexture = SpriteTextures[Index];
		if (SpriteTexture.IsValid() && SpriteTexture->ValidSpriteCount <= 0)
		{
			RemoveSpriteTexture(SpriteTexture);
		}
	}
}

void USpriteMergeSubsystem::TickInternal()
{
	TimeLimit = CVarMergeSpriteTimeLimit.GetValueOnGameThread();
	Tick();
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- Tick"), STATGROUP_Sprite2D_Tick, STATGROUP_Sprite2D);
void USpriteMergeSubsystem::Tick()
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_Tick);

	if (TimeLimit <= 0 && LoadingScreenCounter == 0)
	{
		return;
	}
	
	uint64 StartTime = FPlatformTime::Cycles64();
	
	bDirty = false;
	
	for (int32 Index = SpritePendingActions.Num() - 1; Index >= 0; --Index)
	{
		const auto& Action = SpritePendingActions[Index];

		if (!Action.Sprite.IsValid())
		{
			SpritePendingActions.RemoveAt(Index);
			continue;
		}

		USprite2D* Sprite = Action.Sprite.Get();
		const UTexture2D* SpriteSourceTexture = Sprite->GetSourceTexture();
		if (!IsValid(SpriteSourceTexture))
		{
			Sprite->bWaitingTextureData = false;
			SpritePendingActions.RemoveAt(Index);
			continue;
		}
		
		if (SpriteSourceTexture->IsReadyForAsyncPostLoad())
		{
			Sprite->bWaitingTextureData = false;
			Sprite->DynamicSpriteTexture = InternalGetSpriteTexture2D(Sprite, Sprite->AtlasName);
			Sprite->NotifySpriteTextureChanged(true, true);
			SpritePendingActions.RemoveAt(Index);

			{
				const uint64 EndTime = FPlatformTime::Cycles64();

				TimeLimit -= FPlatformTime::ToSeconds64(EndTime - StartTime);
				if (TimeLimit <= 0 && LoadingScreenCounter == 0)
				{
					return;
				}
			
				StartTime = EndTime;
			}
		}
	}
	
	for (int32 Index = 0, Count = SpriteTextures.Num(); Index < Count; ++Index)
	{
		const auto& SpriteAtlasTexture = SpriteTextures[Index];
		if (SpriteAtlasTexture.IsValid())
		{
			SpriteAtlasTexture->MergeSpriteTextures(TimeLimit, StartTime);
			
			{
				if (TimeLimit <= 0 && LoadingScreenCounter == 0)
				{
					return;
				}
				
				const uint64 EndTime = FPlatformTime::Cycles64();

				TimeLimit -= FPlatformTime::ToSeconds64(EndTime - StartTime);
				if (TimeLimit <= 0 && LoadingScreenCounter == 0)
				{
					return;
				}
			
				StartTime = EndTime;
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- TryAgainTick"), STATGROUP_Sprite2D_TryAgainTick, STATGROUP_Sprite2D);
void USpriteMergeSubsystem::TryAgainTick()
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_TryAgainTick);
	
	if (bDirty)
	{
		Tick();
	}
	
	bDirty = false;
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- FindValidAtlasTexture"), STATGROUP_Sprite2D_FindValidAtlasTexture, STATGROUP_Sprite2D);
TSharedPtr<FSpriteAtlasTexture> USpriteMergeSubsystem::FindValidAtlasTexture(const FName& TargetAtlasName, const EPixelFormat PixelFormat,
	const TextureCompressionSettings CompressionSettings, const bool bSRGB, uint32& X, uint32& Y, uint32& Width, uint32& Height, const FVector2D& SourceDimension)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_FindValidAtlasTexture);
	
	TArray<TSharedPtr<FSpriteAtlasTexture>, TInlineAllocator<8>> UsedSpriteTextures;
	
	for (int32 Index = SpriteTextures.Num() - 1; Index >= 0; --Index)
	{
		const auto& SpriteTexture = SpriteTextures[Index];
		if (SpriteTexture.IsValid())
		{
			if (bDiscardUnreferencedTexture && SpriteTexture->ValidSpriteCount <= 0)
			{
				RemoveSpriteTexture(SpriteTexture);
				continue;
			}
			
			if (SpriteTexture->AtlasName == TargetAtlasName &&
				SpriteTexture->PixelFormat == PixelFormat && SpriteTexture->CompressionSettings == CompressionSettings &&
				SpriteTexture->bSRGB == bSRGB)
			{
				if (SpriteTexture->IsInvalid())
				{
					RemoveSpriteTexture(SpriteTexture);
				}
				else
				{
					UsedSpriteTextures.Add(SpriteTexture);
				}
			}
		}
	}
	
	if (UsedSpriteTextures.Num() >= CVarMaxAtlasCountForTrimMemory.GetValueOnGameThread())
	{
		for (int32 Index = UsedSpriteTextures.Num() - 1; Index >= 0; --Index)
		{
			const auto& SpriteTexture = UsedSpriteTextures[Index];
			if (SpriteTexture->ValidSpriteCount <= 0)
			{
				RemoveSpriteTexture(SpriteTexture);
				UsedSpriteTextures.RemoveAt(Index);
				
				if (UsedSpriteTextures.Num() < CVarMaxAtlasCountForTrimMemory.GetValueOnGameThread())
				{
					break;
				}
			}
		}
	}
	
	for (int32 Index = 0, Count = UsedSpriteTextures.Num(); Index < Count; ++Index)
	{
		const auto& SpriteTexture = UsedSpriteTextures[Index];
		const auto AtlasSlot = SpriteTexture->FindSlotForTexture(SourceDimension.X, SourceDimension.Y);
		if (AtlasSlot)
		{
			X = AtlasSlot->X;
			Y = AtlasSlot->Y;
			Width = AtlasSlot->Width;
			Height = AtlasSlot->Height;
			return SpriteTexture;
		}
	}
	
	return nullptr;
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- GetSpriteTexture2D"), STATGROUP_Sprite2D_GetSpriteTexture2D, STATGROUP_Sprite2D);
UTexture2D* USpriteMergeSubsystem::GetSpriteTexture2D(USprite2D* InSprite, const FName& TargetAtlasName)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_GetSpriteTexture2D);
	
	if (!IsValid(InSprite))
		return nullptr;

	UTexture2D* SpriteSourceTexture = InSprite->SourceTexture;
	if (!IsValid(SpriteSourceTexture))
		return nullptr;
	
	const FVector2D& SourceDimension = InSprite->SourceDimension;
	if (SourceDimension.X > AtlasWidth || SourceDimension.Y > AtlasHeight)
	{
		return SpriteSourceTexture;
	}

	if (!SpriteSourceTexture->IsReadyForAsyncPostLoad())
	{
		InSprite->bWaitingTextureData = true;
		FSpritePendingAction PendingAction;
		PendingAction.Sprite = InSprite;
		PendingAction.Width = AtlasWidth;
		PendingAction.Height = AtlasHeight;
		SpritePendingActions.Emplace(PendingAction);
		return nullptr;
	}

	return InternalGetSpriteTexture2D(InSprite, TargetAtlasName);
}

DECLARE_CYCLE_STAT(TEXT("Sprite2D --- InternalGetSpriteTexture2D"), STATGROUP_Sprite2D_InternalGetSpriteTexture2D, STATGROUP_Sprite2D);
UTexture2D* USpriteMergeSubsystem::InternalGetSpriteTexture2D(USprite2D* InSprite, const FName& TargetAtlasName)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_Sprite2D_InternalGetSpriteTexture2D);
	
	UTexture2D* SpriteSourceTexture = InSprite->SourceTexture;
	const FVector2D& SourceDimension = InSprite->SourceDimension;
	
	const EPixelFormat PixelFormat = SpriteSourceTexture->GetPixelFormat();
	const TextureCompressionSettings CompressionSettings = SpriteSourceTexture->CompressionSettings;
	const bool bSRGB = SpriteSourceTexture->SRGB;
	
	uint32 X = 0;
	uint32 Y = 0;
	uint32 Width = 0;
	uint32 Height = 0;
	
	TSharedPtr<FSpriteAtlasTexture> TargetAtlasTexture = FindValidAtlasTexture(TargetAtlasName, PixelFormat, CompressionSettings, bSRGB, X, Y, Width, Height, SourceDimension);
	if (TargetAtlasTexture.IsValid())
	{
		InSprite->SpriteAtlasTexture = TargetAtlasTexture;
	}
	
	if (!TargetAtlasTexture)
	{
		const int32 BlockSizeX = GPixelFormats[PixelFormat].BlockSizeX;	  // Block width in pixels
		const int32 BlockSizeY = GPixelFormats[PixelFormat].BlockSizeY;	  // Block height in pixels

		UTexture2D* TargetTexture = nullptr;
		TargetAtlasTexture = FSpriteAtlasTexture::CreateSpriteAtlasTexture(TargetTexture, TargetAtlasName, AtlasWidth, AtlasHeight, PixelFormat, BlockSizeX, BlockSizeY, CompressionSettings, bSRGB);
		if (!TargetAtlasTexture.IsValid() && TargetTexture)
			return SpriteSourceTexture;

		Textures.Add(TargetTexture);
		SpriteTextures.Add(TargetAtlasTexture);
		
		const auto AtlasSlot = TargetAtlasTexture->FindSlotForTexture(SourceDimension.X, SourceDimension.Y);
		X = AtlasSlot->X;
		Y = AtlasSlot->Y;
		Width = AtlasSlot->Width;
		Height = AtlasSlot->Height;

		InSprite->SpriteAtlasTexture = TargetAtlasTexture;
	}

	TargetAtlasTexture->AddMergeAction(InSprite, X, Y, Width, Height);
	
	InSprite->AtlasSourceUV = FVector2D(X, Y);

	if (!bDirty)
	{
		bDirty = true;
	}
		
	return TargetAtlasTexture.IsValid() ? TargetAtlasTexture->Texture.Get() : nullptr;
}

/////////////////////////////////////////////////////
