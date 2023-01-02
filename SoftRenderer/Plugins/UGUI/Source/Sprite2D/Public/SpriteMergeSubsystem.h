#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasTexture.h"
#include "Subsystems/EngineSubsystem.h"
#include "SpriteMergeSubsystem.generated.h"

class USprite2D;

struct FSpritePendingAction
{
	TWeakObjectPtr<USprite2D> Sprite;
	uint32 Width;
	uint32 Height;
};

DECLARE_MULTICAST_DELEGATE(FOnSpriteMergeTick)

UCLASS(BlueprintType, Blueprintable)
class SPRITE2D_API USpriteMergeSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	USpriteMergeSubsystem();
	
protected:
	UPROPERTY(Transient)
	TArray<UTexture2D*> Textures;
	
	TArray<TSharedPtr<FSpriteAtlasTexture>> SpriteTextures;
	
	TArray<FSpritePendingAction> SpritePendingActions;

	double TimeLimit;

	uint8 bDirty : 1;
	uint8 bDiscardUnreferencedTexture : 1;
	
public:
	static int32 AtlasWidth;
	static int32 AtlasHeight;
	static uint32 LoadingScreenCounter;

public:
	static FOnSpriteMergeTick OnSpriteMergeTick;
	static FOnSpriteMergeTick OnSpriteMergeTryAgainTick;
	
public:
	UFUNCTION(BlueprintCallable, Category=SpriteMergeSubsystem)
	const TArray<UTexture2D*>& GetSpriteTextures() const
	{
		return Textures;
	}

	FORCEINLINE void RemoveSpriteTexture(TSharedPtr<FSpriteAtlasTexture> SpriteTexture)
	{
		const int32 SpriteIndex = SpriteTextures.Find(SpriteTexture);
		if (SpriteIndex != INDEX_NONE)
		{
			const auto Texture = Textures[SpriteIndex];
			if (IsValid(Texture))
			{
				Texture->ReleaseResource();
			}
			
			Textures.RemoveAt(SpriteIndex);
			SpriteTextures.RemoveAt(SpriteIndex);
		}
	}

	FORCEINLINE bool IsDiscardUnreferencedTexture() const
	{
		return bDiscardUnreferencedTexture;
	}

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void ClearSprite2DAtlas();

protected:
	void PreLoadMap(const FString& MapName);
	void PostLoadMapWithWorld(UWorld* LoadedWorld);

	void TickInternal();
	void Tick();
	void TryAgainTick();

	TSharedPtr<FSpriteAtlasTexture> FindValidAtlasTexture(const FName& TargetAtlasName, const EPixelFormat PixelFormat, const TextureCompressionSettings CompressionSettings,
		const bool bSRGB, uint32& X, uint32& Y, uint32& Width, uint32& Height, const FVector2D& SourceDimension);

public:
	UTexture2D* GetSpriteTexture2D(USprite2D* InSprite, const FName& TargetAtlasName);

protected:
	UTexture2D* InternalGetSpriteTexture2D(USprite2D* InSprite, const FName& TargetAtlasName);
	
};
