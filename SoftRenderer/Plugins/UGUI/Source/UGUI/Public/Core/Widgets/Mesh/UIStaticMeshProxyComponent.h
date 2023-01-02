#pragma once

#include "CoreMinimal.h"
#include "Core/Renderer/UIRenderProxyInterface.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "UObject/ObjectMacros.h"
#include "Core/UICommonDefinitions.h"
#include "Components/StaticMeshComponent.h"
#include "UIStaticMeshProxyComponent.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UGUI_API UUIStaticMeshProxyComponent : public UStaticMeshComponent, public IUIRenderProxyInterface
{
	GENERATED_BODY()

	friend class FUIStaticMeshSceneProxy;
	
protected:
	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;
	
	ECanvasRenderMode RenderMode;

	TWeakPtr<FUIGraphicData> GraphicData;
	EUIGraphicType GraphicType;
	
	FVector2D GraphicEffects;
	float RenderOpacity;

	uint8 bUpdateTranslucentSortPriority : 1;
	uint8 bUpdateVirtualWorldTransform : 1;
	
public:
	UUIStaticMeshProxyComponent(const FObjectInitializer& ObjectInitializer);

public:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UActorComponent Interface.
	virtual void SendRenderDynamicData_Concurrent() override;
	//~ Begin UActorComponent Interface.

public:
	void SetGraphicEffects(FVector2D InGraphicEffects, bool bUpdateMaterial);
	void SetRenderOpacity(float InRenderOpacity, bool bUpdateMaterial);
	
public:
	virtual void SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension) override;
	
	virtual void SetGraphicType(EUIGraphicType InGraphicType) override {}
	virtual void SetGraphicData(TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> InGraphicData) override {}

	virtual void ClearUIMesh() override;
	virtual void SetUIMaterial(const USceneComponent* CanvasSceneComp, UMaterialInterface* InMaterial, UTexture* InTexture,
		const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy) override;

	virtual void UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority) override;

	virtual bool IsExternalRenderProxy() override { return true; }

	virtual void SetupVirtualWorldTransform(bool bInUseVirtualWorldTransform, FTransform InVirtualWorldTransform, USceneComponent* CanvasAttachComponent) override;
	virtual void UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent) override;
};
