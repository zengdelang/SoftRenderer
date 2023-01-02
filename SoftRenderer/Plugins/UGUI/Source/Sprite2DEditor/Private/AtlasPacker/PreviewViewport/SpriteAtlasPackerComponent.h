#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/RawImageComponent.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "SpriteAtlasPackerComponent.generated.h"

UCLASS()
class USpriteAtlasPackerComponent : public URawImageComponent
{
    GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(Transient)
    URawImageComponent* RawImageComponent;

    UPROPERTY(Transient)
    UUGUIEditorViewportInfo* ViewportInfo;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* GridMaterial;
	
public:
    virtual void Awake() override;

protected:
    void OnSpriteListSelectionChanged(int32 Width, int32 Height, UTexture2D* Texture2D);
    
};
