#include "Core/Widgets/BackgroundBlurComponent.h"
#include "UGUISettings.h"
#include "Core/GeometryUtility.h"
#include "Core/Render/VertexHelper.h"

/////////////////////////////////////////////////////
// UBackgroundBlurComponent

UBackgroundBlurComponent::UBackgroundBlurComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    BlurStrength = 0;
    BlurRadius = 0;
    bApplyAlphaToBlur = true;
    bOverrideAutoRadiusCalculation = false;
    BlurMaskType = EBackgroundBlurMaskMode::MaskMode_None;
	MaskTexture = nullptr;
}

void UBackgroundBlurComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
    if (BlurStrength < FLT_EPSILON)
    {
	    return;
    }

    const auto& FinalRect = GetPixelAdjustedRect();
    const float BottomLeftX = FinalRect.XMin;
    const float BottomLeftY = FinalRect.YMin;
    const float TopRightX = BottomLeftX + FinalRect.Width;
    const float TopRightY = BottomLeftY + FinalRect.Height;
	
	FLinearColor FinalColor = FLinearColor(1, 1, 1, Color.A);
	if (!FPlatformMisc::SupportsBackbufferSampling())
	{
		FinalColor = UUGUISettings::Get()->BlurDisabledColor;
	}
	
    const FVector2D UV1 = GetUV1FromGraphicEffects();
    VertexHelper.Empty();

	if (!bAntiAliasing)
	{
	    if (BlurMaskType == EBackgroundBlurMaskMode::MaskMode_Circle)
	    {
	        FGeometryUtility::GenerateCircle(VertexHelper, bAntiAliasing, FVector2D((BottomLeftX + TopRightX) * 0.5, (BottomLeftY + TopRightY) * 0.5),
                60, 0.5 * (TopRightX - BottomLeftX), UV1, Color, FVector2D(0, 0), FVector2D(1, 1));
	    }
	    else
	    {
	    	VertexHelper.Reserve(4, 6);

	    	VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), FinalColor, FVector2D(0, 1), UV1);
	    	VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), FinalColor, FVector2D(1, 1), UV1);
	    	VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), FinalColor, FVector2D(1, 0), UV1);
	    	VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), FinalColor, FVector2D(0, 0), UV1);

	    	VertexHelper.AddTriangle(0, 1, 2);
	    	VertexHelper.AddTriangle(2, 3, 0);
	    }
	}
    else
    {
    	if (BlurMaskType == EBackgroundBlurMaskMode::MaskMode_Circle)
    	{
    		FGeometryUtility::GenerateCircle(VertexHelper, bAntiAliasing, FVector2D((BottomLeftX + TopRightX) * 0.5, (BottomLeftY + TopRightY) * 0.5),
				0, 0.5 * (TopRightX - BottomLeftX), UV1, Color, FVector2D(0, 0), FVector2D(1, 1));
    	}
    	else
    	{
    		VertexHelper.Reserve(4, 6);

    		VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(0, 1), UV1,
			FVector2D(-1, -1));
    		VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(1, 1), UV1,
			FVector2D(-1, 1));
    		VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(1, 0), UV1,
			FVector2D(1, 1));
    		VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color, FVector2D(0, 0), UV1,
			FVector2D(1, -1));

    		VertexHelper.AddTriangle(0, 1, 2);
    		VertexHelper.AddTriangle(2, 3, 0);
    	}
    }
	
	if (!FPlatformMisc::SupportsBackbufferSampling())
	{
		return;
	}
	
	if (!GraphicData.IsValid())
	{
		GraphicData = MakeShareable(new FUIBlurGraphicData());
		GraphicData->BlurStrength = BlurStrength;
		GraphicData->BlurRadius = BlurRadius;
		GraphicData->bApplyAlphaToBlur = bApplyAlphaToBlur;
		GraphicData->bOverrideAutoRadiusCalculation = bOverrideAutoRadiusCalculation;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetGraphicData(GraphicData);
		CanvasRendererComp->SetGraphicType(EUIGraphicType::PostProcessBlur);
	}
}

/////////////////////////////////////////////////////
