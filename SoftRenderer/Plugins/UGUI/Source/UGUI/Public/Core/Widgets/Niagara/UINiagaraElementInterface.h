#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "UObject/Interface.h"
#include "UINiagaraElementInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UGUI_API UUINiagaraElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API IUINiagaraElementInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual UNiagaraSystem* GetNiagaraSystemAsset() const { return nullptr; }

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void SetNiagaraSystemAsset(UNiagaraSystem* InNiagaraSystemAsset) {}

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual bool GetActivateOnEnable() const { return false; }

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void SetActivateOnEnable(bool bInActivateOnEnable) {}

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual bool GetDeactivateOnDisable() const { return false; }

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void SetDeactivateOnDisable(bool bInDeactivateOnDisable) {}

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void SetActivateParticles(bool bInActivateParticles) {}

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void ActivateParticles(bool bReset = true) {}

    UFUNCTION(BlueprintCallable, Category = "UI Niagara")
    virtual void DeactivateParticles() {}
	
};
