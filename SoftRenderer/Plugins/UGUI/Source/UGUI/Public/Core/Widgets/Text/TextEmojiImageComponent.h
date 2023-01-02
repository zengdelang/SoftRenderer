#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/ImageComponent.h"
#include "TextEmojiImageComponent.generated.h"

struct FEmojiRenderInfo
{
public:
	FVector BottomLeft;
	FVector TopRight;

	FVector2D BottomLeftUV;
	FVector2D TopRightUV;

	FLinearColor EmojiColor;

public:
	FEmojiRenderInfo()
	{
		BottomLeft = FVector::ZeroVector;
		TopRight = FVector::ZeroVector;
		BottomLeftUV = FVector2D::ZeroVector;
		TopRightUV = FVector2D::ZeroVector;
		EmojiColor = FLinearColor::White;
	}
};

UCLASS(Blueprintable, BlueprintType)
class UGUI_API UTextEmojiImageComponent : public UMaskableGraphicComponent, public ISpriteTextureAtlasListenerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	TMap<int32, FEmojiRenderInfo> EmojiRenderInfos; 
	 
	TWeakObjectPtr<UTexture> Texture;

	int32 RenderKeyCounter;

	UPROPERTY(Transient)
	TMap<int32, USprite2D*> Sprites;

public:
	//~ Begin BehaviourComponent Interface
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourComponent Interface.

public:
	//~ Begin ISpriteTextureAtlasListenerInterface Interface
	virtual void NotifySpriteTextureChanged(bool bTextureChanged, bool bUVChanged) override
	{
		if (bUVChanged)
		{
			SetVerticesDirty();
		}

		if (bTextureChanged)
		{
			SetMaterialDirty();
		}
	}
	//~ End ISpriteTextureAtlasListenerInterface Interface.
	
public:
	virtual UTexture* GetMainTexture() const override
	{
		return Texture.Get();
	}

	void SetMainTexture(UTexture* InTexture)
	{
		if (Texture != InTexture)
		{
			Texture = InTexture;
			SetMaterialDirty();
		}
	}

public:
	void ClearEmojiRenderInfos()
	{
		for (const auto& SpriteElem : Sprites)
		{
			if (IsValid(SpriteElem.Value))
			{
				SpriteElem.Value->RemoveSpriteListener(this);
				SpriteElem.Value->DecreaseReferenceCount();
			}
		}
		Sprites.Empty();
		
		EmojiRenderInfos.Empty();
	}

	int32 AddEmojiRenderInfo(FEmojiRenderInfo&& EmojiRenderInfo, USprite2D* InSprite = nullptr)
	{
		++RenderKeyCounter;
		if (RenderKeyCounter == 0)
		{
			++RenderKeyCounter;
		}

		const auto SpritePtr = Sprites.Find(RenderKeyCounter);
		if (SpritePtr)
		{
			if (*SpritePtr == InSprite)
			{
				InSprite = nullptr;
			}
			else
			{
				(*SpritePtr)->RemoveSpriteListener(this);
				(*SpritePtr)->DecreaseReferenceCount();
				Sprites.Remove(RenderKeyCounter);
			}
		}

		if (IsValid(InSprite))
		{
			InSprite->AddSpriteListener(this);
			InSprite->IncreaseReferenceCount();
			Sprites.Add(RenderKeyCounter, InSprite);
		}
		
		EmojiRenderInfos.Emplace(RenderKeyCounter, EmojiRenderInfo);
		return RenderKeyCounter;
	}

	int32 AddEmojiRenderInfo(const FEmojiRenderInfo& EmojiRenderInfo, USprite2D* InSprite = nullptr)
	{
		++RenderKeyCounter;
		if (RenderKeyCounter == 0)
		{
			++RenderKeyCounter;
		}

		const auto SpritePtr = Sprites.Find(RenderKeyCounter);
		if (SpritePtr)
		{
			if (*SpritePtr == InSprite)
			{
				InSprite = nullptr;
			}
			else
			{
				(*SpritePtr)->RemoveSpriteListener(this);
				(*SpritePtr)->DecreaseReferenceCount();
				Sprites.Remove(RenderKeyCounter);
			}
		}

		if (IsValid(InSprite))
		{
			InSprite->AddSpriteListener(this);
			InSprite->IncreaseReferenceCount();
			Sprites.Add(RenderKeyCounter, InSprite);
		}
		
		EmojiRenderInfos.Add(RenderKeyCounter, EmojiRenderInfo);
		return RenderKeyCounter;
	}

	void RemoveEmojiRenderInfo(int32 Key)
	{
		const auto SpritePtr = Sprites.Find(Key);
		if (SpritePtr)
		{
			(*SpritePtr)->RemoveSpriteListener(this);
			(*SpritePtr)->DecreaseReferenceCount();
			Sprites.Remove(Key);
		}
		
		EmojiRenderInfos.Remove(Key);
	}

	FEmojiRenderInfo* FindEmojiRenderInfo(int32 Key, USprite2D* InSprite)
	{
		const auto SpritePtr = Sprites.Find(Key);
		if (SpritePtr)
		{
			if (*SpritePtr == InSprite)
			{
				InSprite = nullptr;
			}
			else
			{
				(*SpritePtr)->RemoveSpriteListener(this);
				(*SpritePtr)->DecreaseReferenceCount();
				Sprites.Remove(Key);
			}
		}

		if (IsValid(InSprite))
		{
			InSprite->AddSpriteListener(this);
			InSprite->IncreaseReferenceCount();
			Sprites.Add(Key, InSprite);
		}
		
		return EmojiRenderInfos.Find(Key);
	}

protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

};
