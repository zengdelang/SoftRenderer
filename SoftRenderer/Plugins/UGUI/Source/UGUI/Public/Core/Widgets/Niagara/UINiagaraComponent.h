#pragma once

#include "CoreMinimal.h"
#include "UINiagaraElementInterface.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "UINiagaraComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (FX), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent, DisplayName = "UI Niagra"))
class UGUI_API UUINiagaraComponent : public UMaskableGraphicComponent, public IUINiagaraElementInterface
{
	GENERATED_UCLASS_BODY()
    
protected:
    UPROPERTY(EditAnywhere, Category = Graphic)
    UNiagaraSystem* NiagaraSystemAsset;

    UPROPERTY(Transient)
    class UUINiagaraProxyComponent* NiagaraComponent;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bActivateOnEnable : 1;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bDeactivateOnDisable : 1;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bActivateParticles : 1;    // for sequence

public:
	virtual void OnEnable() override;
    virtual void OnDisable() override;

protected:
    virtual void UpdateGraphicEffects() override;
    virtual void UpdateRenderOpacity() override;
	
protected:
    virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override {};
	
public:
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

public:
	virtual UNiagaraSystem* GetNiagaraSystemAsset() const override
	{
        return NiagaraSystemAsset;
	}

    virtual void SetNiagaraSystemAsset(UNiagaraSystem* InNiagaraSystemAsset) override;
 
    virtual bool GetActivateOnEnable() const override
    {
        return bActivateOnEnable;
    }

    virtual void SetActivateOnEnable(bool bInActivateOnEnable) override
    {
        if (bActivateOnEnable != bInActivateOnEnable)
        {
            bActivateOnEnable = bInActivateOnEnable;
        }
    }

    virtual bool GetDeactivateOnDisable() const override
    {
        return bDeactivateOnDisable;
    }

    virtual void SetDeactivateOnDisable(bool bInDeactivateOnDisable) override
    {
        if (bDeactivateOnDisable != bInDeactivateOnDisable)
        {
            bDeactivateOnDisable = bInDeactivateOnDisable;
        }
    }

    virtual void SetActivateParticles(bool bInActivateParticles) override;

    virtual void ActivateParticles(bool bReset = true) override;

    virtual void DeactivateParticles() override;

protected:
    UFUNCTION()
    void OnParticleSystemFinished(class UNiagaraComponent* FinishedComponent);
	
};
