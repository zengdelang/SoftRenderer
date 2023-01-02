#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/MaskableGraphicComponent.h"
#include "UIPrimitiveDrawerComponent.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UIBLUEPRINTEDITOR_API UUIPrimitiveDrawerComponent : public UMaskableGraphicComponent
{
	GENERATED_UCLASS_BODY()

protected:
    UPROPERTY(Transient)
    class UUIPrimitiveDrawerProxyComponent* PrimitiveDrawerProxyComponent;

public:
    virtual void Awake() override;
    virtual void OnDisable() override;

protected:
    void OnEditorUIDesignerInfoChanged(const UWorld* InWorld) const;
    void MarkProxyRenderStateDirty() const;
    
protected:
    virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override {};

public:
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
    
};
