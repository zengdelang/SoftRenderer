#pragma once

#include "CoreMinimal.h"
#include "BackgroundImageComponent.h"
#include "Core/WidgetActor.h"
#include "BackgroundImageActor.generated.h"

UCLASS(Blueprintable,BlueprintType,HideDropdown)
class ABackgroundImageActor : public AWidgetActor
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Transient)
    URectTransformComponent* UIRoot;

	UPROPERTY(Transient)
    UBackgroundImageComponent* BackgroundImageComponent;
};
