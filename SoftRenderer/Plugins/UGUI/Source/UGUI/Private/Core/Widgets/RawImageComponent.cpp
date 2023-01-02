#include "Core/Widgets/RawImageComponent.h"
#include "Core/Render/VertexHelper.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// URawImageComponent

URawImageComponent::URawImageComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Texture = nullptr;
	UVRect = FUVRect(0, 0, 1, 1);

	bPreserveAspect = false;
	bFillViewRect = false;

	Thickness = 1;

	OverrideTextureSize = FVector2D(-1, -1);
	ImageMaskType = ERawImageMaskMode::MaskMode_None;
}

void URawImageComponent::PreserveTextureAspectRatio(FRect& ViewRect, const FVector2D& TextureSize) const
{
	if (TextureSize.Y <= 0 || ViewRect.Height <= 0
	|| ViewRect.Width <= 0 || ViewRect.Height <= 0)
		return;
	
	const float TextureRatio = TextureSize.X / TextureSize.Y;
	const float RectRatio = ViewRect.Width / ViewRect.Height;

	if (TextureRatio > RectRatio)
	{
		if (TextureRatio == 0)
			return;

		if (bFillViewRect)
		{
			const float OldWidth = ViewRect.Width;
			ViewRect.Width = ViewRect.Height * TextureRatio;
			ViewRect.XMin += (OldWidth - ViewRect.Width) * Pivot.X;
		}
		else
		{
			const float OldHeight = ViewRect.Height;
			ViewRect.Height = ViewRect.Width * (1.0f / TextureRatio);
			ViewRect.YMin += (OldHeight - ViewRect.Height) * Pivot.Y;
		}
	}
	else
	{
		if (TextureRatio == 0)
			return;
		
		if (bFillViewRect)
		{
			const float OldHeight = ViewRect.Height;
			ViewRect.Height = ViewRect.Width * (1.0f / TextureRatio);
			ViewRect.YMin += (OldHeight - ViewRect.Height) * Pivot.Y;
		}
		else
		{
			const float OldWidth = ViewRect.Width;
			ViewRect.Width = ViewRect.Height * TextureRatio;
			ViewRect.XMin += (OldWidth - ViewRect.Width) * Pivot.X;
		}
	}
}

FVector4 URawImageComponent::GetDrawingDimensions(bool bShouldPreserveAspect, FRect& OriginalViewRect)
{
	FVector2D Size = FVector2D::ZeroVector;
	if (OverrideTextureSize.X > 0 && OverrideTextureSize.Y > 0)
	{
		Size = OverrideTextureSize;
	}
	else if (IsValid(Texture))
	{
		Size = FVector2D(Texture->GetSurfaceWidth(), Texture->GetSurfaceHeight());
	}

	OriginalViewRect = GetPixelAdjustedRect();

	if (bShouldPreserveAspect && Size.SizeSquared() > 0.0f)
	{
		PreserveTextureAspectRatio(OriginalViewRect, Size);
	}

	return FVector4(
		OriginalViewRect.XMin,
		OriginalViewRect.YMin,
		OriginalViewRect.XMin + OriginalViewRect.Width,
		OriginalViewRect.YMin + OriginalViewRect.Height
	);
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- OnPopulateMesh"), STAT_UnrealGUI_RawImageOnPopulateMesh, STATGROUP_UnrealGUI);
void URawImageComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RawImageOnPopulateMesh);
	
    VertexHelper.Empty();
	
	FRect OriginalViewRect;
	const auto& FinalRect = GetDrawingDimensions(bPreserveAspect, OriginalViewRect);
	const float BottomLeftX = FinalRect.X;
	const float BottomLeftY = FinalRect.Y;
	const float TopRightX = FinalRect.Z;
	const float TopRightY = FinalRect.W;

	const FVector2D UV1 = GetUV1FromGraphicEffects();
		
	if (!bAntiAliasing)
	{
		VertexHelper.Reserve(4, 6);

		VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(UVRect.X, UVRect.Y + UVRect.H), UV1);
		VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y + UVRect.H), UV1);
		VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y), UV1);
		VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(UVRect.X, UVRect.Y), UV1);

		VertexHelper.AddTriangle(0, 2, 1);
		VertexHelper.AddTriangle(2, 0, 3);
	}
	else
	{
		if (ImageMaskType == ERawImageMaskMode::MaskMode_Circle)
		{
			VertexHelper.Reserve(4, 6);

			const FVector2D UV5 = FVector2D(1, 0);
			
			VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(UVRect.X, UVRect.Y + UVRect.H), UV1,
			FVector2D::ZeroVector, FVector2D(0, 1), UV5);
			VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y + UVRect.H), UV1,
			FVector2D::ZeroVector, FVector2D(1, 1), UV5);
			VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y), UV1,
			FVector2D::ZeroVector, FVector2D(1, 0), UV5);
			VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(UVRect.X, UVRect.Y), UV1,
			FVector2D::ZeroVector, FVector2D(0, 0), UV5);

			VertexHelper.AddTriangle(0, 2, 1);
			VertexHelper.AddTriangle(2, 0, 3);
		}
		else if (ImageMaskType == ERawImageMaskMode::MaskMode_CircleRing)
		{
			VertexHelper.Reserve(4, 6);

			const FVector2D UV5 = FVector2D(1, (1 - GetThickness()) * 0.5f);
			
			VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(UVRect.X, UVRect.Y + UVRect.H), UV1,
			FVector2D::ZeroVector, FVector2D(0, 1), UV5);
			VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y + UVRect.H), UV1,
			FVector2D::ZeroVector, FVector2D(1, 1), UV5);
			VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y), UV1,
			FVector2D::ZeroVector, FVector2D(1, 0), UV5);
			VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(UVRect.X, UVRect.Y), UV1,
			FVector2D::ZeroVector, FVector2D(0, 0), UV5);

			VertexHelper.AddTriangle(0, 2, 1);
			VertexHelper.AddTriangle(2, 0, 3);
		}
		else
		{
			VertexHelper.Reserve(4, 6);

			VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(UVRect.X, UVRect.Y + UVRect.H), UV1,
			FVector2D(-1, -1));
			VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y + UVRect.H), UV1,
			FVector2D(-1, 1));
			VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(UVRect.X + UVRect.W, UVRect.Y), UV1,
			FVector2D(1, 1));
			VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(UVRect.X, UVRect.Y), UV1,
			FVector2D(1, -1));

			VertexHelper.AddTriangle(0, 2, 1);
			VertexHelper.AddTriangle(2, 0, 3);
		}
	}
}

FVector2D URawImageComponent::GetNativeSize() 
{
	if (IsValid(Texture))
	{
		const FVector2D TextureSize = FVector2D(Texture->GetSurfaceWidth() * UVRect.W, Texture->GetSurfaceHeight() * UVRect.H);
		return TextureSize;
	}
   
    return FVector2D::ZeroVector;
}

float URawImageComponent::GetPreferredWidth()
{
	if (!IsValid(Texture))
		return 0;
	
	return Texture->GetSurfaceWidth();
}

float URawImageComponent::GetPreferredHeight()
{
	if (!IsValid(Texture))
		return 0;

	return Texture->GetSurfaceHeight();
}

/////////////////////////////////////////////////////
