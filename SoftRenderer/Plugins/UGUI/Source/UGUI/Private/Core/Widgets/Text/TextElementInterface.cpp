#include "Core/Widgets/Text/TextElementInterface.h"

/////////////////////////////////////////////////////
// FEmojiRuntimeInfo

UTexture* FEmojiRuntimeInfo::GetEmojiTexture(FTextEmoji* TextEmoji) const
{
	const int32 SpriteIndex = FMath::Min(AnimationIndex, TextEmoji->Sprites.Num() - 1);
	if (TextEmoji->Sprites.IsValidIndex(SpriteIndex))
	{
		const auto SpriteObj = TextEmoji->Sprites[SpriteIndex];
		if (const UPaperSprite* Sprite = Cast<UPaperSprite>(SpriteObj))
		{
			const auto& SlateAtlasData = Sprite->GetSlateAtlasData();
			return SlateAtlasData.AtlasTexture;
		}
		else if (USprite2D* Sprite2D = Cast<USprite2D>(SpriteObj))
		{
			return Sprite2D->GetSpriteTexture();
		}
	}

	return nullptr;
}

void FEmojiRuntimeInfo::UpdateEmojiRenderInfo(FTextEmoji* TextEmoji, const FLinearColor& TextColor)
{
	if (!EmojiImageComponent.IsValid())
	{
		return;
	}
	
	FVector2D BottomLeftUV = FVector2D::ZeroVector;
	FVector2D TopRightUV = FVector2D::ZeroVector;

	USprite2D* Sprite2D = nullptr;
	
	const int32 SpriteIndex = FMath::Min(AnimationIndex, TextEmoji->Sprites.Num() - 1);
	if (TextEmoji->Sprites.IsValidIndex(SpriteIndex))
	{
		const auto SpriteObj = TextEmoji->Sprites[SpriteIndex];
		if (const UPaperSprite* Sprite = Cast<UPaperSprite>(SpriteObj))
		{
			const auto& SlateAtlasData = Sprite->GetSlateAtlasData();
			const FVector2D BottomRightUV = SlateAtlasData.StartUV + SlateAtlasData.SizeUV;
			BottomLeftUV = FVector2D(SlateAtlasData.StartUV.X, BottomRightUV.Y);
			TopRightUV = FVector2D(BottomRightUV.X, SlateAtlasData.StartUV.Y);
		}
		else if (USprite2D* Sprite2DObj = Cast<USprite2D>(SpriteObj))
		{
			Sprite2D = Sprite2DObj;
			const FVector4 OuterUV = Sprite2D->GetOuterUV();
			BottomLeftUV = FVector2D(OuterUV.X, OuterUV.W);
			TopRightUV = FVector2D(OuterUV.Z, OuterUV.Y);
		}
	}

	FEmojiRenderInfo* EmojiRenderInfoPtr = EmojiImageComponent->FindEmojiRenderInfo(RenderInfoKey, nullptr);
	if (!EmojiRenderInfoPtr)
	{
		FEmojiRenderInfo EmojiRenderInfo;
		EmojiRenderInfo.BottomLeft = EmojiRegion.BottomLeft;
		EmojiRenderInfo.TopRight = EmojiRegion.TopRight;
		EmojiRenderInfo.BottomLeftUV = BottomLeftUV;
		EmojiRenderInfo.TopRightUV = TopRightUV;
		EmojiRenderInfo.EmojiColor = TextEmoji->Color;
		RenderInfoKey = EmojiImageComponent->AddEmojiRenderInfo(MoveTemp(EmojiRenderInfo), Sprite2D);
	}
	else
	{
		EmojiRenderInfoPtr->BottomLeftUV = BottomLeftUV;
		EmojiRenderInfoPtr->TopRightUV = TopRightUV;
		EmojiRenderInfoPtr->EmojiColor = TextEmoji->Color;
	}

	EmojiImageComponent->SetColor(TextColor);
	EmojiImageComponent->SetVerticesDirty();
}

void FEmojiRuntimeInfo::RemoveEmojiRenderInfo()
{
	if (!EmojiImageComponent.IsValid())
	{
		return;
	}

	EmojiImageComponent->RemoveEmojiRenderInfo(RenderInfoKey);
	EmojiImageComponent->SetVerticesDirty();
	EmojiImageComponent = nullptr;
}

/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// UTextElementInterface

TArray<FUIVertex> ITextElementInterface::TempUIVertices;

UTextElementInterface::UTextElementInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

/////////////////////////////////////////////////////
