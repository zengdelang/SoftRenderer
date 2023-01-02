#pragma once

#include "CoreMinimal.h"
#include "MeshModifierInterface.h"
#include "Core/Widgets/GraphicElementInterface.h"
#include "BaseMeshEffectSubComponent.generated.h"

struct FVertexHelper;

/**
 * Base class for effects that modify the generated Mesh.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UBaseMeshEffectSubComponent : public UBehaviourSubComponent, public IMeshModifierInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	TScriptInterface<IGraphicElementInterface> Graphic;

protected:
    //~ Begin BehaviourSubComponent Interface
	virtual void OnEnable() override;
    virtual void OnDisable() override;
    //~ End BehaviourSubComponent Interface

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	
public:
	/**
	 * The graphic component that the Mesh Effect will apply to.
	 */
	UFUNCTION(BlueprintCallable, Category = MeshEffect)
	TScriptInterface<IGraphicElementInterface> GetGraphic();

	UFUNCTION(BlueprintCallable, Category = MeshEffect)
	void SetGraphic(TScriptInterface<IGraphicElementInterface> InGraphic);
	
public:
	virtual void ModifyMesh(FVertexHelper& VertexHelper) override {};
	
};
