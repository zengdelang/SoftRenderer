#include "Core/Widgets/LineComponent.h"
#include "Core/GeometryUtility.h"
#include "Core/Render/VertexHelper.h"
#include "UObject/ConstructorHelpers.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// ULineComponent

ULineComponent::ULineComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinderOptional<USprite2D> DefaultSprite2D_Finder(TEXT("/UGUI/DefaultResources/Textures/UGUI_DefaultWhite_Sprite2D.UGUI_DefaultWhite_Sprite2D"));
	Sprite = DefaultSprite2D_Finder.Get();
	
	bClosedLine = false;
	LineThickness = 1.0f;
	PointColor = FLinearColor::White;
	PointSize = 10.0f;
	PointMode = EPointMode::None;
	LineType = ELineType::CustomPoints;
	DrawLineTolerance = 10.0f;
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- Line::OnPopulateMesh"), STAT_UnrealGUI_LineOnPopulateMesh, STATGROUP_UnrealGUI);
void ULineComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_LineOnPopulateMesh);
	
	VertexHelper.Reset();
	const FVector2D UV1 = GetUV1FromGraphicEffects();

	FVector4 OuterUV = FVector4(0, 0, 1, 1);
	if (const auto PaperSprite = Cast<UPaperSprite>(Sprite))
	{
		const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
		OuterUV = FVector4(SlateAtlasData.StartUV, SlateAtlasData.StartUV + SlateAtlasData.SizeUV);
	}
	else if (const auto Sprite2D = Cast<USprite2D>(Sprite))
	{
		OuterUV = Sprite2D->GetOuterUV();
	}

	if (LineType == ELineType::CustomPoints)
	{
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, LinePoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, LinePoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));
	}
	else if (LineType == ELineType::RectCustomPoints && LinePoints.Num() > 0)
	{
		TArray<FVector2D> FinalLinePoints;
		FinalLinePoints.Reserve(LinePoints.Num());

		const FRect TransformRect = GetTransformRect();
		const FVector2D Original = FVector2D(TransformRect.XMin, TransformRect.YMin);
		const FVector2D Size = FVector2D(TransformRect.Width, TransformRect.Height);
		
		for(const auto& Point : LinePoints)
		{
			FinalLinePoints.Emplace(Original + Size * Point);
		}
		
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, FinalLinePoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, FinalLinePoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));
	}
	else if (LineType == ELineType::RectLeftAndRight)
	{
		GenerateRectPoints(ELineType::RectLeft, bClosedLine);
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, RectPoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, RectPoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));

		GenerateRectPoints(ELineType::RectRight, bClosedLine);
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, RectPoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, RectPoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));
	}
	else if (LineType == ELineType::RectTopAndBottom)
	{
		GenerateRectPoints(ELineType::RectTop, bClosedLine);
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, RectPoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, RectPoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));

		GenerateRectPoints(ELineType::RectBottom, bClosedLine);
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, RectPoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, RectPoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));
	}
	else
	{
		GenerateRectPoints(LineType, bClosedLine);
		FGeometryUtility::GenerateLines(VertexHelper, bAntiAliasing, bClosedLine, UV1, LineThickness, Color, RectPoints, DrawLineTolerance, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.Z, OuterUV.W));
		FGeometryUtility::GeneratePoints(VertexHelper, bAntiAliasing, UV1, PointSize, PointColor, PointMode, RectPoints, FVector2D(OuterUV.X, OuterUV.Y), FVector2D(OuterUV.X, OuterUV.Y));
	}
}

void ULineComponent::OnEnable()
{
	if (const auto Sprite2D = Cast<USprite2D>(Sprite))
	{
		Sprite2D->AddSpriteListener(this);
		Sprite2D->IncreaseReferenceCount();
	}
	
	Super::OnEnable();
}

void ULineComponent::OnDisable()
{
	if (const auto Sprite2D = Cast<USprite2D>(Sprite))
	{
		Sprite2D->RemoveSpriteListener(this);
		Sprite2D->DecreaseReferenceCount();
	}
	
	Super::OnDisable();
}

void ULineComponent::SetSprite(UObject* InSprite)
{
	if (!IsActiveAndEnabled())
	{
		Sprite = InSprite;
		return;
	}

	if (IsValid(Sprite))
	{
		if (Sprite != InSprite)
		{
			const UTexture* OldAtlasTexture = nullptr;
			const UTexture* NewAtlasTexture = nullptr;

			if (const auto PaperSprite = Cast<UPaperSprite>(Sprite))
			{
				const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
				OldAtlasTexture = SlateAtlasData.AtlasTexture;
			}
			else if (const auto Sprite2D = Cast<USprite2D>(Sprite))
			{
				Sprite2D->RemoveSpriteListener(this);
				Sprite2D->DecreaseReferenceCount();
				OldAtlasTexture = Sprite2D->GetSpriteTextureRaw();
			}

			if (const auto PaperSprite = Cast<UPaperSprite>(InSprite))
			{
				const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
				NewAtlasTexture = SlateAtlasData.AtlasTexture;
			}
			else if (const auto Sprite2D = Cast<USprite2D>(InSprite))
			{
				Sprite2D->AddSpriteListener(this);
				Sprite2D->IncreaseReferenceCount();
				NewAtlasTexture = Sprite2D->GetSpriteTexture();
			}
			
			const bool bSkipMaterialUpdate = OldAtlasTexture == NewAtlasTexture;

			Sprite = InSprite;
			
			if (!bSkipMaterialUpdate)
				SetMaterialDirty();

			SetVerticesDirty();
		}
	}
	else if (IsValid(InSprite))
	{
		Sprite = InSprite;

		const UTexture* AtlasTexture = nullptr;
		if (const auto PaperSprite = Cast<UPaperSprite>(InSprite))
		{
			const auto& SlateAtlasData = PaperSprite->GetSlateAtlasData();
			AtlasTexture = SlateAtlasData.AtlasTexture;
		}
		else if (const auto Sprite2D = Cast<USprite2D>(InSprite))
		{
			Sprite2D->AddSpriteListener(this);
			Sprite2D->IncreaseReferenceCount();
			AtlasTexture = Sprite2D->GetSpriteTexture();
		}
		
		if (IsValid(AtlasTexture))
			SetMaterialDirty();

		SetVerticesDirty();
	}
}

/////////////////////////////////////////////////////
