#include "Core/Layout/CanvasScalerSubComponent.h"
#include "UGUISubsystem.h"
#include "UGUIWorldSubsystem.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Math/UnitConversion.h"
#include "HAL/PlatformApplicationMisc.h"

/////////////////////////////////////////////////////
// UCanvasScalerSubComponent

static constexpr float LogBase = 2;

UCanvasScalerSubComponent::UCanvasScalerSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIScaleMode = ECanvasScalerScaleMode::ScaleMode_ConstantPixelSize;

	ReferencePixelsPerUnit = 1;
    ScaleFactor = 1;

    ReferenceResolution = FVector2D(800, 600);

    ScreenMatchMode = ECanvasScalerScreenMatchMode::ScreenMatchMode_MatchWidthOrHeight;
    MatchWidthOrHeight = 0;

    PhysicalUnit = ECanvasScalerUnit::Unit_Points;

    FallbackScreenDPI = 96;
    DefaultSpriteDPI = 96;
    DynamicPixelsPerUnit = 1;

    CacheCanvas = nullptr;
	
	PrevScaleFactor = 1;
	PrevReferencePixelsPerUnit = 1;
}

void UCanvasScalerSubComponent::OnEnable()
{
	Super::OnEnable();

    CacheCanvas = Cast<UCanvasSubComponent>(GetComponent(UCanvasSubComponent::StaticClass()));
	Handle();

    if (IsValid(CacheCanvas))
    {
        CacheCanvas->OnRenderModeChanged.AddUObject(this, &UCanvasScalerSubComponent::Handle);
        CacheCanvas->OnParentCanvasChanged.AddUObject(this, &UCanvasScalerSubComponent::Handle);
    }
	
    const auto World = GetWorld();
    if (IsValid(World))
    {
        const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
        if (IsValid(UIWorldSubsystem))
        {
            ViewportResizeDelegateHandle = UIWorldSubsystem->ViewportResizedEvent.AddUObject(this, &UCanvasScalerSubComponent::OnViewportResized);
        }
    }

#if WITH_EDITOR
	EditorViewportResizeDelegateHandle = UUGUISubsystem::OnEditorViewportInfoChanged.AddUObject(this, &UCanvasScalerSubComponent::OnEditorViewportResized);
#endif
}

void UCanvasScalerSubComponent::OnDisable()
{
    if (ViewportResizeDelegateHandle.IsValid())
    {
        const auto World = GetWorld();
        if (IsValid(World))
        {
            const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
            if (IsValid(UIWorldSubsystem))
            {
                UIWorldSubsystem->ViewportResizedEvent.Remove(ViewportResizeDelegateHandle);
            }
        }

        ViewportResizeDelegateHandle.Reset();
    }

#if WITH_EDITOR
	UUGUISubsystem::OnEditorViewportInfoChanged.Remove(EditorViewportResizeDelegateHandle);
#endif
	
	SetCanvasScaleFactor(1);
	SetCanvasReferencePixelsPerUnit(1);

    CacheCanvas = nullptr;
	
	Super::OnDisable();
}

#if WITH_EDITORONLY_DATA

void UCanvasScalerSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    Handle();
}

#endif

void UCanvasScalerSubComponent::Handle()
{
    if (!IsValid(CacheCanvas) || !CacheCanvas->IsRootCanvas())
        return;

#if WITH_EDITOR
	ECanvasRenderMode CanvasRenderMode = CacheCanvas->GetRenderMode();
#else
	const ECanvasRenderMode CanvasRenderMode = CacheCanvas->GetRenderMode();
#endif
	
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(GetWorld());
		if (WorldViewportInfo)
		{
			CanvasRenderMode = WorldViewportInfo->GetRenderMode();
		}
	}
#endif

    if (CanvasRenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace ||
    	CanvasRenderMode== ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree)
    {
        HandleWorldCanvas();
        return;
    }

    switch (UIScaleMode)
    {
    case ECanvasScalerScaleMode::ScaleMode_ConstantPixelSize:
    	HandleConstantPixelSize();
    	break;
    case ECanvasScalerScaleMode::ScaleMode_ScaleWithScreenSize:
    	HandleScaleWithScreenSize();
    	break;
    case ECanvasScalerScaleMode::ScaleMode_ConstantPhysicalSize:
    	HandleConstantPhysicalSize();
    	break;
    default:
        break;
    }
}

void UCanvasScalerSubComponent::HandleWorldCanvas()
{
	SetCanvasScaleFactor(DynamicPixelsPerUnit);
	SetCanvasReferencePixelsPerUnit(ReferencePixelsPerUnit);
}

void UCanvasScalerSubComponent::HandleConstantPixelSize()
{
	SetCanvasScaleFactor(ScaleFactor);
	SetCanvasReferencePixelsPerUnit(ReferencePixelsPerUnit);
}

void UCanvasScalerSubComponent::HandleScaleWithScreenSize()
{	
    const auto& ScreenSize = UUGUISubsystem::GetViewportSize(CacheCanvas);

    float TargetScaleFactor = 0;
    switch (ScreenMatchMode)
    {
    case ECanvasScalerScreenMatchMode::ScreenMatchMode_MatchWidthOrHeight:
    {
        // We take the log of the relative width and height before taking the average.
        // Then we transform it back in the original space.
        // the reason to transform in and out of logarithmic space is to have better behavior.
        // If one axis has twice resolution and the other has half, it should even out if widthOrHeight value is at 0.5.
        // In normal space the average would be (0.5 + 2) / 2 = 1.25
        // In logarithmic space the average is (-1 + 1) / 2 = 0
        const float LogWidth = FMath::LogX(LogBase, ScreenSize.X / ReferenceResolution.X);
        const float LogHeight = FMath::LogX(LogBase, ScreenSize.Y / ReferenceResolution.Y);
        const float LogWeightedAverage = FMath::Lerp(LogWidth, LogHeight, MatchWidthOrHeight);
        TargetScaleFactor = FMath::Pow(LogBase, LogWeightedAverage);
        break;
    }
    case ECanvasScalerScreenMatchMode::ScreenMatchMode_Expand:
    {
        TargetScaleFactor = FMath::Min(ScreenSize.X / ReferenceResolution.X, ScreenSize.Y / ReferenceResolution.Y);
        break;
    }
    case ECanvasScalerScreenMatchMode::ScreenMatchMode_Shrink:
    {
        TargetScaleFactor = FMath::Max(ScreenSize.X / ReferenceResolution.X, ScreenSize.Y / ReferenceResolution.Y);
        break;
    }
    default:
        break;
    }

    SetCanvasScaleFactor(TargetScaleFactor);
    SetCanvasReferencePixelsPerUnit(ReferencePixelsPerUnit);
}

void UCanvasScalerSubComponent::HandleConstantPhysicalSize()
{
    int32 CurrentDpi = 0;
	FPlatformApplicationMisc::GetPhysicalScreenDensity(CurrentDpi);
    const float DPI = (CurrentDpi == 0 ? FallbackScreenDPI : CurrentDpi);
    float TargetDPI = 1;

	switch (PhysicalUnit)
    {
    case ECanvasScalerUnit::Unit_Centimeters:
        TargetDPI = 2.54f;
		break;
    case ECanvasScalerUnit::Unit_Millimeters:
        TargetDPI = 25.4f;
		break;
    case ECanvasScalerUnit::Unit_Inches:
        TargetDPI = 1;
		break;
    case ECanvasScalerUnit::Unit_Points:
        TargetDPI = 72;
		break;
    case ECanvasScalerUnit::Unit_Picas:
        TargetDPI = 6;
		break;
	default:
        break;
    }

    SetCanvasScaleFactor(DPI / TargetDPI);
    SetCanvasReferencePixelsPerUnit(ReferencePixelsPerUnit * TargetDPI / DefaultSpriteDPI);
}

void UCanvasScalerSubComponent::SetUIScaleMode(ECanvasScalerScaleMode InUIScaleMode)
{
	if (UIScaleMode != InUIScaleMode)
	{
		UIScaleMode = InUIScaleMode;
		Handle();
	}
}

void UCanvasScalerSubComponent::SetReferencePixelsPerUnit(float InReferencePixelsPerUnit)
{
    if (ReferencePixelsPerUnit != InReferencePixelsPerUnit)
    {
        ReferencePixelsPerUnit = InReferencePixelsPerUnit;
        Handle();
    }
}

void UCanvasScalerSubComponent::SetScaleFactor(float InScaleFactor)
{
	if (ScaleFactor == InScaleFactor)
	{
        ScaleFactor = FMath::Max(0.01f, InScaleFactor);
        Handle();
	}
}

void UCanvasScalerSubComponent::SetReferenceResolution(FVector2D InReferenceResolution)
{
    if (ReferenceResolution == InReferenceResolution)
    {
        constexpr float MinimumResolution = 0.00001f;
        ReferenceResolution = FMath::Max(FVector2D(MinimumResolution, MinimumResolution), ReferenceResolution);
        Handle();
    }
}

void UCanvasScalerSubComponent::SetScreenMatchMode(ECanvasScalerScreenMatchMode InScreenMatchMode)
{
	if (ScreenMatchMode != InScreenMatchMode)
	{
        ScreenMatchMode = InScreenMatchMode;
        Handle();
	}
}

void UCanvasScalerSubComponent::SetMatchWidthOrHeight(float InMatchWidthOrHeight)
{
	if (MatchWidthOrHeight != InMatchWidthOrHeight)
	{
        MatchWidthOrHeight = InMatchWidthOrHeight;
        Handle();
	}
}

void UCanvasScalerSubComponent::SetPhysicalUnit(ECanvasScalerUnit InPhysicalUnit)
{
    if (PhysicalUnit != InPhysicalUnit)
    {
        PhysicalUnit = InPhysicalUnit;
        Handle();
    }
}

void UCanvasScalerSubComponent::SetFallbackScreenDPI(float InFallbackScreenDPI)
{
    if (FallbackScreenDPI != InFallbackScreenDPI)
    {
        FallbackScreenDPI = InFallbackScreenDPI;
        Handle();
    }
}

void UCanvasScalerSubComponent::SetDefaultSpriteDPI(float InDefaultSpriteDPI)
{
    if (DefaultSpriteDPI != InDefaultSpriteDPI)
    {
        DefaultSpriteDPI = InDefaultSpriteDPI;
        Handle();
    }
}

void UCanvasScalerSubComponent::SetDynamicPixelsPerUnit(float InDynamicPixelsPerUnit)
{
    if (DynamicPixelsPerUnit != InDynamicPixelsPerUnit)
    {
        DynamicPixelsPerUnit = InDynamicPixelsPerUnit;
        Handle();
    }
}

void UCanvasScalerSubComponent::SetCanvasScaleFactor(float InScaleFactor)
{
	if (InScaleFactor == PrevScaleFactor)
		return;

	if (IsValid(CacheCanvas))
	{
		CacheCanvas->SetScaleFactor(InScaleFactor);
	}
	
	PrevScaleFactor = InScaleFactor;
}

void UCanvasScalerSubComponent::SetCanvasReferencePixelsPerUnit(float InReferencePixelsPerUnit)
{
	if (InReferencePixelsPerUnit == PrevReferencePixelsPerUnit)
		return;

	if (IsValid(CacheCanvas))
	{
		CacheCanvas->SetReferencePixelsPerUnit(InReferencePixelsPerUnit);
	}
	
	PrevReferencePixelsPerUnit = InReferencePixelsPerUnit;
}

void UCanvasScalerSubComponent::OnViewportResized()
{
    Handle();
}

#if WITH_EDITOR

void UCanvasScalerSubComponent::OnEditorViewportResized(const UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		Handle();
	}
}

#endif

/////////////////////////////////////////////////////
