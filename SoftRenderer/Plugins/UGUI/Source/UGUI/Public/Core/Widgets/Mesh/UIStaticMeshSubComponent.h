#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/MaskableGraphicSubComponent.h"
#include "UIStaticMeshElementInterface.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"
#include "UIStaticMeshSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "UI Static Mesh"))
class UGUI_API UUIStaticMeshSubComponent : public UMaskableGraphicSubComponent, public IUIStaticMeshElementInterface, public IUIPrimitiveElementInterface
{
	GENERATED_UCLASS_BODY()

protected:
    /** The static mesh that this component uses to render */
    UPROPERTY(EditAnywhere, Category = Graphic)
    UStaticMesh* StaticMesh;

    /** Per-Component material overrides.  These must NOT be set directly or a race condition can occur between GC and the rendering thread. */
    UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Graphic, Meta = (ToolTip = "Material overrides."))
    TArray<class UMaterialInterface*> OverrideMaterials;
	
    UPROPERTY(Transient)
    class UUIStaticMeshProxyComponent* StaticMeshComponent;

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
    virtual UStaticMesh* GetStaticMesh() const override
    {
        return StaticMesh;
    }

    virtual void SetStaticMesh(UStaticMesh* InStaticMesh) override;

    virtual int32 GetNumOverrideMaterials() const override;
    virtual UMaterialInterface* GetOverrideMaterial(int32 MaterialIndex) const override;
    virtual void SetOverrideMaterial(int32 ElementIndex, UMaterialInterface* InMaterial) override;
	
};
