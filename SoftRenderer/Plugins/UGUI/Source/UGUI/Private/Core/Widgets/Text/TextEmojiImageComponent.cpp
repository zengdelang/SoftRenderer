#include "Core/Widgets/Text/TextEmojiImageComponent.h"

/////////////////////////////////////////////////////
// UTextEmojiImageComponent

UTextEmojiImageComponent::UTextEmojiImageComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Texture = nullptr;
	bRaycastTarget = false;
	RenderKeyCounter = 0;
}

void UTextEmojiImageComponent::OnEnable()
{
	Super::OnEnable();

	for (const auto& SpriteElem : Sprites)
	{
		if (IsValid(SpriteElem.Value))
		{
			SpriteElem.Value->AddSpriteListener(this);
			SpriteElem.Value->IncreaseReferenceCount();
		}
	}
}

void UTextEmojiImageComponent::OnDisable()
{
	for (const auto& SpriteElem : Sprites)
	{
		if (IsValid(SpriteElem.Value))
		{
			SpriteElem.Value->RemoveSpriteListener(this);
			SpriteElem.Value->DecreaseReferenceCount();
		}
	}
	
	Super::OnDisable();
}

void UTextEmojiImageComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	VertexHelper.Reset();

	const int32 EmojiCount = EmojiRenderInfos.Num();
	VertexHelper.Reserve(4 * EmojiCount, 6 * EmojiCount);
	const FVector2D UV1 = GetUV1FromGraphicEffects();

	int32 Index = 0;
	for (const auto& Elem : EmojiRenderInfos)
	{
		const auto& EmojiRender = Elem.Value;
		const FLinearColor FinalColor = Color * EmojiRender.EmojiColor;
		VertexHelper.AddVert(FVector(EmojiRender.BottomLeft.X, EmojiRender.BottomLeft.Y, 0), FinalColor, FVector2D(EmojiRender.BottomLeftUV.X, EmojiRender.BottomLeftUV.Y), UV1);
		VertexHelper.AddVert(FVector(EmojiRender.TopRight.X, EmojiRender.BottomLeft.Y, 0), FinalColor, FVector2D(EmojiRender.TopRightUV.X, EmojiRender.BottomLeftUV.Y), UV1);
		VertexHelper.AddVert(FVector(EmojiRender.TopRight.X, EmojiRender.TopRight.Y, 0), FinalColor, FVector2D(EmojiRender.TopRightUV.X, EmojiRender.TopRightUV.Y), UV1);
		VertexHelper.AddVert(FVector(EmojiRender.BottomLeft.X, EmojiRender.TopRight.Y, 0), FinalColor, FVector2D(EmojiRender.BottomLeftUV.X, EmojiRender.TopRightUV.Y), UV1);

		const int32 IndexDelta = 4 * Index;
		VertexHelper.AddTriangle(0 + IndexDelta, 1 + IndexDelta, 2 + IndexDelta);
		VertexHelper.AddTriangle(2 + IndexDelta, 3 + IndexDelta, 0 + IndexDelta);

		++Index;
	}
}

/////////////////////////////////////////////////////
