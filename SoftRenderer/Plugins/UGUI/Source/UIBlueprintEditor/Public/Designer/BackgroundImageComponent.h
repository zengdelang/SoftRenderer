#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/RawImageComponent.h"
#include "BackgroundImageComponent.generated.h"

struct FImageRegion
{
    FVector2D BottomLeft;
    FVector2D TopLeft;
    FVector2D TopRight;
    FVector2D BottomRight;
};

UCLASS()
class UBackgroundImageComponent : public URawImageComponent
{
    GENERATED_UCLASS_BODY()

protected:
    FString BackgroundImagePath;

    FImageRegion BackgroundRegion;

public:
    TWeakPtr<class FSCSUIEditorViewportClient> ViewportClient;

public:
    virtual void OnEnable() override;
    virtual void OnDisable() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual auto OnPopulateMesh(FVertexHelper& VertexHelper) -> void override;

protected:
    const FString RelativePathToAbsolutePath(const FString RelativePath);
    void LoadImageToTexture(class AWidgetActor* WidgetActor, const FString& ImagePath);
    
};
