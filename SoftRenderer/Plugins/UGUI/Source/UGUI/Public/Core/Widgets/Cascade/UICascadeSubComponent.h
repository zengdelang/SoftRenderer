#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/MaskableGraphicSubComponent.h"
#include "UICascadeElementInterface.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"
#include "UICascadeSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "UI Cascade"))
class UGUI_API UUICascadeSubComponent : public UMaskableGraphicSubComponent, public IUICascadeElementInterface, public IUIPrimitiveElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(EditAnywhere, Category = Graphic)
	UParticleSystem* ParticleTemplate;

    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Graphic, Meta = (ToolTip = "Material overrides."))
    TArray<class UMaterialInterface*> OverrideMaterials;

    UPROPERTY(Transient)
    class UUICascadeProxyComponent* ParticleSystemComponent;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bActivateOnEnable : 1;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bDeactivateOnDisable : 1;

    UPROPERTY(EditAnywhere, Category = Graphic)
    uint8 bActivateParticles : 1;    // for sequence

#if WITH_EDITOR
public:
    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;
    void CleanUpOverrideMaterials();
#endif

public:
    virtual void OnEnable() override;
    virtual void OnDisable() override;
    virtual void OnDestroy() override;

protected:
    virtual void UpdateGraphicEffects() override;
    virtual void UpdateRenderOpacity() override;
	
protected:
    virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override {};

public:
    virtual UParticleSystem* GetParticleTemplate() const override
    {
        return ParticleTemplate;
    }

    virtual void SetParticleTemplate(UParticleSystem* InParticleTemplate) override;

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

    virtual int32 GetNumOverrideMaterials() const override;
    virtual UMaterialInterface* GetOverrideMaterial(int32 MaterialIndex) const override;
    virtual void SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) override;

protected:
    UFUNCTION()
    void OnParticleSystemFinished(class UParticleSystemComponent* FinishedComponent);
	
};
