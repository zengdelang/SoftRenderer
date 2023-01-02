#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "RenderingThread.h"

/**
 * Handle to resources used for depth stencil processing.  This should not be deleted manually because it implements FDeferredCleanupInterface
 */
class FUIDepthStencilResource : public FRenderResource, private FDeferredCleanupInterface
{
public:
	FUIDepthStencilResource();
	virtual ~FUIDepthStencilResource() override;

	FTexture2DRHIRef GetRenderTarget()
	{
		return DepthStencil;
	}

	/** Performs per frame updates to this resource */
	void Update(const FIntPoint& NewSize);

	void CleanUp();

	/** FRenderResource interface */
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;

private:
	/** Resizes targets to the new size */
	void ResizeTargets(const FIntPoint& NewSize);

private:
	FTexture2DRHIRef DepthStencil;
	FIntPoint DepthStencilSize;

};
