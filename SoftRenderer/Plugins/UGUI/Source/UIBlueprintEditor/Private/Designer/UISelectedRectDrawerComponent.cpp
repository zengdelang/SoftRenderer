#include "Designer/UISelectedRectDrawerComponent.h"

/////////////////////////////////////////////////////
// UUISelectedRectLineComponent

UUISelectedRectDrawerComponent::UUISelectedRectDrawerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRaycastTarget = false;
	SetAnchorAndOffset(FVector2D::ZeroVector, FVector2D(1, 1), FVector2D::ZeroVector, FVector2D::ZeroVector);
}

void UUISelectedRectDrawerComponent::Awake()
{
	Super::Awake();
	
	if ( IsValid(GetOwner()))
	{
		SelectedRectDrawerProxyComponent = NewObject<UUISelectedRectDrawerProxyComponent>(GetOwner(), NAME_None, RF_Transient);
#if WITH_EDITOR
		SelectedRectDrawerProxyComponent->bIsEditorOnly = true;
#endif
		SelectedRectDrawerProxyComponent->bAutoActivate = false;
		SelectedRectDrawerProxyComponent->SetVisibility(false);
		SelectedRectDrawerProxyComponent->SetHiddenInGame(true);
		SelectedRectDrawerProxyComponent->SetupAttachment(this);
		SelectedRectDrawerProxyComponent->SetRelativeTransform(FTransform::Identity);
		SelectedRectDrawerProxyComponent->SetRelativeRotation(FRotator(-90, 90, 0));
		SelectedRectDrawerProxyComponent->RegisterComponent();

		const auto CanvasRendererComp = GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetUseCustomRenderProxy(true);
			CanvasRendererComp->SetCustomRenderProxyComponent(SelectedRectDrawerProxyComponent);
		}
	}
}

void UUISelectedRectDrawerComponent::OnDisable()
{
	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetUseCustomRenderProxy(false);
	}
	
	Super::OnDisable();
}

void UUISelectedRectDrawerComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(SelectedRectDrawerProxyComponent))
	{
		SelectedRectDrawerProxyComponent->DestroyComponent();
		SelectedRectDrawerProxyComponent = nullptr;
	}

	const auto CanvasRendererComp = GetCanvasRenderer();
	if (IsValid(CanvasRendererComp))
	{
		CanvasRendererComp->SetCustomRenderProxyComponent(nullptr);
	}
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

/////////////////////////////////////////////////////
