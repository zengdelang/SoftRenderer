#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "UIAnimationSlotComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Animation), meta = (DisplayName = "UI Animation Slot", UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UUIAnimationSlotComponent : public URectTransformComponent
{
	GENERATED_BODY()

};
