#include "Core/Widgets/RectMask2DSubComponent.h"
#include "Core/Culling/ClippableInterface.h"
#include "Core/Culling/ClipperRegistry.h"
#include "Core/Culling/Clipping.h"
#include "Core/Culling/MaskUtilities.h"
#include "Core/Culling/RectangularVertexClipper.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Widgets/MaskableGraphicElementInterface.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// URectMask2DSubComponent

URectMask2DSubComponent::URectMask2DSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RootCanvas = nullptr;
    RectTransform = nullptr;

	bForceClip = false;
    bShouldRecalculateClipRects = false;
}

UCanvasSubComponent* URectMask2DSubComponent::GetCanvas()
{
    if (!IsValid(RootCanvas))
    {
        RootCanvas = GetOwnerCanvas();
        if (IsValid(RootCanvas))
        {
            RootCanvas = RootCanvas->GetRootCanvas();
        }
    }
    return RootCanvas;
}

FRect URectMask2DSubComponent::GetCanvasRect()
{
    return FRectangularVertexClipper::GetCanvasRect(GetRectTransform(), RootCanvas);
}

FRect URectMask2DSubComponent::GetRootCanvasRect()
{
    return FRectangularVertexClipper::GetCanvasRect(GetRectTransform(), RootCanvas);
}

URectTransformComponent* URectMask2DSubComponent::GetRectTransform()
{
	if (!IsValid(RectTransform))
	{
        RectTransform = Cast<URectTransformComponent>(GetOuter());
	}
    return RectTransform;
}

void URectMask2DSubComponent::OnEnable()
{
    Super::OnEnable();

	bShouldRecalculateClipRects = true;
    FClipperRegistry::Register(this);
	FMaskUtilities::Notify2DMaskStateChanged(GetRectTransform());
}

void URectMask2DSubComponent::OnDisable()
{
    // we call base OnDisable first here
    // as we need to have the IsActive return the
    // correct value when we notify the children
    // that the mask state has changed.
    Super::OnDisable();

    ClipTargets.Empty();
    MaskableTargets.Empty();
    Clippers.Empty();
    FClipperRegistry::Unregister(this);
    FMaskUtilities::Notify2DMaskStateChanged(GetRectTransform());
}

void URectMask2DSubComponent::OnCanvasHierarchyChanged()
{
    RootCanvas = nullptr;
	
    Super::OnCanvasHierarchyChanged();
	
    bShouldRecalculateClipRects = true;
}

void URectMask2DSubComponent::OnTransformParentChanged()
{
    RootCanvas = nullptr;
    
    Super::OnTransformParentChanged();
    
	bShouldRecalculateClipRects = true;
}

#if WITH_EDITORONLY_DATA

void URectMask2DSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

	bShouldRecalculateClipRects = true;

    if (!IsActiveAndEnabled())
        return;

    FMaskUtilities::Notify2DMaskStateChanged(GetRectTransform());
}

void URectMask2DSubComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    Super::PostEditChangeChainProperty(PropertyChangedEvent);

    bShouldRecalculateClipRects = true;
    
    if (!IsActiveAndEnabled())
        return;
    
    FMaskUtilities::Notify2DMaskStateChanged(GetRectTransform());
}

#endif

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- IsRaycastLocationValid"), STAT_UnrealGUI_Sub_RectMaskImageRaycast, STATGROUP_UnrealGUI);
bool URectMask2DSubComponent::IsRaycastLocationValid(IMaskableGraphicElementInterface* MaskableGraphicElement, const FVector& WorldRayOrigin, const FVector& WorldRayDir, bool bIgnoreReversedGraphicsScreenPoint)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Sub_RectMaskImageRaycast);

    if (!IsActiveAndEnabled())
        return true;

    if (MaskableGraphicElement && !MaskableGraphicElement->IsMaskable())
        return true;
    
    const auto RectTransformComp = Cast<URectTransformComponent>(GetOuter());
    if (!IsValid(RectTransformComp))
        return true;
    
    return FRectTransformUtility::RectangleIntersectRay(RectTransformComp, WorldRayOrigin, WorldRayDir, bIgnoreReversedGraphicsScreenPoint);
}

void URectMask2DSubComponent::PerformClipping()
{
    if (!IsValid(GetCanvas()))
    {
        return;
    }

    // if the parents are changed
    // or something similar we
    // do a recalculate here
    if (bShouldRecalculateClipRects)
    {
        FMaskUtilities::GetRectMasksForClip(this, Clippers);
        bShouldRecalculateClipRects = false;
    }

    // get the compound rects from
    // the clippers that are valid
    bool bValidRect = true;
    FRect ClipRect = FClipping::FindCullAndClipRect(Clippers, bValidRect);

    // If the mask is in ScreenSpaceOverlay/Camera render mode, its content is only rendered when its rect
    // overlaps that of the root canvas.
    const ECanvasRenderMode RenderMode = RootCanvas->GetRenderMode();

    bool bMaskIsCulled = RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay || RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree;
	if (bMaskIsCulled)
	{
        bMaskIsCulled = !ClipRect.Overlaps(GetRootCanvasRect(), true);
	}

    if (bMaskIsCulled)
    {
        // Children are only displayed when inside the mask. If the mask is culled, then the children
        // inside the mask are also culled. In that situation, we pass an invalid rect to allow callees
        // to avoid some processing.
        ClipRect.XMin = 0;
        ClipRect.YMin = 0;
        ClipRect.Width = 0;
        ClipRect.Height = 0;
        bValidRect = false;
    }

    ClipRect.XMin += Padding.Left;
    ClipRect.YMin += Padding.Bottom;
    ClipRect.Width = FMath::Max(0.0f, ClipRect.Width - Padding.Right);
    ClipRect.Height = FMath::Max(0.0f, ClipRect.Height - Padding.Top);

    FRect ClipSoftnessRect;
    const float SoftnessLeft = FMath::Max(Softness.Left, 0.0f);
    const float SoftnessTop = FMath::Max(Softness.Top, 0.0f);
    const float SoftnessRight = FMath::Max(Softness.Right, 0.0f);
    const float SoftnessBottom = FMath::Max(Softness.Bottom, 0.0f);
    ClipSoftnessRect.XMin = FMath::Clamp(ClipRect.XMin + SoftnessLeft, ClipRect.XMin, ClipRect.GetXMax());
    ClipSoftnessRect.YMin = FMath::Clamp(ClipRect.YMin + SoftnessBottom, ClipRect.YMin, ClipRect.GetYMax());
    ClipSoftnessRect.Width = FMath::Clamp(ClipRect.Width - SoftnessRight - SoftnessLeft, 0.0f, ClipRect.Width);
    ClipSoftnessRect.Height = FMath::Clamp(ClipRect.Height - SoftnessTop - SoftnessBottom, 0.0f, ClipRect.Height);
    
    if (ClipRect != LastClipRectCanvasSpace || ClipSoftnessRect != LastClipSoftnessRectCanvasSpace)
    {
    	for (const auto& ClipTargetObj : ClipTargets)
    	{
            const auto ClipTarget = Cast<IClippableInterface>(ClipTargetObj);
            if (ClipTarget)
            {
                ClipTarget->SetClipRect(ClipRect, bValidRect, ClipSoftnessRect);
            }
    	}

        for (const auto& MaskableTarget : MaskableTargets)
        {
            const auto ClipTarget = Cast<IClippableInterface>(MaskableTarget);
            if (ClipTarget)
            {
                ClipTarget->SetClipRect(ClipRect, bValidRect, ClipSoftnessRect);
                ClipTarget->Cull(ClipRect, bValidRect);
            }
        }
    }
    else if (bForceClip)
    {
        for (const auto& ClipTargetObj : ClipTargets)
        {
            const auto ClipTarget = Cast<IClippableInterface>(ClipTargetObj);
            if (ClipTarget)
            {
                ClipTarget->SetClipRect(ClipRect, bValidRect, ClipSoftnessRect);
            }
        }

        for (const auto& MaskableTarget : MaskableTargets)
        {
            const auto ClipTarget = Cast<IClippableInterface>(MaskableTarget);
            if (ClipTarget)
            {
                ClipTarget->SetClipRect(ClipRect, bValidRect, ClipSoftnessRect);

                // TODO 看下是否需要HasMoved优化   Rect改变的也要重新设置
            	// if (maskableTarget.canvasRenderer.hasMoved)
                //{
				ClipTarget->Cull(ClipRect, bValidRect);
                //}
            }
        }
    }
    else
    {
        for (const auto& MaskableTarget : MaskableTargets)
        {
            const auto ClipTarget = Cast<IClippableInterface>(MaskableTarget);
            if (ClipTarget)
            {	
            	// TODO 看下是否需要HasMoved优化
                //if (maskableTarget.canvasRenderer.hasMoved)
                //{
                ClipTarget->Cull(ClipRect, bValidRect);
                //}                
            }
        }
    }

    LastClipSoftnessRectCanvasSpace = ClipSoftnessRect;
    LastClipRectCanvasSpace = ClipRect;
    bForceClip = false;
}

void URectMask2DSubComponent::AddClippable(IClippableInterface* Clippable)
{
    if (Clippable == nullptr)
        return;
	
    bShouldRecalculateClipRects = true;
    IMaskableGraphicElementInterface* Maskable = Cast<IMaskableGraphicElementInterface>(Clippable);

    if (Maskable == nullptr)
    {
        ClipTargets.Add(Cast<UObject>(Clippable));
    }
    else
    {
        MaskableTargets.Add(Cast<UObject>(Maskable));
    }

    bForceClip = true;
}

void URectMask2DSubComponent::RemoveClippable(IClippableInterface* Clippable)
{
    if (Clippable == nullptr)
        return;

    bShouldRecalculateClipRects = true;
    Clippable->SetClipRect(FRect(), false, FRect());

    IMaskableGraphicElementInterface* Maskable = Cast<IMaskableGraphicElementInterface>(Clippable);

    if (Maskable == nullptr)
    {
        ClipTargets.Remove(Cast<UObject>(Clippable));
    }
    else
    {
        MaskableTargets.Remove(Cast<UObject>(Maskable));
    }

    bForceClip = true;
}

/////////////////////////////////////////////////////
