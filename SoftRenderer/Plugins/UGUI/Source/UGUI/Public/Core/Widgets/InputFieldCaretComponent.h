#pragma once

#include "CoreMinimal.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "InputFieldCaretComponent.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UGUI_API UInputFieldCaretComponent : public URectTransformComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Transient)
	UCanvasRendererSubComponent* CanvasRenderer;

};
