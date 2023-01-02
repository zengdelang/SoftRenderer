#pragma once

#include "CoreMinimal.h"
#include "Core/UICommonDefinitions.h"
#include "UISceneViewExtension.h"
#include "UIRenderProxyInterface.generated.h"

class UCanvasRendererSubComponent;
class UUIMeshProxyComponent;
class FUIMeshBatchElement;

class FUIRenderProxyInfo
{
public:
	TMap<TWeakObjectPtr<UCanvasRendererSubComponent>, int32> CanvasRenderers;
	TWeakObjectPtr<UUIMeshProxyComponent> OwnerRenderProxy;

public:
	FUIRenderProxyInfo()
	{
		CanvasRenderers.Reserve(64);
	}
};

UINTERFACE(BlueprintType)
class UGUI_API UUIRenderProxyInterface : public UInterface
{
	GENERATED_BODY()
};

class UGUI_API IUIRenderProxyInterface
{
	GENERATED_BODY()

public:
	int32 GetMinInstructionIndex() const
	{
		return MinInstructionIndex;
	}

	int32 GetMaxInstructionIndex() const
	{
		return MaxInstructionIndex;
	}
	
	void SetInstructionIndex(int32 InMinInstructionIndex, int32 InMaxInstructionIndex);
	
	virtual void ClearUIMesh()
	{
		MinInstructionIndex = INT32_MAX;
		MaxInstructionIndex = -1;
		BatchDescIndex = -1;

		ProxyInfo.Reset();
	}
	
	virtual void UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority) = 0;

	virtual void SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension) = 0;
	virtual void SetGraphicType(EUIGraphicType InGraphicType) = 0;
	virtual void SetGraphicData(TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> InGraphicData) = 0;
	
	virtual void SetUIMaterial(const USceneComponent* CanvasSceneComp, UMaterialInterface* InMaterial, UTexture* InTexture,
		const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy) = 0;
	
	virtual bool IsExternalRenderProxy() = 0;

	virtual void SetupVirtualWorldTransform(bool bInUseVirtualWorldTransform, FTransform InVirtualWorldTransform, USceneComponent* CanvasAttachComponent) {}
	virtual void UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent) = 0;

	virtual void SetupCanvas(const UCanvasSubComponent* Canvas, TSharedPtr<FUIRenderProxyInfo> InProxyInfo, TSharedPtr<FUIMeshBatchElement> InUIMeshBatchElement,
		const int32 InBatchVerticesCount, const int32 InBatchIndexCount, bool& bRefreshRenderProxy) {}
	
public:
	int32 MinInstructionIndex = INT32_MAX;
	int32 MaxInstructionIndex = -1;
	int32 BatchDescIndex = -1;

	TSharedPtr<FUIRenderProxyInfo> ProxyInfo;
	
	uint32 VirtualTransformID = 0;
	FMatrix VirtualWorldTransform = FMatrix::Identity;
	bool bUseVirtualWorldTransform = false;
};
