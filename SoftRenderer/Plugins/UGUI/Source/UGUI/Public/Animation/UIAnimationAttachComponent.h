#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "UIAnimationAttachComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Animation), meta = (DisplayName = "UI Animation Attach", UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UUIAnimationAttachComponent : public URectTransformComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "UI Animation Attach")
	FName SlotName = NAME_None;
	
};
