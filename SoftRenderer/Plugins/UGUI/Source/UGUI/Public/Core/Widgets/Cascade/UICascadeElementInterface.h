#pragma once

#include "CoreMinimal.h"
#include "Particles/ParticleSystem.h"
#include "UObject/Interface.h"
#include "UICascadeElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UUICascadeElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API IUICascadeElementInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual UParticleSystem* GetParticleTemplate() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void SetParticleTemplate(UParticleSystem* InTemplate) {}

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual bool GetActivateOnEnable() const { return false; }

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void SetActivateOnEnable(bool bInActivateOnEnable) {}

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual bool GetDeactivateOnDisable() const { return false; }

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void SetDeactivateOnDisable(bool bInDeactivateOnDisable) {}

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void SetActivateParticles(bool bInActivateParticles) {}

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void ActivateParticles(bool bReset = true) {}

    UFUNCTION(BlueprintCallable, Category = "UI Cascade")
    virtual void DeactivateParticles() {}
	
};
