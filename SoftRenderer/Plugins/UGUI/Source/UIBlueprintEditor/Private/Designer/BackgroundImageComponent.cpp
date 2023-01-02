#include "Designer/BackgroundImageComponent.h"
#include "SCSUIEditorViewportClient.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "UIEditorPerProjectUserSettings.h"
#include "Core/WidgetActor.h"
#include "Misc/FileHelper.h"

UBackgroundImageComponent::UBackgroundImageComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bRaycastTarget = false;
    
    BackgroundImagePath = TEXT("");

    bTickInEditor = true;
    PrimaryComponentTick.bHighPriority = true;
    PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
    PrimaryComponentTick.bCanEverTick = true;
}

void UBackgroundImageComponent::OnEnable()
{
	Super::OnEnable();

#if WITH_EDITORONLY_DATA
    AWidgetActor::OnBackgroundImageFilePathChanged.AddUObject(this, &UBackgroundImageComponent::LoadImageToTexture);
#endif

    SetAnchorAndOffset(FVector2D::ZeroVector, FVector2D(1, 1), FVector2D::ZeroVector, FVector2D::ZeroVector);
}

void UBackgroundImageComponent::OnDisable()
{
#if WITH_EDITORONLY_DATA
    AWidgetActor::OnBackgroundImageFilePathChanged.RemoveAll(this);
#endif
	Super::OnDisable();
}

void UBackgroundImageComponent::TickComponent(float DeltaTime, ELevelTick Tick,FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

    if(GetMutableDefault<UUIEditorPerProjectUserSettings>()->bShowBackgroundImage && this->Texture)
    {
        this->SetHidePrimitive(false);
    }
    else
    {
        this->SetHidePrimitive(true);
    }

    if(!GetCanvasRenderer()->IsHidePrimitive())
    {
        FVector WorldCorners[4];

        if (!ViewportClient.IsValid())
            return;

        const AActor* WidgetActor = ViewportClient.Pin()->GetPreviewActor();
        const auto Root = WidgetActor->GetRootComponent();

        const auto Comp = Cast<URectTransformComponent>(Root);
        Comp->GetWorldCorners(WorldCorners);

        const FTransform WorldToLocal = GetComponentTransform().Inverse();

        BackgroundRegion.BottomLeft = FVector2D(WorldToLocal.TransformPosition(WorldCorners[0]));
        BackgroundRegion.TopLeft = FVector2D(WorldToLocal.TransformPosition(WorldCorners[1]));
        BackgroundRegion.TopRight = FVector2D(WorldToLocal.TransformPosition(WorldCorners[2]));
        BackgroundRegion.BottomRight = FVector2D(WorldToLocal.TransformPosition(WorldCorners[3]));

        SetVerticesDirty();
    }
}

void UBackgroundImageComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
    Super::OnPopulateMesh(VertexHelper);

    VertexHelper.Reset();

    FLinearColor RegionColor(1.0, 1.0, 1.0, 1.0);

    VertexHelper.AddVert(FVector(BackgroundRegion.BottomLeft.X, BackgroundRegion.BottomLeft.Y, 0), RegionColor, FVector2D(0, 1));
    VertexHelper.AddVert(FVector(BackgroundRegion.TopLeft.X, BackgroundRegion.TopLeft.Y, 0), RegionColor, FVector2D(0, 0));
    VertexHelper.AddVert(FVector(BackgroundRegion.TopRight.X, BackgroundRegion.TopRight.Y, 0), RegionColor, FVector2D(1, 0));
    VertexHelper.AddVert(FVector(BackgroundRegion.BottomRight.X, BackgroundRegion.BottomRight.Y, 0), RegionColor, FVector2D(1, 1));

    VertexHelper.AddTriangle(0, 1, 2);
    VertexHelper.AddTriangle(2, 3, 0);
}

const FString UBackgroundImageComponent::RelativePathToAbsolutePath(const FString RelativePath)
{
    if (RelativePath.Contains(FString("./")) || RelativePath.Contains(FString("../")))
    {
        return FPaths::ProjectDir() + "/" + RelativePath;
    }
    else
    {
        return RelativePath;
    }
}

TSharedPtr<IImageWrapper> GetImageWrapper(const FString ImageName)
{
    const FString Ex = FPaths::GetExtension(ImageName, false);
    EImageFormat ImageFormat = EImageFormat::Invalid;

    if (Ex.Equals(TEXT("jpeg"), ESearchCase::IgnoreCase) || Ex.Equals(TEXT("JPG"), ESearchCase::IgnoreCase))
    {
        ImageFormat = EImageFormat::JPEG;
    }
    else if (Ex.Equals(TEXT("png"), ESearchCase::IgnoreCase))
    {
        ImageFormat = EImageFormat::PNG;
    }
    else if (Ex.Equals(TEXT("bmp"), ESearchCase::IgnoreCase))
    {
        ImageFormat = EImageFormat::BMP;
    }

    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
    TSharedPtr<IImageWrapper> ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
    return ImageWrapperPtr;
}

void UBackgroundImageComponent::LoadImageToTexture(AWidgetActor* WidgetActor, const FString& ImagePath)
{
    if (!IsValid(WidgetActor))
    {
        return;
    }

    bool bContinueLoadImage = false;

    if (WidgetActor->IsTemplate())
    {
        TArray<UObject*> ArchetypeInstances;
        WidgetActor->GetArchetypeInstances(ArchetypeInstances);
        for (const auto& Instance : ArchetypeInstances)
        {
            if (Instance->GetWorld() == GetWorld())
            {
                bContinueLoadImage = true;
                break;
            }
        }
    }
    else
    {
        if (WidgetActor->GetWorld() == GetWorld())
        {
            bContinueLoadImage = true;
        }
    }
    
    if (!bContinueLoadImage)
    {
        return;
    }
    
    if (ImagePath == BackgroundImagePath)
    {
        return;
    }

    BackgroundImagePath = ImagePath;

    const FString FinalPath = RelativePathToAbsolutePath(ImagePath);  
    
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.FileExists(*FinalPath))
    {
        Texture = nullptr;
        SetMaterialDirty();
        return;
    }

    TArray<uint8> ImageResultData;
    FFileHelper::LoadFileToArray(ImageResultData, *FinalPath);

    const TSharedPtr<IImageWrapper> ImageWrapper = GetImageWrapper(*FinalPath);
    if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageResultData.GetData(), ImageResultData.Num()))
    {
        TArray<uint8> OutRawData;
        constexpr ERGBFormat InFormat = ERGBFormat::RGBA;
        if (ImageWrapper->GetRaw(InFormat, 8, OutRawData))
        {
            UTexture2D* Texture2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
            const auto TextureData = Texture2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

            FMemory::Memcpy(TextureData, OutRawData.GetData(), OutRawData.Num());

            Texture2D->PlatformData->Mips[0].BulkData.Unlock();
            Texture2D->UpdateResource();

            Texture = Texture2D;
            SetMaterialDirty();
            return;
        }
    }
    
    Texture = nullptr;
    SetMaterialDirty();
}
