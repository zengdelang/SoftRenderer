#pragma once

#include "CoreMinimal.h"
#include "Core/WidgetActor.h"
#include "UIPrimitiveSelectComponent.h"
#include "UISelectedRectDrawerComponent.h"
#include "UGUI/Public/Core/Widgets/ImageComponent.h"
#include "EventSystem/EventSystemComponent.h"
#include "Designer2DWidgetActor.generated.h"

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class UIBLUEPRINTEDITOR_API ADesigner2DWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;

    UPROPERTY(Transient)
    UUIPrimitiveSelectComponent* PrimitiveSelectComponent;

    UPROPERTY(Transient)
    UUISelectedRectDrawerComponent* SelectedRectDrawerComponent;

    UPROPERTY(Transient)
    UImageComponent* CheckBoxDrawComponent;

public:
    UPROPERTY(Transient)
    UEventSystemComponent* EventSystemComponent;
    
};
