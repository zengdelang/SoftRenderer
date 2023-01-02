#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "Core/Widgets/UIPrimitiveElementInterface.h"
#include "UIStaticMeshElementInterface.h"
#include "UIStaticMeshComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Mesh), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent, DisplayName = "UI Static Mesh"))
class UGUI_API UUIStaticMeshComponent : public UMaskableGraphicComponent, public IUIStaticMeshElementInterface
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
	
protected:
    virtual void UpdateGraphicEffects() override;
    virtual void UpdateRenderOpacity() override;
	
protected:
    virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override {};

public:
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

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
