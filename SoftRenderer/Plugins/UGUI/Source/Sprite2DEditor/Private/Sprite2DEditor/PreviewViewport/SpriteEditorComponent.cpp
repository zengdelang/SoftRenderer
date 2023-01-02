#include "SpriteEditorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

/////////////////////////////////////////////////////
// USpriteEditorComponent

USpriteEditorComponent::USpriteEditorComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), RawImageComponent(nullptr), ViewportInfo(nullptr)
{
	GridMaterial = nullptr;
}

void USpriteEditorComponent::Awake()
{
    Super::Awake();

	GridMaterial = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/UGUI/DefaultResources/Editor/Materials/CheckerboardUIMaterial.CheckerboardUIMaterial")), this);
	if (GridMaterial)
	{
		GridMaterial->SetVectorParameterValue(TEXT("Size"), FLinearColor(2048, 2048, 0, 0));
	}

    SetHidePrimitive(true);
    SetMaterial(GridMaterial);
    SetSizeDelta(FVector2D(2048, 2048));
    
    RawImageComponent = NewObject<URawImageComponent>(this);
    RawImageComponent->RegisterComponent();
    RawImageComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    RawImageComponent->SetLocalTransform(FTransform::Identity);
    RawImageComponent->SetSizeDelta(FVector2D(2048, 2048));
    RawImageComponent->SetHidePrimitive(true);
    RawImageComponent->AwakeFromLoad();

    if (!IsValid(ViewportInfo))
    {
        UWorld* World = GetWorld();
        check(World);

        for (int32 Index = 0, Count = World->ExtraReferencedObjects.Num(); Index < Count; ++Index)
        {
            const auto& Object = Cast<USprite2DEditorViewportInfo>(World->ExtraReferencedObjects[Index]);
            if (Object)
            {
                ViewportInfo = Object;
                break;
            }
        }
    }

    const USprite2D* Sprite = ViewportInfo->Sprite2DEditor.Pin()->GetSprite2D();
    if (IsValid(Sprite))
    {
        const int32 Width = Sprite->GetSpriteSize().X;
        const int32 Height = Sprite->GetSpriteSize().Y;
        
        UTexture2D* Texture2D = Sprite->GetSourceTexture();
        
        SetHidePrimitive(Texture2D == nullptr);
        SetSizeDelta(FVector2D(Width, Height));
        
        RawImageComponent->SetHidePrimitive(Texture2D == nullptr);
        RawImageComponent->SetTexture(Texture2D);
        RawImageComponent->SetSizeDelta(FVector2D(Width, Height));

        if (GridMaterial)
        {
            GridMaterial->SetVectorParameterValue(TEXT("Size"), FLinearColor(Width, Height, 0, 0));
        }
	
        if (IsValid(ViewportInfo))
        {
            if (ViewportInfo->ViewportClient.IsValid())
            {
                ViewportInfo->ViewportClient.Pin()->SetTargetViewSize(FVector2D(Width, Height));
                ViewportInfo->ViewportClient.Pin()->SetZoomOffset(FVector2D(25, 25));
                ViewportInfo->ViewportClient.Pin()->SetZoomToFit(true, false);
            }
        }
    }
}

void USpriteEditorComponent::UpdateSprite()
{
    const USprite2D* Sprite = ViewportInfo->Sprite2DEditor.Pin()->GetSprite2D();
    UTexture2D* Texture2D = Sprite->GetSourceTexture();

    if(!IsValid(Texture2D))
    {
        return;
    }
    
    const int32 Width = Texture2D->GetSizeX();
    const int32 Height = Texture2D->GetSizeY();
    
    SetHidePrimitive(Texture2D == nullptr);
    SetSizeDelta(FVector2D(Width, Height));
    
    RawImageComponent->SetHidePrimitive(Texture2D == nullptr);
    RawImageComponent->SetTexture(Texture2D);
    RawImageComponent->SetSizeDelta(FVector2D(Width, Height));

    if (GridMaterial)
    {
        GridMaterial->SetVectorParameterValue(TEXT("Size"), FLinearColor(Width, Height, 0, 0));
    }
	
    if (IsValid(ViewportInfo))
    {
        if (ViewportInfo->ViewportClient.IsValid())
        {
            ViewportInfo->ViewportClient.Pin()->SetTargetViewSize(FVector2D(Width, Height));
            ViewportInfo->ViewportClient.Pin()->SetZoomOffset(FVector2D(25, 25));
            ViewportInfo->ViewportClient.Pin()->SetZoomToFit(true, false);
        }
    }
}

/////////////////////////////////////////////////////
