#include "SpriteAtlasVisualizerComponent.h"
#include "SpriteMergeSubsystem.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "Materials/MaterialInstanceDynamic.h"

/////////////////////////////////////////////////////
// USpriteAtlasVisualizerComponent

USpriteAtlasVisualizerComponent::USpriteAtlasVisualizerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickInEditor = true;
	PrimaryComponentTick.bHighPriority = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bCanEverTick = true;

	bChildControlWidth = true;
	bChildControlHeight = true;

	Spacing = 5;

	ViewportInfo = nullptr;
	GridMaterial = nullptr;
}

void USpriteAtlasVisualizerComponent::Awake()
{
	Super::Awake();

	GridMaterial = UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr, TEXT("/UGUI/DefaultResources/Editor/Materials/CheckerboardUIMaterial.CheckerboardUIMaterial")), this);
	if (GridMaterial)
	{
		GridMaterial->SetVectorParameterValue(TEXT("Size"), FLinearColor(USpriteMergeSubsystem::AtlasWidth, USpriteMergeSubsystem::AtlasHeight, 0, 0));
	}
}

void USpriteAtlasVisualizerComponent::TickComponent(float DeltaTime, ELevelTick Tick,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

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
    
    if (GEngine)
    {
        const auto SpriteMergeSubsystem = GEngine->GetEngineSubsystem<USpriteMergeSubsystem>();
        if (SpriteMergeSubsystem)
        {
            auto& SpriteTextures = SpriteMergeSubsystem->GetSpriteTextures();
            if ((SpriteTextures.Num() > RawImageComponents.Num()) || (bFirstShow && RawImageComponents.Num() == 0))
            {
                int32 ExtraCount = 0;
                if (bFirstShow && RawImageComponents.Num() == 0 && SpriteTextures.Num() == 0)
                {
                    ExtraCount = 1;
                }
                
                for (int32 Index = 0, Count = SpriteTextures.Num() - RawImageComponents.Num() + ExtraCount; Index < Count; ++Index)
                {
                    URawImageComponent* ParentRawImageComponent= NewObject<URawImageComponent>(this);
                    ParentRawImageComponent->RegisterComponent();
                    ParentRawImageComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
                    ParentRawImageComponent->SetLocalTransform(FTransform::Identity);
                    ParentRawImageComponent->SetSizeDelta(FVector2D(USpriteMergeSubsystem::AtlasWidth, USpriteMergeSubsystem::AtlasHeight));
                    ParentRawImageComponent->SetMaterial(GridMaterial);
                    ParentRawImageComponent->AwakeFromLoad();
                    
                    URawImageComponent* RawImageComponent = NewObject<URawImageComponent>(this);
                    RawImageComponent->RegisterComponent();
                    RawImageComponent->AttachToComponent(ParentRawImageComponent, FAttachmentTransformRules::KeepRelativeTransform);
                    RawImageComponent->SetLocalTransform(FTransform::Identity);
                    RawImageComponent->SetSizeDelta(FVector2D(USpriteMergeSubsystem::AtlasWidth, USpriteMergeSubsystem::AtlasHeight));
                    RawImageComponent->AwakeFromLoad();

                    RawImageComponents.Add(RawImageComponent);
                }
            }

            for (int32 Index = 0, Count = SpriteTextures.Num(); Index < Count; ++Index)
            {
                if (IsValid(RawImageComponents[Index]))
                {
                    RawImageComponents[Index]->SetTexture(SpriteTextures[Index]);

                    const auto ParentComponent = Cast<URectTransformComponent>(RawImageComponents[Index]->GetAttachParent());
                    if (IsValid(ParentComponent))
                    {
                        ParentComponent->SetEnabled(true);
                        RawImageComponents[Index]->SetHidePrimitive(false);
                    }
                }
            }

            FVector2D RegionSize = FVector2D(USpriteMergeSubsystem::AtlasWidth, USpriteMergeSubsystem::AtlasHeight);
            if (SpriteTextures.Num() > 0)
            {
                RegionSize = FVector2D(USpriteMergeSubsystem::AtlasWidth,  SpriteTextures.Num() * USpriteMergeSubsystem::AtlasHeight + Spacing * (SpriteTextures.Num() - 1));
            }
            SetSizeDelta(RegionSize);

            if (IsValid(ViewportInfo))
            {
                if (ViewportInfo->ViewportClient.IsValid())
                {
                    ViewportInfo->ViewportClient.Pin()->SetTargetViewSize(RegionSize);
                }
            }

            for (int32 Index = SpriteTextures.Num(), Count = RawImageComponents.Num(); Index < Count; ++Index)
            {
                if (IsValid(RawImageComponents[Index]))
                {
                    const auto ParentComponent = Cast<URectTransformComponent>(RawImageComponents[Index]->GetAttachParent());
                    if (IsValid(ParentComponent))
                    {
                        ParentComponent->SetEnabled(false);
                    }
                }
            }

            if (SpriteTextures.Num() == 0)
            {
                if (IsValid(RawImageComponents[0]))
                {
                    const auto ParentComponent = Cast<URectTransformComponent>(RawImageComponents[0]->GetAttachParent());
                    if (IsValid(ParentComponent))
                    {
                        ParentComponent->SetEnabled(true);
                    }
                    RawImageComponents[0]->SetHidePrimitive(true);
                }
            }

            bFirstShow = false;
        }
    }
}

/////////////////////////////////////////////////////
