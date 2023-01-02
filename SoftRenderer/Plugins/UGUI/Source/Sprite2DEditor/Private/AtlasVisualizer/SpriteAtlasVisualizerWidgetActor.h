#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasVisualizerComponent.h"
#include "Core/WidgetActor.h"
#include "SpriteAtlasVisualizerWidgetActor.generated.h"

class UEventSystemComponent;

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class ASpriteAtlasVisualizerWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;
    
    UPROPERTY(Transient)
    USpriteAtlasVisualizerComponent* SpriteAtlasVisualizerComponent;
    
};
