#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/RawImageComponent.h"
#include "Sprite2DEditor/Sprite2DEditorViewportInfo.h"
#include "SpriteEditorComponent.generated.h"

UCLASS()
class USpriteEditorComponent : public URawImageComponent
{
    GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(Transient)
    URawImageComponent* RawImageComponent;

    UPROPERTY(Transient)
    USprite2DEditorViewportInfo* ViewportInfo;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* GridMaterial;
	
public:
    virtual void Awake() override;

public:
    void UpdateSprite();
    
};
