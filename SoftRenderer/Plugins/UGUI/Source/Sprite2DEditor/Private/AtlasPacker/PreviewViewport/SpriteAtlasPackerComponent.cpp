#include "SpriteAtlasPackerComponent.h"
#include "AtlasPacker/SpriteAtlasPackerPrivate.h"
#include "AtlasPacker/SSpriteAtlasPackerSpriteListItem.h"
#include "Materials/MaterialInstanceDynamic.h"

/////////////////////////////////////////////////////
// USpriteAtlasPackerComponent

USpriteAtlasPackerComponent::USpriteAtlasPackerComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer), RawImageComponent(nullptr), ViewportInfo(nullptr)
{
	GridMaterial = nullptr;
}

void USpriteAtlasPackerComponent::Awake()
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
            const auto& Object = Cast<UUGUIEditorViewportInfo>(World->ExtraReferencedObjects[Index]);
            if (Object)
            {
                ViewportInfo = Object;
                break;
            }
        }
    }

    if (FSpriteAtlasPacker::Get().SelectedSprite.IsValid())
    {
        OnSpriteListSelectionChanged(FSpriteAtlasPacker::Get().SelectedSprite.Pin()->Width,
            FSpriteAtlasPacker::Get().SelectedSprite.Pin()->Height, FSpriteAtlasPacker::Get().SelectedSprite.Pin()->Texture);
    }
    
    FSpriteAtlasPacker::Get().GetEvents().OnSpriteListSelectionChangedEvent.AddUObject(this, &USpriteAtlasPackerComponent::OnSpriteListSelectionChanged);
}

void USpriteAtlasPackerComponent::OnSpriteListSelectionChanged(int32 Width, int32 Height, UTexture2D* Texture2D)
{
    SetHidePrimitive(Texture2D == nullptr);
    RawImageComponent->SetHidePrimitive(Texture2D == nullptr);
    RawImageComponent->SetTexture(Texture2D);
    SetSizeDelta(FVector2D(Width, Height));
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
