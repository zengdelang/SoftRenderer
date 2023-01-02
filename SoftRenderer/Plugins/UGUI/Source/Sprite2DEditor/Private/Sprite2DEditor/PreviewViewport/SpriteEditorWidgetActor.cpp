#include "SpriteEditorWidgetActor.h"
#include "Core/Layout/CanvasScalerSubComponent.h"
#include "Core/Layout/VerticalLayoutGroupComponent.h"

/////////////////////////////////////////////////////
// ASpriteEditorWidgetActor

ASpriteEditorWidgetActor::ASpriteEditorWidgetActor(const FObjectInitializer& ObjectInitializer)
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

    SpriteEditorComponent = CreateEditorOnlyDefaultSubobject<USpriteEditorComponent>(TEXT("Sprite2DEditor"));
    if (SpriteEditorComponent)
    {
        SpriteEditorComponent->SetupAttachment(RootComponent);
    }
}

void ASpriteEditorWidgetActor::UpdateActor() const
{
    if (SpriteEditorComponent)
    {
        SpriteEditorComponent->UpdateSprite();
    }
}

/////////////////////////////////////////////////////
