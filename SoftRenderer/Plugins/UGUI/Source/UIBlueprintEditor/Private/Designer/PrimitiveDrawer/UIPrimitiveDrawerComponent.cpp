#include "Designer/PrimitiveDrawer/UIPrimitiveDrawerComponent.h"
#include "Designer/PrimitiveDrawer/UIPrimitiveDrawerProxyComponent.h"
#include "Core/Layout/RectTransformPreviewComponent.h"
#include "UGUISubsystem.h"

/////////////////////////////////////////////////////
// UUIPrimitiveDrawerComponent

UUIPrimitiveDrawerComponent::UUIPrimitiveDrawerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimitiveDrawerProxyComponent = nullptr;
	bRaycastTarget = false;
}

void UUIPrimitiveDrawerComponent::Awake()
{
	Super::Awake();
	
	if ( IsValid(GetOwner()))
	{
		PrimitiveDrawerProxyComponent = NewObject<UUIPrimitiveDrawerProxyComponent>(GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
		PrimitiveDrawerProxyComponent->bIsEditorOnly = true;
#endif
		PrimitiveDrawerProxyComponent->bAutoActivate = false;
		PrimitiveDrawerProxyComponent->SetVisibility(false);
		PrimitiveDrawerProxyComponent->SetHiddenInGame(true);
		PrimitiveDrawerProxyComponent->SetupAttachment(this);
		PrimitiveDrawerProxyComponent->SetRelativeTransform(FTransform::Identity);
		PrimitiveDrawerProxyComponent->SetRelativeRotation(FRotator(-90, 90, 0));
		PrimitiveDrawerProxyComponent->RegisterComponent();

		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(true);
			CanvasRendererComp->SetCustomRenderProxyComponent(PrimitiveDrawerProxyComponent);
		}
	}

	URectTransformPreviewComponent::OnEditorUIDesignerInfoChanged.AddUObject(this, &UUIPrimitiveDrawerComponent::OnEditorUIDesignerInfoChanged);
	
#if WITH_EDITOR
	 UUGUISubsystem::OnEditorViewportInfoChanged.AddUObject(this, &UUIPrimitiveDrawerComponent::OnEditorUIDesignerInfoChanged);
#endif
}

void UUIPrimitiveDrawerComponent::OnDisable()
{
	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetUseCustomRenderProxy(false);
	}
	
	Super::OnDisable();
}

void UUIPrimitiveDrawerComponent::OnEditorUIDesignerInfoChanged(const UWorld* InWorld) const
{
	if (InWorld == GetWorld())
	{
		if (IsValid(GetWorld()))
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UUIPrimitiveDrawerComponent::MarkProxyRenderStateDirty);
		}
	}
}

void UUIPrimitiveDrawerComponent::MarkProxyRenderStateDirty() const
{
	if (IsValid(PrimitiveDrawerProxyComponent))
	{
		PrimitiveDrawerProxyComponent->MarkRenderStateDirty();
	}
}

void UUIPrimitiveDrawerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(PrimitiveDrawerProxyComponent))
	{
		PrimitiveDrawerProxyComponent->DestroyComponent();
		PrimitiveDrawerProxyComponent = nullptr;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetCustomRenderProxyComponent(nullptr);
	}
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

/////////////////////////////////////////////////////
