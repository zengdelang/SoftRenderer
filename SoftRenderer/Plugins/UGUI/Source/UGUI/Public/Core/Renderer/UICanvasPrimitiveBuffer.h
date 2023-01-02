#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "RenderingThread.h"

struct FPrimitiveBufferInfo
{
public:
	int32 Count;
	TSharedPtr<TUniformBuffer<FPrimitiveUniformShaderParameters>> PrimitiveUniformBuffer;
	FMatrix VirtualWorldTransform;
	
public:
	FPrimitiveBufferInfo()
		: Count(0)
	{
		
	}
};

class UGUI_API FUICanvasPrimitiveBuffer : public FRenderResource, private FDeferredCleanupInterface
{
public:
	FUICanvasPrimitiveBuffer();
	virtual ~FUICanvasPrimitiveBuffer() override;
	
	/** Performs per frame updates to this resource */
	void Update(const FMatrix& NewCanvasLocalToWorld);
	
	void CleanUp();

	TUniformBuffer<FPrimitiveUniformShaderParameters>* GetCanvasPrimitiveUniformBuffer() const { return CanvasPrimitiveUniformBuffer.Get();}

	/** FRenderResource interface */
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;

public:
	void AddPrimitiveUniformBufferCount(uint32 TransformId, FMatrix VirtualWorldTransform);
	void RemovePrimitiveUniformBuffer(uint32 TransformId);
	void UpdatePrimitiveUniformBuffer(uint32 TransformId, FMatrix VirtualWorldTransform);

	TUniformBuffer<FPrimitiveUniformShaderParameters>* GetPrimitiveUniformBuffer(uint32 TransformId);
	const FMatrix& GetVirtualWorldTransform(uint32 TransformId);

private:
	TSharedPtr<TUniformBuffer<FPrimitiveUniformShaderParameters>> CanvasPrimitiveUniformBuffer;
	FMatrix CanvasLocalToWorld;

	TMap<uint32, FPrimitiveBufferInfo> PrimitiveUniformBufferMap;
	
};
