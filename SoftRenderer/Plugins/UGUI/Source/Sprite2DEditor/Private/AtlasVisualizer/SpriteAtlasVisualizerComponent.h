#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/VerticalLayoutGroupComponent.h"
#include "Core/Widgets/RawImageComponent.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "SpriteAtlasVisualizerComponent.generated.h"

UCLASS()
class USpriteAtlasVisualizerComponent : public UVerticalLayoutGroupComponent
{
    GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(Transient)
    TArray<URawImageComponent*> RawImageComponents;

    UPROPERTY(Transient)
    UUGUIEditorViewportInfo* ViewportInfo;
    
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* GridMaterial;

    bool bFirstShow = true;
    
public:
	virtual void Awake() override;
	
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
