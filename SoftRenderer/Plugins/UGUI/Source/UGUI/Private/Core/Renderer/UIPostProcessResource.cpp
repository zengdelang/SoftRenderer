#include "Core/Renderer/UIPostProcessResource.h"

int32 FUIPostProcessResource::Count = 0;
FUIPostProcessResource* FUIPostProcessResource::GlobalUIPostProcessResource = nullptr;
FCriticalSection FUIPostProcessResource::CriticalSection;

FUIPostProcessResource::FUIPostProcessResource(int32 InRenderTargetCount)
	: RenderTargetSize(FIntPoint::ZeroValue)
	, RenderTargetCount(InRenderTargetCount)
{
	BeginInitResource(this);
}

FUIPostProcessResource::~FUIPostProcessResource()
{

}

void FUIPostProcessResource::Update(const FIntPoint& NewSize)
{
	if (NewSize.X > RenderTargetSize.X || NewSize.Y > RenderTargetSize.Y || RenderTargetSize == FIntPoint::ZeroValue || RenderTargets.Num() == 0)
	{
		if (!IsInitialized())
		{
			InitResource();
		}

		ResizeTargets(NewSize);
	}
}

void FUIPostProcessResource::ResizeTargets(const FIntPoint& NewSize)
{
	check(IsInRenderingThread());

	RenderTargets.Empty();

	RenderTargetSize = NewSize;
	PixelFormat = PF_B8G8R8A8;
	if (RenderTargetSize.X > 0 && RenderTargetSize.Y > 0)
	{
		for (int32 TexIndex = 0; TexIndex < RenderTargetCount; ++TexIndex)
		{
			FTexture2DRHIRef RenderTargetTextureRHI;
			FTexture2DRHIRef ShaderResourceUnused;
			FRHIResourceCreateInfo CreateInfo;
			RHICreateTargetableShaderResource2D(
				RenderTargetSize.X,
				RenderTargetSize.Y,
				PixelFormat,
				1,
				TexCreate_None,
				TexCreate_RenderTargetable,
				/*bNeedsTwoCopies=*/false,
				CreateInfo,
				RenderTargetTextureRHI,
				ShaderResourceUnused
			);

			RenderTargets.Add(RenderTargetTextureRHI);
		}
	}
}

void FUIPostProcessResource::CleanUp()
{
	BeginReleaseResource(this);
	BeginCleanup(this);
}

void FUIPostProcessResource::InitDynamicRHI()
{
	
}

void FUIPostProcessResource::ReleaseDynamicRHI()
{
	RenderTargetSize = FIntPoint::ZeroValue;
	RenderTargets.Empty();
}
