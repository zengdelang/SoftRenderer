#include "Designer/BackgroundImageActor.h"
#include "Core/Layout/CanvasScalerSubComponent.h"

/////////////////////////////////////////////////////
// ABackgroundImageActor

ABackgroundImageActor::ABackgroundImageActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UIRoot = CreateDefaultSubobject<URectTransformComponent>(TEXT("UIRoot"));
	RootComponent = UIRoot;

	UIRoot->SetSizeDelta(FVector2D(1920, 1080));
	UIRoot->AddSubComponentByClass(UCanvasSubComponent::StaticClass());

	UCanvasSubComponent* CanvasComp = Cast<UCanvasSubComponent>(UIRoot->GetComponent(UCanvasSubComponent::StaticClass(), true));
	if (IsValid(CanvasComp))
	{
		CanvasComp->SetSortingOrder(-1e8);
		CanvasComp->SetRenderMode(ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree);
		CanvasComp->bForceUseSelfRenderMode = true;
	}

	BackgroundImageComponent = CreateEditorOnlyDefaultSubobject<UBackgroundImageComponent>(TEXT("BackgroundImage"));
	if (BackgroundImageComponent)
	{
		BackgroundImageComponent->SetupAttachment(RootComponent);
		BackgroundImageComponent->SetHidePrimitive(true);
		BackgroundImageComponent->SetAllDirty();
	}
}

/////////////////////////////////////////////////////
