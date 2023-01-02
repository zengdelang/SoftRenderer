#pragma once

#include "CoreMinimal.h"
#include "SpriteEditorComponent.h"
#include "Core/WidgetActor.h"
#include "SpriteEditorWidgetActor.generated.h"

class UEventSystemComponent;

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class ASpriteEditorWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;
    
    UPROPERTY(Transient)
    USpriteEditorComponent* SpriteEditorComponent;

public:
    void UpdateActor() const;
    
};
