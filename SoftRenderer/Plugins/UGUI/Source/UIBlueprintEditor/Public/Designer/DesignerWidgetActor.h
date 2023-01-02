#pragma once

#include "CoreMinimal.h"
#include "UIRaycastRegionComponent.h"
#include "Core/WidgetActor.h"
#include "PrimitiveDrawer/UIPrimitiveDrawerComponent.h"
#include "DesignerWidgetActor.generated.h"

UCLASS(Blueprintable, BlueprintType, HideDropdown)
class UIBLUEPRINTEDITOR_API ADesignerWidgetActor : public AWidgetActor
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY(Transient)
    URectTransformComponent* UIRoot;

    UPROPERTY(Transient)
    UUIPrimitiveDrawerComponent* DrawerComponent;

    UPROPERTY(Transient)
    UUIRaycastRegionComponent* RaycastRegionComponent;
};
