#include "Core/Widgets/GraphicElementInterface.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Render/CanvasRendererSubComponent.h"

/////////////////////////////////////////////////////
// UGraphicElementInterface

FVertexHelper IGraphicElementInterface::StaticVertexHelper;

UGraphicElementInterface::UGraphicElementInterface(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

int32 IGraphicElementInterface::GetDepth()
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        return CanvasRendererComp->GetAbsoluteDepth();
    }

    return -1;
}

void IGraphicElementInterface::SetAllDirty()
{
    SetLayoutDirty();
    SetMaterialDirty();
    SetVerticesDirty();
    SetGraphicEffectsDirty();
    SetRenderOpacityDirty();
}

void IGraphicElementInterface::SetCanvasRendererColor(FLinearColor InColor)
{
    const auto CanvasRendererComp = GetCanvasRenderer();
    if (IsValid(CanvasRendererComp))
    {
        CanvasRendererComp->SetColor(InColor);
    }
}

void IGraphicElementInterface::CrossFadeColor(FLinearColor TargetColor, float Duration, bool bIgnoreTimeScale, bool bUseAlpha)
{
    CrossFadeColor(TargetColor, Duration, bIgnoreTimeScale, bUseAlpha, true);
}

void IGraphicElementInterface::CrossFadeAlpha(float Alpha, float Duration, bool bIgnoreTimeScale)
{
    CrossFadeColor(CreateColorFromAlpha(Alpha), Duration, bIgnoreTimeScale, true, false);
}

FLinearColor IGraphicElementInterface::CreateColorFromAlpha(float Alpha)
{
    FLinearColor AlphaColor = FLinearColor::Black;
    AlphaColor.A = Alpha;
    return AlphaColor;
}

/////////////////////////////////////////////////////
