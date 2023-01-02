#pragma once

#include "CoreMinimal.h"
#include "SDFFontEditorViewportInfo.h"
#include "Core/Widgets/Text/TextComponent.h"
#include "SDFFontEditorComponent.generated.h"

UCLASS()
class USDFFontEditorComponent : public URectTransformComponent
{
    GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(Transient)
    UTextComponent* TextComponent;

    UPROPERTY(Transient)
    USDFFontEditorViewportInfo* ViewportInfo;

public:
    virtual void Awake() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
