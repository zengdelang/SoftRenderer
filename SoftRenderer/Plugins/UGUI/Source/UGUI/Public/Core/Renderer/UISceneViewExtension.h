#pragma once

#include "CoreMinimal.h"
#include "UISceneProxy.h"
#include "SceneViewExtension.h"
#include "UIBlurPostProcessor.h"
#include "UICanvasPrimitiveBuffer.h"
#include "UIGlitchPostProcessor.h"
#include "UIDepthStencilResource.h"
#include "Core/UICommonDefinitions.h"

class UCanvasSubComponent;

struct FUIMeshProxyElement
{
public:
	uint32 ProxyId;
	uint32 ComponentId;
	FUISceneProxy* SceneProxyPtr;

public:
	FUIMeshProxyElement()
		: ProxyId(0)
		, ComponentId(0)
		, SceneProxyPtr(nullptr)
	{
		
	}

	FUIMeshProxyElement(uint32 InProxyId, uint32 InComponentId, FUISceneProxy* InSceneProxyPtr)
		: ProxyId(InProxyId)
		, ComponentId(InComponentId)
		, SceneProxyPtr(InSceneProxyPtr)
	{
		
	}
};

struct FProxyIndexedSet
{
protected:
	TArray<FUIMeshProxyElement> List;

    TMap<uint32, int32> Map;

public:
	FProxyIndexedSet()
	{
		List.Reserve(128);
		Map.Reserve(128);
	}
	
public:
	FORCEINLINE bool Emplace(FUIMeshProxyElement&& Item)
    {
        const uint32 ComponentId = Item.ComponentId;
        const auto ItemIndexPtr = Map.Find(ComponentId);
        if (ItemIndexPtr)
        {
            return false;
        }
    	
    	const int32 NewIndex = List.Emplace(Item);
    	Map.Emplace(ComponentId, NewIndex); 
    	return true;
    }
	
    FORCEINLINE int32 Num() const
    {
        return List.Num();
    }
	
    FORCEINLINE const FUIMeshProxyElement& operator[](int32 Index)
    {
        return List[Index];
    }

	FORCEINLINE const FUIMeshProxyElement* Get(uint32 ComponentId)
	{
		const auto ItemIndexPtr = Map.Find(ComponentId);
		if (!ItemIndexPtr)
		{
			return nullptr;
		}
		return &List[*ItemIndexPtr];
	}

	FORCEINLINE bool Remove(uint32 ComponentId)
    {
    	const auto ItemIndexPtr = Map.Find(ComponentId);
    	if (!ItemIndexPtr)
    	{
    		return false;
    	}

		RemoveAt(*ItemIndexPtr);
    	return true;
    }

	FORCEINLINE void RemoveAt(int32 Index)
	{
		const uint32 ComponentId = List[Index].ComponentId;
		Map.Remove(ComponentId);
		if (Index == (List.Num() - 1))
		{
			List.RemoveAt(Index, 1, false);
		}
		else
		{
			const int32 ReplaceItemIndex = List.Num() - 1;
			const FUIMeshProxyElement& ReplaceItem = List[ReplaceItemIndex];
			List[Index].ComponentId = ReplaceItem.ComponentId;
			List[Index].ProxyId = ReplaceItem.ProxyId;
			List[Index].SceneProxyPtr = ReplaceItem.SceneProxyPtr;
			Map.Emplace(ReplaceItem.ComponentId, Index);
			List.RemoveAt(ReplaceItemIndex, 1, false);
		}
	}

    struct FCompareUIProxyElement
    {
    	FORCEINLINE bool operator()(const FUIMeshProxyElement& A, const FUIMeshProxyElement& B) const
    	{
    		return A.SceneProxyPtr->GetRenderPriority() < B.SceneProxyPtr->GetRenderPriority();
    	}
    };

	FORCEINLINE void Sort()
    {
        if (List.Num() <= 0)
            return;
    	
        // There might be better ways to sort and keep the dictionary index up to date.
        List.Sort(FCompareUIProxyElement());
    	
    	for (int32 Index = 0, Count = List.Num(); Index < Count; ++Index)
        {
            Map.Emplace(List[Index].ComponentId, Index); 
        }
    }
};

class UGUI_API FUISceneViewExtension : public FSceneViewExtensionBase
{
public:
	FUISceneViewExtension(const FAutoRegister&, UCanvasSubComponent* InCanvas);
	virtual ~FUISceneViewExtension() override;

public:
	void SetRenderMode(ECanvasRenderMode InRenderMode)
	{
		RenderMode = InRenderMode;
	}

	void AddUISceneProxy(uint32 ProxyId, uint32 ComponentId, FUISceneProxy* ProxyPtr);
	void AddUISceneProxy_RenderThread(uint32 ProxyId, uint32 ComponentId, FUISceneProxy* ProxyPtr);
	
	void RemoveUISceneProxy_RenderThread(uint32 ProxyId, uint32 ComponentId);

	void SortRenderPriority();
	void SortRenderPriority_RenderThread();

	void FlushGeneratedResources() const;
	
	void SetPriority(int32 NewPriority) { Priority = NewPriority; }
	
public:
	//Begin ISceneViewExtension interfaces
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {}
	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo) override {}
	virtual void SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData) override {}

	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {}
	
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override {}
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override {}

	virtual void PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
	virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override {}

	virtual int32 GetPriority() const override { return Priority; }

#if ENGINE_MINOR_VERSION <= 26 && ENGINE_MAJOR_VERSION <= 4
	virtual bool IsActiveThisFrameInContext(FSceneViewExtensionContext& Context) const override;
#else
protected:
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
#endif
	//End ISceneViewExtension interfaces

protected:
	static void UpdateRasterizerState(const FMaterial& InMaterialResource, FGraphicsPipelineStateInitializer& InGraphicsPSOInit);
	
protected:
	FProxyIndexedSet RenderThread_UIProxies;

	TWeakObjectPtr<UCanvasSubComponent> Canvas;
	TWeakObjectPtr<UCanvasSubComponent> RootCanvas;

	ECameraProjectionMode::Type ProjectionMode;
	
	FVector ViewLocation;
	FMatrix ViewRotationMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;
	
	int32 Priority;
	
	int32 UIBatches;
	int32 BlurUIBatches;
	int32 GlitchUIBatches;
	
	FIntPoint LastDepthStencilSize;

	FUICanvasPrimitiveBuffer* CanvasPrimitiveBuffer;
	FMatrix CanvasLocalToWorld;

	TArray<const FSceneView*> Views;
	
	ECanvasRenderMode RenderMode;

	ECanvasRenderTargetMode RenderTargetMode;
	FTextureRenderTargetResource* RenderTargetResource;

	TSharedRef<FUIBlurPostProcessor> BlurPostProcessor;
	TSharedRef<FUIGlitchPostProcessor> GlitchPostProcessor;
	FUIDepthStencilResource* IntermediateDepthStencil;

	int32 GameThread_RenderTimes;
	int32 RenderThread_RenderTimes;
	
	uint8 bGammaCorrect : 1;
	uint8 bSortUIProxies : 1;
	uint8 bPerformFrustumCull : 1;
	uint8 bCheckSceneViewVisibility : 1;

	static FGlobalDynamicIndexBuffer DynamicIndexBuffer;
	static FGlobalDynamicVertexBuffer DynamicVertexBuffer;
	static TGlobalResource<FGlobalDynamicReadBuffer> DynamicReadBuffer;

public:
	/** Next id to be used by a component. */
	static FThreadSafeCounter NextProxyId;

};
