#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UGUIUtility.generated.h"

UCLASS()
class UGUI_API UUGUIUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = UGUIUtility)
	static URectTransformComponent* AddUIComponent(TSubclassOf<URectTransformComponent> ComponentClass, URectTransformComponent* ParentComponent);

};
