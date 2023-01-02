#include "Core/Layout/SafeZoneComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Widgets/SViewport.h"
#include "UGUISettings.h"

/////////////////////////////////////////////////////
// USafeZoneComponent

FMargin USafeZoneComponent::GlobalSafeMargin = FMargin(0, 0, 0, 0);
bool USafeZoneComponent::bUseGlobalSafeMargin = false;;

USafeZoneComponent::USafeZoneComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bChildControlWidth = true;
	bChildControlHeight = true;
	bChildForceExpandWidth = true;
	bChildForceExpandHeight = true;

	bPadLeft = true;
	bPadRight = true;
	bPadTop = true;
	bPadBottom = true;

	SafeAreaScale = FUIMargin(1, 1, 1, 1);

	Canvas = nullptr;
}

void USafeZoneComponent::UseGlobalSafeMargin(bool bUse)
{
	bUseGlobalSafeMargin = bUse;
}

void USafeZoneComponent::UpdateGlobalSafeMargin(FMargin InGlobalSafeMargin)
{
	GlobalSafeMargin = InGlobalSafeMargin;
}

#if WITH_EDITORONLY_DATA

void USafeZoneComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void USafeZoneComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (!HasAnyFlags(RF_ArchetypeObject))
	{
		UpdateSafeMargin();
	}
}

#endif

void USafeZoneComponent::OnEnable()
{
	Super::OnEnable();

	UpdateSafeMargin();
	OnSafeFrameChangedHandle = FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &USafeZoneComponent::UpdateSafeMargin);
}

void USafeZoneComponent::OnDisable()
{
	FCoreDelegates::OnSafeFrameChangedEvent.Remove(OnSafeFrameChangedHandle);
	
	Super::OnDisable();
}

void USafeZoneComponent::OnDestroy()
{
	SetRootCanvas(nullptr);
	
	Super::OnDestroy();
}

void USafeZoneComponent::OnCanvasHierarchyChanged()
{
	Super::OnCanvasHierarchyChanged();

	if (SetRootCanvas(GetRootCanvas()))
	{
		UpdateSafeMargin();
	}
}

void USafeZoneComponent::OnTransformParentChanged()
{
	Super::OnTransformParentChanged();

	if (SetRootCanvas(GetRootCanvas()))
	{
		UpdateSafeMargin();
	}
}

UCanvasSubComponent* USafeZoneComponent::GetRootCanvas() const
{
	if (IsValid(OwnerCanvas))
	{
		return OwnerCanvas->GetRootCanvas();
	}
	return nullptr;
}

bool USafeZoneComponent::SetRootCanvas(UCanvasSubComponent* NewRootCanvas)
{
	if (Canvas != NewRootCanvas)
	{
		if (IsValid(Canvas))
		{
			Canvas->OnRenderModeChanged.Remove(OnRenderModeChangedHandle);
			Canvas->OnScaleFactorChanged.Remove(OnScaleFactorChangedHandle);
		}

		Canvas = NewRootCanvas;

		if (IsValid(Canvas))
		{
			OnRenderModeChangedHandle = Canvas->OnRenderModeChanged.AddUObject(this, &USafeZoneComponent::UpdateSafeMargin);
			OnScaleFactorChangedHandle = Canvas->OnScaleFactorChanged.AddUObject(this, &USafeZoneComponent::UpdateSafeMargin);
		}
		
		return true;
	}
	
	return false;
}

void USafeZoneComponent::UpdateSafeMargin()
{
	check(IsInGameThread());
	
	if (!FSlateApplication::IsInitialized())
		return;

#if WITH_EDITOR
	const auto World = GetWorld();
	if (!IsValid(World) || !World->IsGameWorld())
	{
		return;
	}
#endif
	
	if (!IsValid(Canvas))
	{
		SetRootCanvas(GetRootCanvas());
	}

	if (!IsValid(Canvas) || Canvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
	{
		SetPadding(ExtraPadding);
		return;
	}
	
	FMargin SafeMargin;

	if (bUseGlobalSafeMargin)
	{
		SafeMargin = GlobalSafeMargin;
	}
	else
	{
		// Need to get owning viewport not display 
		// use pixel values (same as custom safe zone above)
		TSharedPtr<SViewport> GameViewport = FSlateApplication::Get().GetGameViewport();
		if (!GameViewport.IsValid())
			return;

		const TSharedPtr<ISlateViewport> ViewportInterface = GameViewport->GetViewportInterface().Pin();
		if (!ViewportInterface.IsValid())
			return;

		const FIntPoint ViewportSize = ViewportInterface->GetSize();
		FSlateApplication::Get().GetSafeZoneSize(SafeMargin, ViewportSize);
	}
	
	SafeMargin = FMargin(bPadLeft ? SafeMargin.Left : 0.0f, bPadTop ? SafeMargin.Top : 0.0f, bPadRight ? SafeMargin.Right : 0.0f, bPadBottom ? SafeMargin.Bottom : 0.0f);

	if (UUGUISettings::Get()->bSafeZoneHorizontalAlign)
	{
		float MaxMarginInHorizontal = FMath::Max(SafeMargin.Left, SafeMargin.Right);
		SafeMargin.Left = MaxMarginInHorizontal;
		SafeMargin.Right = MaxMarginInHorizontal;
	}
	if (UUGUISettings::Get()->bSafeZoneVerticalAlign)
	{
		float MaxMarginInVertical = FMath::Max(SafeMargin.Top, SafeMargin.Bottom);
		SafeMargin.Top = MaxMarginInVertical;
		SafeMargin.Bottom = MaxMarginInVertical;
	}

	const float InvScale = 1.0f / Canvas->GetScaleFactor();
	
	const FUIMargin ScaledSafeMargin(
		FMath::RoundToFloat(SafeMargin.Left * InvScale),
		FMath::RoundToFloat(SafeMargin.Top * InvScale),
		FMath::RoundToFloat(SafeMargin.Right * InvScale),
		FMath::RoundToFloat(SafeMargin.Bottom * InvScale));
	
	SetPadding(ExtraPadding + ScaledSafeMargin * SafeAreaScale);
}

/////////////////////////////////////////////////////
