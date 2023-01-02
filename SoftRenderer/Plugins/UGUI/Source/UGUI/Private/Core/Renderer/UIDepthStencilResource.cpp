#include "Core/Renderer/UIDepthStencilResource.h"

FUIDepthStencilResource::FUIDepthStencilResource()
	: DepthStencilSize(FIntPoint::ZeroValue)
{
}

FUIDepthStencilResource::~FUIDepthStencilResource()
{

}

void FUIDepthStencilResource::Update(const FIntPoint& NewSize)
{
	if (NewSize != DepthStencilSize)
	{
		if (!IsInitialized())
		{
			InitResource();
		}

		ResizeTargets(NewSize);
	}
}

void FUIDepthStencilResource::ResizeTargets(const FIntPoint& NewSize)
{
	check(IsInRenderingThread());

	DepthStencilSize = NewSize;
	DepthStencil.SafeRelease();
	
	if (DepthStencilSize.X > 0 && DepthStencilSize.Y > 0)
	{
		FTexture2DRHIRef ShaderResourceUnused;
		FRHIResourceCreateInfo CreateInfo(FClearValueBinding::DepthFar);

		constexpr ETextureCreateFlags TargetableTextureFlags = TexCreate_DepthStencilTargetable | TexCreate_Memoryless;
		RHICreateTargetableShaderResource2D(DepthStencilSize.X, DepthStencilSize.Y, PF_DepthStencil, 1, TexCreate_None,
			TargetableTextureFlags, false, CreateInfo, DepthStencil, ShaderResourceUnused);
		check(IsValidRef(DepthStencil));
	}
}

void FUIDepthStencilResource::CleanUp()
{
	BeginReleaseResource(this);
	BeginCleanup(this);
}

void FUIDepthStencilResource::InitDynamicRHI()
{

}

void FUIDepthStencilResource::ReleaseDynamicRHI()
{
	DepthStencilSize = FIntPoint::ZeroValue;
	DepthStencil.SafeRelease();
}
