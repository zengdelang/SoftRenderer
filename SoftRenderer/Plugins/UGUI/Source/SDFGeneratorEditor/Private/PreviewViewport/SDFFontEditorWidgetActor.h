#pragma once

#include "CoreMinimal.h"
#include "SDFFontEditorComponent.h"
#include "Core/WidgetActor.h"
#include "SDFFontEditorWidgetActor.generated.h"

class UEventSystemComponent;

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class ASDFFontEditorWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;
    
    UPROPERTY(Transient)
    USDFFontEditorComponent* SDFFontEditorComponent;
    
};
