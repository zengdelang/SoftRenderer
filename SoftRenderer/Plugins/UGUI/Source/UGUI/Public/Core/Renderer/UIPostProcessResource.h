#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"
#include "RenderingThread.h"

/**
 * Handle to resources used for post processing.  This should not be deleted manually because it implements FDeferredCleanupInterface
 */
class FUIPostProcessResource : public FRenderResource, private FDeferredCleanupInterface
{
public:
	FUIPostProcessResource(int32 InRenderTargetCount);
	virtual ~FUIPostProcessResource() override;

	FTexture2DRHIRef GetRenderTarget(int32 Index)
	{
		return RenderTargets[Index];
	}

	int32 GetRenderTargetNum() const
	{
		return RenderTargets.Num();
	}

	/** Performs per frame updates to this resource */
	void Update(const FIntPoint& NewSize);

	void CleanUp();

	/** FRenderResource interface */
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;

	virtual uint32 GetWidth() const { return RenderTargetSize.X; }
	virtual uint32 GetHeight() const { return RenderTargetSize.Y; }

private:
	/** Resizes targets to the new size */
	void ResizeTargets(const FIntPoint& NewSize);

public:
	static FUIPostProcessResource* GetUIPostProcessResource()
	{
		FScopeLock ScopeLock(&CriticalSection);
		
		Count = FMath::Max(0, ++Count);
		if (GlobalUIPostProcessResource == nullptr && Count > 0)
		{
			GlobalUIPostProcessResource = new FUIPostProcessResource(2);
		}
		return GlobalUIPostProcessResource;
	}

	static void ReleaseUIPostProcessResource()
	{
		FScopeLock ScopeLock(&CriticalSection);
		
		Count = FMath::Max(0, --Count);
		if (GlobalUIPostProcessResource != nullptr && Count <= 0)
		{
			GlobalUIPostProcessResource->CleanUp();
			GlobalUIPostProcessResource = nullptr;
		}
	}

private:
	static int32 Count;
	static FUIPostProcessResource* GlobalUIPostProcessResource;
	static FCriticalSection CriticalSection;

private:
	TArray<FTexture2DRHIRef, TInlineAllocator<2>> RenderTargets;
	EPixelFormat PixelFormat;
	FIntPoint RenderTargetSize;
	int32 RenderTargetCount;
	
};
