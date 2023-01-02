#include "Core/Widgets/BackgroundGlitchComponent.h"
#include "Core/Render/VertexHelper.h"

/////////////////////////////////////////////////////
// UBackgroundGlitchComponent

UBackgroundGlitchComponent::UBackgroundGlitchComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    bUseGlitch = false;
    Strength = 1;
    Method = EUIGlitchType::UIGlitchType_None;

    DownSampleAmount = 1;

#if UE_EDITOR
    RenderOpacity = 0.1f;
#endif
}

#if UE_EDITOR

void UBackgroundGlitchComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
    if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, bUseGlitch))
    {
        SetUseGlitch(bUseGlitch);
    }
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, Strength))
    {
        SetStrength(Strength);
    }
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, Method))
    {
        SetMethod(Method);
    }
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, DownSampleAmount))
    {
        SetDownSampleAmount(DownSampleAmount);
    }
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, UIGlitchAnalogNoiseAndRGBSplitSet))
    {
        SetUIGlitchAnalogNoiseAndRGBSplitSet(UIGlitchAnalogNoiseAndRGBSplitSet);
    }
    else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UBackgroundGlitchComponent, UIGlitchImageBlockSet))
    {
        SetUIGlitchImageBlockSet(UIGlitchImageBlockSet);
    }

    Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UBackgroundGlitchComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
    if (!FPlatformMisc::SupportsBackbufferSampling())
    {
        const auto CanvasRendererComp = GetCanvasRenderer();
        if (IsValid(CanvasRendererComp))
        {
            CanvasRendererComp->SetHidePrimitive(true);
        }
        return;
    }
    
    const auto& FinalRect = GetPixelAdjustedRect();
    const float BottomLeftX = FinalRect.XMin;
    const float BottomLeftY = FinalRect.YMin;
    const float TopRightX = BottomLeftX + FinalRect.Width;
    const float TopRightY = BottomLeftY + FinalRect.Height;
 
    const FLinearColor& FinalColor = FLinearColor(1,1,1, Color.A);
    const FVector2D UV1 = GetUV1FromGraphicEffects();
	
    VertexHelper.Empty();
 
	if (!bAntiAliasing)
	{
        VertexHelper.Reserve(4, 6);
 
        VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), FinalColor, FVector2D(0, 1), UV1);
        VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), FinalColor, FVector2D(1, 1), UV1);
        VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), FinalColor, FVector2D(1, 0), UV1);
        VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), FinalColor, FVector2D(0, 0), UV1);
 
        VertexHelper.AddTriangle(0, 1, 2);
        VertexHelper.AddTriangle(2, 3, 0);
	}
    else
    {
        VertexHelper.Reserve(5, 12);
 
        VertexHelper.AddVert(FVector(BottomLeftX, BottomLeftY, 0), Color, FVector2D(0, 1), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(TopRightX, BottomLeftY, 0), Color, FVector2D(1, 1), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(BottomLeftX + (TopRightX - BottomLeftX) * 0.5, BottomLeftY + (TopRightY - BottomLeftY) * 0.5, 0),
            Color, FVector2D(0.5, 0.5), UV1, FVector2D(0, 0));
        VertexHelper.AddVert(FVector(TopRightX, TopRightY, 0), Color, FVector2D(1, 0), UV1, FVector2D(1, 0));
        VertexHelper.AddVert(FVector(BottomLeftX, TopRightY, 0), Color,  FVector2D(0, 0), UV1, FVector2D(1, 0));
 
        VertexHelper.AddTriangle(2, 0, 1);
        VertexHelper.AddTriangle(2, 1, 3);
        VertexHelper.AddTriangle(2, 3, 4);
        VertexHelper.AddTriangle(2, 4, 0);
    }
	
    if (!GraphicData.IsValid())
    {
        GraphicData = MakeShareable(new FUIGlitchGraphicData());
        GraphicData->bUseGlitch = bUseGlitch;
        GraphicData->Method = Method;
        GraphicData->Strength = Strength;

        GraphicData->DownSampleAmount = DownSampleAmount;

        GraphicData->UIGlitchAnalogNoiseAndRGBSplitSet = UIGlitchAnalogNoiseAndRGBSplitSet;
        GraphicData->UIGlitchImageBlockSet = UIGlitchImageBlockSet;

    }

    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetGraphicData(GraphicData);
        CanvasRendererComp->SetGraphicType(EUIGraphicType::PostProcessGlitch);
    }
}

/////////////////////////////////////////////////////
