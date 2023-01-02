#include "Designer/DesignerWidgetActor.h"
#include "Core/Layout/CanvasScalerSubComponent.h"

/////////////////////////////////////////////////////
// ADesignerWidgetActor

ADesignerWidgetActor::ADesignerWidgetActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIRoot = CreateDefaultSubobject<URectTransformComponent>(TEXT("UIRoot"));
	RootComponent = UIRoot;
	
	UIRoot->SetSizeDelta(FVector2D(1920, 1080));
	UIRoot->AddSubComponentByClass(UCanvasSubComponent::StaticClass());
	
	UCanvasSubComponent* CanvasComp = Cast<UCanvasSubComponent>(UIRoot->GetComponent(UCanvasSubComponent::StaticClass(), true));
	if (IsValid(CanvasComp))
	{
		CanvasComp->SetSortingOrder(1e8);
		CanvasComp->SetRenderMode(ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree);
		CanvasComp->bForceUseSelfRenderMode = true;
	}

	DrawerComponent = CreateEditorOnlyDefaultSubobject<UUIPrimitiveDrawerComponent>(TEXT("UIPrimitiveDrawer"));
	if (DrawerComponent)
	{
		DrawerComponent->SetupAttachment(RootComponent);
	}

	RaycastRegionComponent = CreateEditorOnlyDefaultSubobject<UUIRaycastRegionComponent>(TEXT("RaycastRegion"));
	if (RaycastRegionComponent)
	{
		RaycastRegionComponent->SetupAttachment(RootComponent);
	}
}

/////////////////////////////////////////////////////
