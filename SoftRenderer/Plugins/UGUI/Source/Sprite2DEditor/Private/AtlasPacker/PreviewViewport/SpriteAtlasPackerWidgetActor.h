#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerComponent.h"
#include "Core/WidgetActor.h"
#include "SpriteAtlasPackerWidgetActor.generated.h"

class UEventSystemComponent;

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class ASpriteAtlasPackerWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;
    
    UPROPERTY(Transient)
    USpriteAtlasPackerComponent* SpriteAtlasPackerComponent;
    
};
