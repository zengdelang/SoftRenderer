#include "Core/Widgets/ImageComponent.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Render/VertexHelper.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// USlateImageComponent

UImageComponent::UImageComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Sprite = nullptr;
	OverrideSprite = nullptr;

	ImageType = EImageFillType::ImageFillType_Simple;
	ImageMaskType = EImageMaskMode::MaskMode_None;
	FillMethod = EFillMethod::FillMethod_Radial360;

	FillOrigin = 0;
	FillAmount = 1;

	Thickness = 1;
	
	PixelsPerUnitMultiplier = 1;
	CachedReferencePixelsPerUnit = 1;

#if WITH_EDITORONLY_DATA
	OriginHorizontal = EOriginHorizontal::OriginHorizontal_Left;
	OriginVertical = EOriginVertical::OriginVertical_Bottom;
	Origin90 = EOrigin90::Origin90_BottomLeft;
	Origin180 = EOrigin180::Origin180_Bottom;
	Origin360 = EOrigin360::Origin360_Bottom;
#endif

	bPreserveAspect = false;
	bFillViewRect = false;

	bFillCenter = true;
	bFillClockwise = true;
}

float UImageComponent::GetPixelsPerUnit()
{
	float SpritePixelsPerUnit = 1;
	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		SpritePixelsPerUnit = PaperSprite->GetPixelsPerUnrealUnit();
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		SpritePixelsPerUnit = Sprite2D->GetPixelsPerUnrealUnit();
	}
	
	const auto TargetCanvas = GetCanvas();
	if (TargetCanvas)
	{
		CachedReferencePixelsPerUnit = TargetCanvas->GetReferencePixelsPerUnit();
	}

	return SpritePixelsPerUnit / CachedReferencePixelsPerUnit;
}

#if WITH_EDITORONLY_DATA
void UImageComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, FillMethod))
	{
		FillOrigin = 0;
		OriginHorizontal = EOriginHorizontal::OriginHorizontal_Left;
		OriginVertical = EOriginVertical::OriginVertical_Bottom;
		Origin90 = EOrigin90::Origin90_BottomLeft;
		Origin180 = EOrigin180::Origin180_Bottom;
		Origin360 = EOrigin360::Origin360_Bottom;
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, OriginHorizontal) && FillMethod == EFillMethod::FillMethod_Horizontal)
	{
		FillOrigin = static_cast<int32>(OriginHorizontal);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, OriginVertical) && FillMethod == EFillMethod::FillMethod_Vertical)
	{
		FillOrigin = static_cast<int32>(OriginVertical);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, Origin90) && FillMethod == EFillMethod::FillMethod_Radial90)
	{
		FillOrigin = static_cast<int32>(Origin90);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, Origin180) && FillMethod == EFillMethod::FillMethod_Radial180)
	{
		FillOrigin = static_cast<int32>(Origin180);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UImageComponent, Origin360) && FillMethod == EFillMethod::FillMethod_Radial360)
	{
		FillOrigin = static_cast<int32>(Origin360);
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UImageComponent::OnEnable()
{
	Super::OnEnable();

	if (const auto Sprite2D = Cast<USprite2D>(Sprite))
	{
		Sprite2D->AddSpriteListener(this);
		Sprite2D->IncreaseReferenceCount();
	}

	if (const auto Sprite2D = Cast<USprite2D>(OverrideSprite))
	{
		Sprite2D->AddSpriteListener(this);
		Sprite2D->IncreaseReferenceCount();
	}
}

void UImageComponent::OnDisable()
{
	if (const auto Sprite2D = Cast<USprite2D>(Sprite))
	{
		Sprite2D->RemoveSpriteListener(this);
		Sprite2D->DecreaseReferenceCount();
	}

	if (const auto Sprite2D = Cast<USprite2D>(OverrideSprite))
	{
		Sprite2D->RemoveSpriteListener(this);
		Sprite2D->DecreaseReferenceCount();
	}
	
	Super::OnDisable();
}

float UImageComponent::GetPreferredWidth()
{
	const auto ActiveSprite = GetActiveSprite();
	if (!IsValid(ActiveSprite))
		return 0;
	
	if (ImageType == EImageFillType::ImageFillType_Sliced || ImageType == EImageFillType::ImageFillType_Tiled)
	{
		FVector4 SpriteBorder;
		if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
		{
			const auto& AtlasData = PaperSprite->GetSlateAtlasData();
			SpriteBorder = GetBorderSize(AtlasData);
		}
		else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
		{
		    SpriteBorder = GetBorderSize(Sprite2D);
		}

		return (SpriteBorder.X + SpriteBorder.Z) / GetPixelsPerUnit();
	}
	
	FVector2D SpriteSize = FVector2D::ZeroVector;
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& AtlasData = PaperSprite->GetSlateAtlasData();
		SpriteSize = GetSpriteSize(AtlasData);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		SpriteSize = Sprite2D->GetSpriteSize();
	}
	
	return SpriteSize.X / GetPixelsPerUnit();
}

float UImageComponent::GetPreferredHeight()
{
	const auto ActiveSprite = GetActiveSprite();
	if (!IsValid(ActiveSprite))
		return 0;
	
	if (ImageType == EImageFillType::ImageFillType_Sliced || ImageType == EImageFillType::ImageFillType_Tiled)
	{
		FVector4 SpriteBorder;
		if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
		{
			const auto& AtlasData = PaperSprite->GetSlateAtlasData();
			SpriteBorder = GetBorderSize(AtlasData);
		}
		else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
		{
			SpriteBorder = GetBorderSize(Sprite2D);
		}
		
		return (SpriteBorder.Y + SpriteBorder.W) / GetPixelsPerUnit();
	}
	
	FVector2D SpriteSize = FVector2D::ZeroVector;
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& AtlasData = PaperSprite->GetSlateAtlasData();
		SpriteSize = GetSpriteSize(AtlasData);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		SpriteSize = Sprite2D->GetSpriteSize();
	}
	
	return SpriteSize.Y / GetPixelsPerUnit();
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- IsRaycastLocationValid"), STAT_UnrealGUI_ImageRaycast, STATGROUP_UnrealGUI);
bool UImageComponent::IsRaycastLocationValid(IMaskableGraphicElementInterface* MaskableGraphicElement, const FVector& WorldRayOrigin, const FVector& WorldRayDir,
	bool bIgnoreReversedGraphicsScreenPoint)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_ImageRaycast);
	
	const auto ActiveSprite = GetActiveSprite();
	if (!IsValid(ActiveSprite))
		return true;

	FVector2D LocalPosition = FVector2D::ZeroVector;
	if (!FRectTransformUtility::ScreenPointToLocalPointInRectangle(this, WorldRayOrigin, WorldRayDir, LocalPosition))
		return false;
	
	return true;
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- OnPopulateMesh"), STAT_UnrealGUI_ImageOnPopulateMesh, STATGROUP_UnrealGUI);
void UImageComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_ImageOnPopulateMesh);
	
	if (!IsValid(GetActiveSprite()))
	{
		if (bAntiAliasing)
		{
			GenerateSimpleSpriteAA(VertexHelper, bPreserveAspect);
		}
		else
		{
			GenerateSimpleSprite(VertexHelper, bPreserveAspect);
		}
		return;
	}

	switch (ImageType)
	{
	case EImageFillType::ImageFillType_Simple:
		if (bAntiAliasing)
		{
			GenerateSimpleSpriteAA(VertexHelper, bPreserveAspect);
		}
		else
		{
			GenerateSimpleSprite(VertexHelper, bPreserveAspect);
		}
		break;
	case EImageFillType::ImageFillType_Sliced:
		GenerateSlicedSprite(VertexHelper);
		break;
	case EImageFillType::ImageFillType_Tiled:
		GenerateTiledSprite(VertexHelper);
		break;
	case EImageFillType::ImageFillType_Filled:
		GenerateFilledSprite(VertexHelper, bPreserveAspect);
		break;
	default:;
	}
}

FVector2D UImageComponent::GetNativeSize()
{
	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& AtlasData = PaperSprite->GetSlateAtlasData();
		const auto& SpriteSize = GetSpriteSize(AtlasData);
		return SpriteSize / GetPixelsPerUnit();
	}

	if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		return Sprite2D->GetSpriteSize() / GetPixelsPerUnit();
	}
	return FVector2D::ZeroVector;
}

void UImageComponent::PreserveSpriteAspectRatio(FRect& ViewRect, const FVector2D& SpriteSize) const
{
	if (SpriteSize.Y <= 0 || ViewRect.Height <= 0
		|| ViewRect.Width <= 0 || ViewRect.Height <= 0)
		return;
	
	const float SpriteRatio = SpriteSize.X / SpriteSize.Y;
	const float RectRatio = ViewRect.Width / ViewRect.Height;

	if (SpriteRatio > RectRatio)
	{
		if (SpriteRatio == 0)
			return;

		if (bFillViewRect)
		{
			const float OldWidth = ViewRect.Width;
			ViewRect.Width = ViewRect.Height * SpriteRatio;
			ViewRect.XMin += (OldWidth - ViewRect.Width) * Pivot.X;
		}
		else
		{
			const float OldHeight = ViewRect.Height;
			ViewRect.Height = ViewRect.Width * (1.0f / SpriteRatio);
			ViewRect.YMin += (OldHeight - ViewRect.Height) * Pivot.Y;
		}
	}
	else
	{
		if (SpriteRatio == 0)
			return;
		
		if (bFillViewRect)
		{
			const float OldHeight = ViewRect.Height;
			ViewRect.Height = ViewRect.Width * (1.0f / SpriteRatio);
			ViewRect.YMin += (OldHeight - ViewRect.Height) * Pivot.Y;
		}
		else
		{
			const float OldWidth = ViewRect.Width;
			ViewRect.Width = ViewRect.Height * SpriteRatio;
			ViewRect.XMin += (OldWidth - ViewRect.Width) * Pivot.X;
		}
	}
}

FVector4 UImageComponent::GetDrawingDimensions(bool bShouldPreserveAspect, FRect& OriginalViewRect)
{
	const auto ActiveSprite = GetActiveSprite();
	FVector2D Size = FVector2D::ZeroVector;
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		Size = GetSpriteSize(PaperSprite->GetSlateAtlasData());
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		Size = Sprite2D->GetSpriteSize();
	}
	
	OriginalViewRect = GetPixelAdjustedRect();

	if (bShouldPreserveAspect && Size.SizeSquared() > 0.0f)
	{
		PreserveSpriteAspectRatio(OriginalViewRect, Size);
	}

	return FVector4(
		OriginalViewRect.XMin,
		OriginalViewRect.YMin,
		OriginalViewRect.XMin + OriginalViewRect.Width,
		OriginalViewRect.YMin + OriginalViewRect.Height
	);
}

void UImageComponent::GenerateSimpleSprite(FVertexHelper& VertexHelper, bool bInPreserveAspect)
{
	FRect OriginalViewRect;
	const FVector4 ViewRect = GetDrawingDimensions(bInPreserveAspect, OriginalViewRect);

	FVector4 OuterUV = FVector4(0, 0, 1, 1);
	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(SlateAtlasData.StartUV, SlateAtlasData.StartUV + SlateAtlasData.SizeUV);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
	}
	
	const FVector2D UV1 = GetUV1FromGraphicEffects();
	
	VertexHelper.Empty();

	VertexHelper.Reserve(4, 6);

	VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.Y, 0), Color, FVector2D(OuterUV.X, OuterUV.W), UV1);
	VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.W, 0), Color, FVector2D(OuterUV.X, OuterUV.Y), UV1);
	VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.W, 0), Color, FVector2D(OuterUV.Z, OuterUV.Y), UV1);
	VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.Y, 0), Color, FVector2D(OuterUV.Z, OuterUV.W), UV1);
	
	VertexHelper.AddTriangle(0, 1, 2);
	VertexHelper.AddTriangle(2, 3, 0);
}

void UImageComponent::GenerateSimpleSpriteAA(FVertexHelper& VertexHelper, bool bInPreserveAspect)
{
	FRect OriginalViewRect;
	const FVector4 ViewRect = GetDrawingDimensions(bInPreserveAspect, OriginalViewRect);

	FVector4 OuterUV = FVector4(0, 0, 1, 1);
	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(SlateAtlasData.StartUV, SlateAtlasData.StartUV + SlateAtlasData.SizeUV);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
	}

	const FVector2D UV1 = GetUV1FromGraphicEffects();
	
	VertexHelper.Empty();
	if (ImageMaskType == EImageMaskMode::MaskMode_Circle)
	{
		VertexHelper.Reserve(4, 6);

		const FVector2D UV5 = FVector2D(1, 0);
		
		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.Y, 0), Color, FVector2D(OuterUV.X, OuterUV.W), UV1,
			FVector2D::ZeroVector, FVector2D(0, 1), UV5);
		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.W, 0), Color, FVector2D(OuterUV.X, OuterUV.Y), UV1,
			FVector2D::ZeroVector, FVector2D(1, 1), UV5);
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.W, 0), Color, FVector2D(OuterUV.Z, OuterUV.Y), UV1,
			FVector2D::ZeroVector, FVector2D(1, 0), UV5);
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.Y, 0), Color, FVector2D(OuterUV.Z, OuterUV.W), UV1,
			FVector2D::ZeroVector, FVector2D(0, 0), UV5);
	
		VertexHelper.AddTriangle(0, 1, 2);
		VertexHelper.AddTriangle(2, 3, 0);
	}
	else if (ImageMaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		VertexHelper.Reserve(4, 6);

		const FVector2D UV5 = FVector2D(1, (1 - GetThickness()) * 0.5f);
                
		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.Y, 0), Color, FVector2D(OuterUV.X, OuterUV.W), UV1,
				FVector2D::ZeroVector, FVector2D(0, 1), UV5);
		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.W, 0), Color, FVector2D(OuterUV.X, OuterUV.Y), UV1,
				FVector2D::ZeroVector, FVector2D(1, 1), UV5);
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.W, 0), Color, FVector2D(OuterUV.Z, OuterUV.Y), UV1,
				FVector2D::ZeroVector, FVector2D(1, 0), UV5);
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.Y, 0), Color, FVector2D(OuterUV.Z, OuterUV.W), UV1,
				FVector2D::ZeroVector, FVector2D(0, 0), UV5);

		VertexHelper.AddTriangle(0, 1, 2);
		VertexHelper.AddTriangle(2, 3, 0);
	}
	else
	{
		VertexHelper.Reserve(4, 6);

		const FVector2D UV5 = FVector2D(0, 1);

		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.Y, 0), Color, FVector2D(OuterUV.X, OuterUV.W), UV1,
		                     FVector2D(-1, -1));
		VertexHelper.AddVert(FVector(ViewRect.X, ViewRect.W, 0), Color, FVector2D(OuterUV.X, OuterUV.Y), UV1,
		                     FVector2D(-1, 1));
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.W, 0), Color, FVector2D(OuterUV.Z, OuterUV.Y), UV1,
		                     FVector2D(1, 1));
		VertexHelper.AddVert(FVector(ViewRect.Z, ViewRect.Y, 0), Color, FVector2D(OuterUV.Z, OuterUV.W), UV1,
		                     FVector2D(1, -1));

		VertexHelper.AddTriangle(0, 1, 2);
		VertexHelper.AddTriangle(2, 3, 0);
	}
}

void UImageComponent::GenerateSlicedSprite(FVertexHelper& VertexHelper)
{
	if (!HasBorder())
	{
		if (!bAntiAliasing)
		{
			GenerateSimpleSprite(VertexHelper, false);
		}
		else
		{
			GenerateSimpleSpriteAA(VertexHelper, false);
		}
		return;
	}

	FVector4 OuterUV = FVector4(0,0,0,0);
	FVector4 InnerUV = FVector4(0, 0, 0, 0);
	FVector4 SpriteBorder = FVector4(0, 0, 0, 0);

	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& AtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(AtlasData.StartUV, AtlasData.StartUV + AtlasData.SizeUV);
		InnerUV = GetInnerUV(OuterUV);
		SpriteBorder = GetBorderSize(AtlasData);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
		InnerUV = GetInnerUV(OuterUV);
		SpriteBorder = GetBorderSize(Sprite2D);
	}

	const FRect ViewRect = GetPixelAdjustedRect();

	const FVector4 AdjustedBorders = GetAdjustedBorders(SpriteBorder / MultipliedPixelsPerUnit(), ViewRect);

	FVector2D VertScratch[4] =
	{
		FVector2D(0, 0),
		FVector2D(AdjustedBorders.X, AdjustedBorders.Y),
		FVector2D(ViewRect.Width - AdjustedBorders.Z, ViewRect.Height - AdjustedBorders.W),
		FVector2D(ViewRect.Width, ViewRect.Height)
	};

	for (int32 Index = 0; Index < 4; ++Index)
	{
		VertScratch[Index].X += ViewRect.XMin;
		VertScratch[Index].Y += ViewRect.YMin;
	}

	const FVector2D UVScratch[4] = 
	{
		FVector2D(OuterUV.X, OuterUV.W),
		FVector2D(InnerUV.X, InnerUV.W),
		FVector2D(InnerUV.Z, InnerUV.Y),
		FVector2D(OuterUV.Z, OuterUV.Y)
	};

	const FVector2D UV2Scratch[4] = 
	{
		FVector2D(-1, -1),
		FVector2D(-1 + AdjustedBorders.X / ViewRect.Width, -1 + AdjustedBorders.Y / ViewRect.Height),
		FVector2D(1 - AdjustedBorders.X / ViewRect.Width, 1 - AdjustedBorders.Y / ViewRect.Height),
		FVector2D(1, 1)
	};

	const FVector2D UV1 = GetUV1FromGraphicEffects();
 
	FVector2D UV5 = FVector2D::ZeroVector;
	if (ImageMaskType == EImageMaskMode::MaskMode_Circle)
	{
		UV5 = FVector2D(1,0);
	}
	else if (ImageMaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		UV5 = FVector2D(1,(1 - GetThickness()) * 0.5f);
	}
	
	VertexHelper.Empty();

	for (int32 X = 0; X < 3; ++X)
	{
		const int32 X2 = X + 1;

		for (int32 Y = 0; Y < 3; ++Y)
		{
			if (!bFillCenter && X == 1 && Y == 1)
				continue;

			const int32 Y2 = Y + 1;

			if (!bFillCenter && Y == 1 && X != 1)
			{
				AddQuad(VertexHelper,
				FVector2D(VertScratch[X].X, VertScratch[Y].Y),
				FVector2D(VertScratch[X2].X, VertScratch[Y2].Y),
				Color,
				FVector2D(UVScratch[X].X, UVScratch[Y].Y),
				FVector2D(UVScratch[X2].X, UVScratch[Y2].Y),
				UV1,
				FVector2D(-1, UV2Scratch[Y].Y),
				FVector2D(1, UV2Scratch[Y2].Y),
				ImageMaskType, UV5, ViewRect);
			}
			else if (!bFillCenter && Y != 1 && X == 1)
			{
				AddQuad(VertexHelper,
					FVector2D(VertScratch[X].X, VertScratch[Y].Y),
					FVector2D(VertScratch[X2].X, VertScratch[Y2].Y),
					Color,
					FVector2D(UVScratch[X].X, UVScratch[Y].Y),
					FVector2D(UVScratch[X2].X, UVScratch[Y2].Y),
					UV1,
					FVector2D(UV2Scratch[X].X, -1),
					FVector2D(UV2Scratch[X2].X, 1),
					ImageMaskType, UV5, ViewRect);
			}
			else
			{
				AddQuad(VertexHelper,
					FVector2D(VertScratch[X].X, VertScratch[Y].Y),
					FVector2D(VertScratch[X2].X, VertScratch[Y2].Y),
					Color,
					FVector2D(UVScratch[X].X, UVScratch[Y].Y),
					FVector2D(UVScratch[X2].X, UVScratch[Y2].Y),
					UV1,
					FVector2D(UV2Scratch[X].X, UV2Scratch[Y].Y),
					FVector2D(UV2Scratch[X2].X, UV2Scratch[Y2].Y),
					ImageMaskType, UV5, ViewRect);
			}
		}
	}
}

FVector4 UImageComponent::GetAdjustedBorders(FVector4 InBorder, const FRect& InAdjustedRect) const
{
	const FRect& OriginalRect = Rect;

	for (int32 Axis = 0; Axis <= 1; ++Axis)
	{
		float BorderScaleRatio;

		// The adjusted rect (adjusted for pixel correctness)
		// may be slightly larger than the original rect.
		// Adjust the border to match the adjustedRect to avoid
		// small gaps between borders.

		const auto& OriginalRectSize = OriginalRect.GetSize();
		const auto& AdjustedRectSize = InAdjustedRect.GetSize();
		if (OriginalRectSize[Axis] != 0)
		{
			BorderScaleRatio = AdjustedRectSize[Axis] / OriginalRectSize[Axis];
			InBorder[Axis] *= BorderScaleRatio;
			InBorder[Axis + 2] *= BorderScaleRatio;
		}

		// If the rect is smaller than the combined borders, then there's not room for the borders at their normal size.
		// In order to avoid artifacts with overlapping borders, we scale the borders down to fit.
		const float CombinedBorders = InBorder[Axis] + InBorder[Axis + 2];
		if (AdjustedRectSize[Axis] < CombinedBorders && CombinedBorders != 0)
		{
			BorderScaleRatio = AdjustedRectSize[Axis] / CombinedBorders;
			InBorder[Axis] *= BorderScaleRatio;
			InBorder[Axis + 2] *= BorderScaleRatio;
		}
	}
	
	return InBorder;
}

void UImageComponent::GenerateTiledSprite(FVertexHelper& VertexHelper)
{
	const auto ActiveSprite = GetActiveSprite();
	if (!IsValid(ActiveSprite))
		return;

	FVector4 OuterUV = FVector4(0, 0, 0, 0);
	FVector4 InnerUV = FVector4(0, 0, 0, 0);
	FVector4 SpriteBorder = FVector4(0, 0, 0, 0);
	FVector2D SpriteSize = FVector2D::UnitVector * 100;

	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& AtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(AtlasData.StartUV, AtlasData.StartUV + AtlasData.SizeUV);
		InnerUV = GetInnerUV(OuterUV);
		SpriteBorder = GetBorderSize(AtlasData);
		SpriteSize = GetSpriteSize(AtlasData);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
		InnerUV = GetInnerUV(OuterUV);
		SpriteBorder = GetBorderSize(Sprite2D);
	}
	
	FRect ViewRect = GetPixelAdjustedRect();
	float TileWidth = (SpriteSize.X - SpriteBorder.X - SpriteBorder.Z) / MultipliedPixelsPerUnit();
	float TileHeight = (SpriteSize.Y - SpriteBorder.Y - SpriteBorder.W) / MultipliedPixelsPerUnit();

	SpriteBorder = GetAdjustedBorders(SpriteBorder / MultipliedPixelsPerUnit(), ViewRect);

	FVector2D UVMin = FVector2D(InnerUV.X, InnerUV.W);
	FVector2D UVMax = FVector2D(InnerUV.Z, InnerUV.Y);

	FVector2D CenterUV2Min = FVector2D(-1 + TileWidth / ViewRect.Width, -1 + TileHeight / ViewRect.Height);
	FVector2D CenterUV2Max = FVector2D(1 - TileWidth / ViewRect.Width, 1 - TileHeight / ViewRect.Height);

	// Min to max max range for tiled region in coordinates relative to lower left corner.
	float XMin = SpriteBorder.X;
	float XMax = ViewRect.Width - SpriteBorder.Z;
	float YMin = SpriteBorder.Y;
	float YMax = ViewRect.Height - SpriteBorder.X;

	VertexHelper.Empty();

	FVector2D Clipped = UVMax;

	// if either width is zero we cant tile so just assume it was the full width.
	if (TileWidth <= 0)
		TileWidth = XMax - XMin;

	if (TileHeight <= 0)
		TileHeight = YMax - YMin;

	// Sprite has border, or is not in repeat mode, or cannot be repeated because of packing.
	// We cannot use texture tiling so we will generate a mesh of quads to tile the texture.

	// Evaluate how many vertices we will generate. Limit this number to something sane,
	// especially since meshes can not have more than 65000 vertices.

	int64 NTilesW = 0;
	int64 NTilesH = 0;

	if (bFillCenter)
	{
		NTilesW = FMath::CeilToFloat((XMax - XMin) / TileWidth);
		NTilesH = FMath::CeilToFloat((YMax - YMin) / TileHeight);

		double NVertices = 0;
		if (HasBorder())
		{
			NVertices = (NTilesW + 2.0) * (NTilesH + 2.0) * 4.0; // 4 vertices per tile
		}
		else
		{
			NVertices = NTilesW * NTilesH * 4.0; // 4 vertices per tile
		}

		if (NVertices > 65000.0)
		{
			UE_LOG(LogUGUI, Error, TEXT("Too many sprite tiles on Image %s. The tile size will be increased."), *GetFName().ToString());

			double MaxTiles = 65000.0 / 4.0; // Max number of vertices is 65000; 4 vertices per tile.
			double ImageRatio;
			if (HasBorder())
			{
				ImageRatio = (NTilesW + 2.0) / (NTilesH + 2.0);
			}
			else
			{
				ImageRatio = static_cast<double>(NTilesW) / NTilesH;
			}

			double TargetTilesW = FMath::Sqrt(MaxTiles / ImageRatio);
			double TargetTilesH = TargetTilesW * ImageRatio;
			if (HasBorder())
			{
				TargetTilesW -= 2;
				TargetTilesH -= 2;
			}

			NTilesW = FMath::Floor(TargetTilesW);
			NTilesH = FMath::Floor(TargetTilesH);
			TileWidth = (XMax - XMin) / NTilesW;
			TileHeight = (YMax - YMin) / NTilesH;
		}
	}
	else
	{
		if (HasBorder())
		{
			// Texture on the border is repeated only in one direction.
			NTilesW = FMath::CeilToFloat((XMax - XMin) / TileWidth);
			NTilesH = FMath::CeilToFloat((YMax - YMin) / TileHeight);
			double NVertices = (NTilesH + NTilesW + 2.0 /*corners*/) * 2.0 /*sides*/ * 4.0 /*vertices per tile*/;
			if (NVertices > 65000.0)
			{
				UE_LOG(LogUGUI, Error, TEXT("Too many sprite tiles on Image %s. The tile size will be increased."), *GetFName().ToString());

				double MaxTiles = 65000.0 / 4.0; // Max number of vertices is 65000; 4 vertices per tile.
				double ImageRatio = static_cast<double>(NTilesW) / NTilesH;
				double TargetTilesW = (MaxTiles - 4 /*corners*/) / (2 * (1.0 + ImageRatio));
				double TargetTilesH = TargetTilesW * ImageRatio;

				NTilesW = FMath::Floor(TargetTilesW);
				NTilesH = FMath::Floor(TargetTilesH);
				TileWidth = (XMax - XMin) / NTilesW;
				TileHeight = (YMax - YMin) / NTilesH;
			}
		}
	}

	FVector2D RectPosition = Rect.GetPosition();
	const FVector2D UV1 = GetUV1FromGraphicEffects();
	
	FVector2D UV5 = FVector2D::ZeroVector;
	if (ImageMaskType == EImageMaskMode::MaskMode_Circle)
	{
		UV5 = FVector2D(1,0);
	}
	else if (ImageMaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		UV5 = FVector2D(1,(1 - GetThickness()) * 0.5f);
	}
	
	if (bFillCenter)
	{
		for (int64 H = 0; H < NTilesH; ++H)
		{
			float Y1 = YMin + H * TileHeight;
			float Y2 = YMin + (H + 1) * TileHeight;

			if (Y2 > YMax)
			{
				Clipped.Y = UVMin.Y + (UVMax.Y - UVMin.Y) * (YMax - Y1) / (Y2 - Y1);
				Y2 = YMax;
			}

			Clipped.X = UVMax.X;

			for (int64 W = 0; W < NTilesW; ++W)
			{
				float X1 = XMin + W * TileWidth;
				float X2 = XMin + (W + 1) * TileWidth;

				if (X2 > XMax)
				{
					Clipped.X = UVMin.X + (UVMax.X - UVMin.X) * (XMax - X1) / (X2 - X1);
					X2 = XMax;
				}

				AddQuad(VertexHelper,
					FVector2D(X1, Y1) + RectPosition,
					FVector2D(X2, Y2) + RectPosition,
					Color,
					UVMin, Clipped,
					UV1,
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)W / NTilesW), FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)H / NTilesH)),
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)(W + 1) / NTilesW), FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)(H + 1) / NTilesH)),
					ImageMaskType, UV5, ViewRect);
			}
		}
	}

	if (HasBorder())
	{
		Clipped = UVMax;
		for (int64 H = 0; H < NTilesH; ++H)
		{
			float Y1 = YMin + H * TileHeight;
			float Y2 = YMin + (H + 1) * TileHeight;

			if (Y2 > YMax)
			{
				Clipped.Y = UVMin.Y + (UVMax.Y - UVMin.Y) * (YMax - Y1) / (Y2 - Y1);
				Y2 = YMax;
			}

			if (bFillCenter)
			{
				AddQuad(VertexHelper,
				   FVector2D(0, Y1) + RectPosition,
				   FVector2D(XMin, Y2) + RectPosition,
				   Color,
				   FVector2D(OuterUV.X, UVMin.Y),
				   FVector2D(UVMin.X, Clipped.Y),
				   UV1,
				   FVector2D(-1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)H / NTilesH)),
				   FVector2D(CenterUV2Min.X, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)(H + 1) / NTilesH)),
				   ImageMaskType, UV5, ViewRect);
				AddQuad(VertexHelper,
					FVector2D(XMax, Y1) + RectPosition,
					FVector2D(ViewRect.Width, Y2) + RectPosition,
					Color,
					FVector2D(UVMax.X, UVMin.Y),
					FVector2D(OuterUV.Z, Clipped.Y),
					UV1,
					FVector2D(CenterUV2Max.X, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)H / NTilesH)),
					FVector2D(1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)(H + 1) / NTilesH)),
					ImageMaskType, UV5, ViewRect);
			}
			else
			{
				AddQuad(VertexHelper,
				   FVector2D(0, Y1) + RectPosition,
				   FVector2D(XMin, Y2) + RectPosition,
				   Color,
				   FVector2D(OuterUV.X, UVMin.Y),
				   FVector2D(UVMin.X, Clipped.Y),
				   UV1,
				   FVector2D(-1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)H / NTilesH)),
				   FVector2D(1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)(H + 1) / NTilesH)),
				   ImageMaskType, UV5, ViewRect);
				AddQuad(VertexHelper,
					FVector2D(XMax, Y1) + RectPosition,
					FVector2D(ViewRect.Width, Y2) + RectPosition,
					Color,
					FVector2D(UVMax.X, UVMin.Y),
					FVector2D(OuterUV.Z, Clipped.Y),
					UV1,
					FVector2D(-1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)H / NTilesH)),
					FVector2D(1, FMath::Lerp(CenterUV2Min.Y, CenterUV2Max.Y, (float)(H + 1) / NTilesH)),
					ImageMaskType, UV5, ViewRect);
			}
		}

		// Bottom and top tiled border
		Clipped = UVMax;
		for (int64 W = 0; W < NTilesW; ++W)
		{
			float X1 = XMin + W * TileWidth;
			float X2 = XMin + (W + 1) * TileWidth;

			if (X2 > XMax)
			{
				Clipped.X = UVMin.X + (UVMax.X - UVMin.X) * (XMax - X1) / (X2 - X1);
				X2 = XMax;
			}

			if (bFillCenter)
			{
				AddQuad(VertexHelper,
					FVector2D(X1, 0) + RectPosition,
					FVector2D(X2, YMin) + RectPosition,
					Color,
					FVector2D(UVMin.X, OuterUV.Y),
					FVector2D(Clipped.X, UVMin.Y),
					UV1,
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)W / NTilesW), -1),
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)(W + 1) / NTilesW), CenterUV2Min.Y),
					ImageMaskType, UV5, ViewRect);
				AddQuad(VertexHelper,
					FVector2D(X1, YMax) + RectPosition,
					FVector2D(X2, ViewRect.Height) + RectPosition,
					Color,
					FVector2D(UVMin.X, UVMax.Y),
					FVector2D(Clipped.X, OuterUV.W),
					UV1,
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)W / NTilesW), CenterUV2Max.Y),
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)(W + 1) / NTilesW), 1),
					ImageMaskType, UV5, ViewRect);
			}
			else
			{
				AddQuad(VertexHelper,
					FVector2D(X1, 0) + RectPosition,
					FVector2D(X2, YMin) + RectPosition,
					Color,
					FVector2D(UVMin.X, OuterUV.Y),
					FVector2D(Clipped.X, UVMin.Y),
					UV1,
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)W / NTilesW), -1),
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)(W + 1) / NTilesW), 1),
					ImageMaskType, UV5, ViewRect);
				AddQuad(VertexHelper,
					FVector2D(X1, YMax) + RectPosition,
					FVector2D(X2, ViewRect.Height) + RectPosition,
					Color,
					FVector2D(UVMin.X, UVMax.Y),
					FVector2D(Clipped.X, OuterUV.W),
					UV1,
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)W / NTilesW), -1),
					FVector2D(FMath::Lerp(CenterUV2Min.X, CenterUV2Max.X, (float)(W + 1) / NTilesW), 1),
					ImageMaskType, UV5, ViewRect);
			}
		}

		// Corners
		AddQuad(VertexHelper,
			FVector2D(0, 0) + RectPosition,
			FVector2D(XMin, YMin) + RectPosition,
			Color,
			FVector2D(OuterUV.X, OuterUV.Y),
			FVector2D(UVMin.X, UVMin.Y), UV1, FVector2D(-1, -1), CenterUV2Min, ImageMaskType, UV5, ViewRect);
		AddQuad(VertexHelper,
			FVector2D(XMax, 0) + RectPosition,
			FVector2D(ViewRect.Width, YMin) + RectPosition,
			Color,
			FVector2D(UVMax.X, OuterUV.Y),
			FVector2D(OuterUV.Z, UVMin.Y), UV1, FVector2D(CenterUV2Max.X, -1), FVector2D(1, CenterUV2Min.Y),
			ImageMaskType, UV5, ViewRect);
		AddQuad(VertexHelper,
			FVector2D(0, YMax) + RectPosition,
			FVector2D(XMin, ViewRect.Height) + RectPosition,
			Color,
			FVector2D(OuterUV.X, UVMax.Y),
			FVector2D(UVMin.X, OuterUV.W), UV1, FVector2D(-1, CenterUV2Max.Y), FVector2D(CenterUV2Min.X, 1),
			ImageMaskType, UV5, ViewRect);
		AddQuad(VertexHelper,
			FVector2D(XMax, YMax) + RectPosition,
			FVector2D(ViewRect.Width, ViewRect.Height) + RectPosition,
			Color,
			FVector2D(UVMax.X, UVMax.Y),
			FVector2D(OuterUV.Z, OuterUV.W), UV1, FVector2D(CenterUV2Max.X, CenterUV2Max.Y), FVector2D(1, 1),
			ImageMaskType, UV5, ViewRect);
	}
}

void UImageComponent::GenerateFilledSprite(FVertexHelper& VertexHelper, bool bInPreserveAspect)
{
	VertexHelper.Empty();

    if (FillAmount < 0.001f)
        return;

	FRect OriginalViewRect;
	FVector4 ViewRect = GetDrawingDimensions(bInPreserveAspect, OriginalViewRect);
	
	FVector4 OuterUV = FVector4(0, 0, 0, 0);
	const auto ActiveSprite = GetActiveSprite();
	if (const auto PaperSprite = Cast<UPaperSprite>(ActiveSprite))
	{
		const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(SlateAtlasData.StartUV, SlateAtlasData.StartUV + SlateAtlasData.SizeUV);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(ActiveSprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
	}
	
    float Tx0 = OuterUV.X;
    float Ty0 = OuterUV.W;
    float Tx1 = OuterUV.Z;
    float Ty1 = OuterUV.Y;

    // Horizontal and vertical filled sprites are simple -- just end the Image prematurely
	if (FillMethod == EFillMethod::FillMethod_Horizontal)
	{
		const float Fill = (Tx1 - Tx0) * FillAmount;

		if (FillOrigin == 1)
		{
			ViewRect.X = ViewRect.Z - (ViewRect.Z - ViewRect.X) * FillAmount;
			Tx0 = Tx1 - Fill;
		}
		else
		{
			ViewRect.Z = ViewRect.X + (ViewRect.Z - ViewRect.X) * FillAmount;
			Tx1 = Tx0 + Fill;
		}
	}
	else if (FillMethod == EFillMethod::FillMethod_Vertical)
	{
		const float Fill = (Ty1 - Ty0) * FillAmount;

		if (FillOrigin == 1)
		{
			ViewRect.Y = ViewRect.W - (ViewRect.W - ViewRect.Y) * FillAmount;
			Ty0 = Ty1 - Fill;
		}
		else
		{
			ViewRect.W = ViewRect.Y + (ViewRect.W - ViewRect.Y) * FillAmount;
			Ty1 = Ty0 + Fill;
		}
	}

	FVector2D QuadPositions[4] =
	{
		FVector2D(ViewRect.X, ViewRect.Y),
		FVector2D(ViewRect.X, ViewRect.W),
		FVector2D(ViewRect.Z, ViewRect.W),
		FVector2D(ViewRect.Z, ViewRect.Y)
	};

	FVector2D QuadUVs[4] =
	{
		FVector2D(Tx0, Ty0),
		FVector2D(Tx0, Ty1),
		FVector2D(Tx1, Ty1),
		FVector2D(Tx1, Ty0)
	};

	FVector2D QuadUV2s[4] =
	{
		FVector2D(-1, -1),
		FVector2D(-1, 1),
		FVector2D(1, 1),
		FVector2D(1, -1)
	};
	
	const FVector2D UV1 = GetUV1FromGraphicEffects();
	
	FVector2D UV5 = FVector2D::ZeroVector;
	if (ImageMaskType == EImageMaskMode::MaskMode_Circle)
	{
		UV5 = FVector2D(1,0);
	}
	else if (ImageMaskType == EImageMaskMode::MaskMode_CircleRing)
	{
		UV5 = FVector2D(1,(1 - GetThickness()) * 0.5f);
	}
	
	if (FillAmount < 1 && FillMethod != EFillMethod::FillMethod_Horizontal && FillMethod !=
		EFillMethod::FillMethod_Vertical)
	{
		if (FillMethod == EFillMethod::FillMethod_Radial90)
		{
			if (RadialCut(QuadPositions, QuadUVs, QuadUV2s, FillAmount, bFillClockwise, FillOrigin))
				AddQuad(VertexHelper, QuadPositions, Color, QuadUVs, UV1, QuadUV2s, ImageMaskType, UV5, OriginalViewRect);
		}
		else if (FillMethod == EFillMethod::FillMethod_Radial180)
		{
			for (int32 Side = 0; Side < 2; ++Side)
			{
				float Fx0, Fx1, Fy0, Fy1;
				const int32 Even = FillOrigin > 1 ? 1 : 0;

				if (FillOrigin == 0 || FillOrigin == 2)
				{
					Fy0 = 0;
					Fy1 = 1;
					if (Side == Even)
					{
						Fx0 = 0;
						Fx1 = 0.5f;
					}
					else
					{
						Fx0 = 0.5f;
						Fx1 = 1;
					}
				}
				else
				{
					Fx0 = 0;
					Fx1 = 1;
					if (Side == Even)
					{
						Fy0 = 0.5f;
						Fy1 = 1;
					}
					else
					{
						Fy0 = 0;
						Fy1 = 0.5f;
					}
				}

				QuadPositions[0].X = FMath::Lerp(ViewRect.X, ViewRect.Z, Fx0);
				QuadPositions[1].X = QuadPositions[0].X;
				QuadPositions[2].X = FMath::Lerp(ViewRect.X, ViewRect.Z, Fx1);
				QuadPositions[3].X = QuadPositions[2].X;

				QuadPositions[0].Y = FMath::Lerp(ViewRect.Y, ViewRect.W, Fy0);
				QuadPositions[1].Y = FMath::Lerp(ViewRect.Y, ViewRect.W, Fy1);
				QuadPositions[2].Y = QuadPositions[1].Y;
				QuadPositions[3].Y = QuadPositions[0].Y;

				QuadUVs[0].X = FMath::Lerp(Tx0, Tx1, Fx0);
				QuadUVs[1].X = QuadUVs[0].X;
				QuadUVs[2].X = FMath::Lerp(Tx0, Tx1, Fx1);
				QuadUVs[3].X = QuadUVs[2].X;

				QuadUVs[0].Y = FMath::Lerp(Ty0, Ty1, Fy0);
				QuadUVs[1].Y = FMath::Lerp(Ty0, Ty1, Fy1);
				QuadUVs[2].Y = QuadUVs[1].Y;
				QuadUVs[3].Y = QuadUVs[0].Y;

				QuadUV2s[0].X = FMath::Lerp(-1, 1, Fx0);
				QuadUV2s[1].X = QuadUV2s[0].X;
				QuadUV2s[2].X = FMath::Lerp(-1, 1, Fx1);
				QuadUV2s[3].X = QuadUV2s[2].X;

				QuadUV2s[0].Y = FMath::Lerp(-1, 1, Fy0);
				QuadUV2s[1].Y = FMath::Lerp(-1, 1, Fy1);
				QuadUV2s[2].Y = QuadUV2s[1].Y;
				QuadUV2s[3].Y = QuadUV2s[0].Y;

				const float Val = bFillClockwise ? FillAmount * 2 - Side : FillAmount * 2 - (1 - Side);

				if (RadialCut(QuadPositions, QuadUVs, QuadUV2s, FMath::Clamp(Val, 0.0f, 1.0f), bFillClockwise,
				              ((Side + FillOrigin + 3) % 4)))
				{
					AddQuad(VertexHelper, QuadPositions, Color, QuadUVs, UV1, QuadUV2s, ImageMaskType, UV5, OriginalViewRect);
				}
			}
		}
		else if (FillMethod == EFillMethod::FillMethod_Radial360)
		{
			for (int32 QuadIndex = 0; QuadIndex < 4; ++QuadIndex)
			{
				float Fx0, Fx1, Fy0, Fy1;

				if (QuadIndex < 2)
				{
					Fx0 = 0;
					Fx1 = 0.5f;
				}
				else
				{
					Fx0 = 0.5f;
					Fx1 = 1;
				}

				if (QuadIndex == 0 || QuadIndex == 3)
				{
					Fy0 = 0;
					Fy1 = 0.5f;
				}
				else
				{
					Fy0 = 0.5f;
					Fy1 = 1;
				}

				QuadPositions[0].X = FMath::Lerp(ViewRect.X, ViewRect.Z, Fx0);
				QuadPositions[1].X = QuadPositions[0].X;
				QuadPositions[2].X = FMath::Lerp(ViewRect.X, ViewRect.Z, Fx1);
				QuadPositions[3].X = QuadPositions[2].X;

				QuadPositions[0].Y = FMath::Lerp(ViewRect.Y, ViewRect.W, Fy0);
				QuadPositions[1].Y = FMath::Lerp(ViewRect.Y, ViewRect.W, Fy1);
				QuadPositions[2].Y = QuadPositions[1].Y;
				QuadPositions[3].Y = QuadPositions[0].Y;

				QuadUVs[0].X = FMath::Lerp(Tx0, Tx1, Fx0);
				QuadUVs[1].X = QuadUVs[0].X;
				QuadUVs[2].X = FMath::Lerp(Tx0, Tx1, Fx1);
				QuadUVs[3].X = QuadUVs[2].X;

				QuadUVs[0].Y = FMath::Lerp(Ty0, Ty1, Fy0);
				QuadUVs[1].Y = FMath::Lerp(Ty0, Ty1, Fy1);
				QuadUVs[2].Y = QuadUVs[1].Y;
				QuadUVs[3].Y = QuadUVs[0].Y;

				QuadUV2s[0].X = FMath::Lerp(-1, 1, Fx0);
				QuadUV2s[1].X = QuadUV2s[0].X;
				QuadUV2s[2].X = FMath::Lerp(-1, 1, Fx1);
				QuadUV2s[3].X = QuadUV2s[2].X;

				QuadUV2s[0].Y = FMath::Lerp(-1, 1, Fy0);
				QuadUV2s[1].Y = FMath::Lerp(-1, 1, Fy1);
				QuadUV2s[2].Y = QuadUV2s[1].Y;
				QuadUV2s[3].Y = QuadUV2s[0].Y;

				const float Val = bFillClockwise
					                  ? FillAmount * 4 - ((QuadIndex + FillOrigin) % 4)
					                  : FillAmount * 4 - (3 - ((QuadIndex + FillOrigin) % 4));

				const int32 StartQuad = bFillClockwise ?  (4 - FillOrigin) % 4 : (3 - FillOrigin) % 4;
				if (RadialCut(QuadPositions, QuadUVs, FMath::Clamp(Val, 0.0f, 1.0f), bFillClockwise,((QuadIndex + 2) % 4)))
				{
					RadialCutUV2(QuadUV2s, Val, bFillClockwise, StartQuad, QuadIndex, (QuadIndex + 2) % 4);
					AddQuad(VertexHelper, QuadPositions, Color, QuadUVs, UV1, QuadUV2s, ImageMaskType, UV5, OriginalViewRect, ((QuadIndex + 2) % 4));
				}
			}
		}
	}
	else
	{
		AddQuad(VertexHelper, QuadPositions, Color, QuadUVs, UV1, QuadUV2s, ImageMaskType, UV5, OriginalViewRect);
	}
}

/////////////////////////////////////////////////////
