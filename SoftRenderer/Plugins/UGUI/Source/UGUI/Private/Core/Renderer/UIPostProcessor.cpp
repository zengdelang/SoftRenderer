#include "Core/Renderer/UIPostProcessor.h"
#include "ScreenRendering.h"
#include "Core/Renderer/UIPostProcessResource.h"
#include "CommonRenderResources.h"
#include "PixelShaderUtils.h"
#include "Core/Renderer/UIPostProcessShaders.h"
#include "Core/Renderer/UIVertex.h"
#include "Renderer/Private/PostProcess/SceneFilterRendering.h"
#include "UGUI.h"

FUIPostProcessor::FUIPostProcessor()
{
	IntermediateTargets = FUIPostProcessResource::GetUIPostProcessResource();
}

FUIPostProcessor::~FUIPostProcessor()
{
	IntermediateTargets->ReleaseUIPostProcessResource();
}

void FUIPostProcessor::ReleaseRenderTargets() const
{

}

void FUIPostProcessor::DownsampleRect(FRHICommandListImmediate& RHICmdList, const FUIPostProcessRectParams& Params, const FIntPoint& DownsampleSize, int32& UIBatches, int32& PostProcessUIBatches)
{
	SCOPED_DRAW_EVENT(RHICmdList, UGUIPostProcessDownsample);

	// Source is the viewport.  This is the width and height of the viewport backbuffer
	const int32 SrcTextureWidth = Params.SourceTextureSize.X;
	const int32 SrcTextureHeight = Params.SourceTextureSize.Y;

	// Dest is the destination quad for the downsample
	const int32 DestTextureWidth = IntermediateTargets->GetWidth();
	const int32 DestTextureHeight = IntermediateTargets->GetHeight();

	// Rect of the final destination post process effect (not downsample rect).  This is the area we sample from
	const FUIRect& DestRect = Params.DestRect;

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FUIScreenVS> VertexShader(ShaderMap);

	FSamplerStateRHIRef BilinearClamp = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FTexture2DRHIRef DestTexture = IntermediateTargets->GetRenderTarget(0);

	// Downsample and store in intermediate texture
	{
		TShaderMapRef<FUIPostProcessDownsamplePS> PixelShader(ShaderMap);

		RHICmdList.Transition(FRHITransitionInfo(Params.SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

		const FVector2D InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);

		const FVector2D UVStart = FVector2D(DestRect.Left, DestRect.Top) * InvSrcTextureSize;
		const FVector2D UVEnd = FVector2D(DestRect.Right, DestRect.Bottom) * InvSrcTextureSize;
		const FVector2D SizeUV = UVEnd - UVStart;

		RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);
		RHICmdList.SetScissorRect(false, 0, 0, 0, 0);

		FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIDownsampleRect"));
		{
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			PixelShader->SetShaderParams(RHICmdList, FVector4(InvSrcTextureSize.X, InvSrcTextureSize.Y, 0, 0));
			PixelShader->SetUVBounds(RHICmdList, FVector4(UVStart, UVEnd));
			PixelShader->SetTexture(RHICmdList, Params.SourceTexture, BilinearClamp);

			++UIBatches;
			++PostProcessUIBatches;
			
			DrawRectangle(
				RHICmdList,
				0, 0,
				DownsampleSize.X, DownsampleSize.Y,
				UVStart.X, UVStart.Y,
				SizeUV.X, SizeUV.Y,
				FIntPoint(DestTextureWidth, DestTextureHeight),
				FIntPoint(1, 1),
				VertexShader);
		}
		RHICmdList.EndRenderPass();
	}
}

void FUIPostProcessor::UpsampleRect(FRHICommandListImmediate& RHICmdList, const FUIPostProcessRectParams& Params, const FIntPoint& DownsampleSize, FSamplerStateRHIRef& Sampler,
	const FMatrix& LocalToWorldMatrix, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32 SrcTextureIndex)
{
	SCOPED_DRAW_EVENT(RHICmdList, UGUIPostProcessUpsample);

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	// use BLEND_Translucent for anti-aliasing
	GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	// Original source texture is now the destination texture
	FTexture2DRHIRef DestTexture = Params.SourceTexture;
	const int32 DestTextureWidth = Params.SourceTextureSize.X;
	const int32 DestTextureHeight = Params.SourceTextureSize.Y;

	const int32 DownsampledWidth = DownsampleSize.X;
	const int32 DownsampledHeight = DownsampleSize.Y;

	// Source texture is the texture that was originally downsampled
	FTexture2DRHIRef SrcTexture = IntermediateTargets->GetRenderTarget(SrcTextureIndex);
	const int32 SrcTextureWidth = IntermediateTargets->GetWidth();
	const int32 SrcTextureHeight = IntermediateTargets->GetHeight();

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FUIPostProcessElementVS> VertexShader(ShaderMap);

	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);

	// Perform Writable transitions first
	RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

	FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIUpsampleRect"));
	{
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		TShaderMapRef<FUIPostProcessElementPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetUIScreenVertexDeclaration();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		const float SizeU = (DownsampledWidth == SrcTextureWidth) ? 1.0f : (DownsampledWidth / (float)SrcTextureWidth) - (1.0f / (float)SrcTextureWidth);
		const float SizeV = (DownsampledHeight == SrcTextureHeight) ? 1.0f : (DownsampledHeight / (float)SrcTextureHeight) - (1.0f / (float)SrcTextureHeight);
		VertexShader->SetUVScaleOffset(RHICmdList, FVector4(RectTopLeftUV, FVector2D(SizeU, SizeV) / (RectBottomRightUV - RectTopLeftUV)));
		VertexShader->SetParameters(RHICmdList, ViewProjectionMatrix, LocalToWorldMatrix);

		PixelShader->SetParameters(RHICmdList, Sampler, SrcTexture);
	}

	//RHICmdList.EndRenderPass();
}

void FUIPostProcessor::DrawRectangle(FRHICommandList& RHICmdList, float X, float Y, float SizeX, float SizeY, float U,
	float V, float SizeU, float SizeV, FIntPoint TargetSize, FIntPoint TextureSize,
	const TShaderRefBase<FShader, FShaderMapPointerTable>& VertexShader)
{
	// Set up vertex uniform parameters for scaling and biasing the rectangle.
	// Note: Use DrawRectangle in the vertex shader to calculate the correct vertex position and uv.

	FDrawRectangleParameters Parameters;
	Parameters.PosScaleBias = FVector4(SizeX, SizeY, X, Y);
	Parameters.UVScaleBias = FVector4(SizeU, SizeV, U, V);

	Parameters.InvTargetSizeAndTextureSize = FVector4(
		1.0f / TargetSize.X, 1.0f / TargetSize.Y,
		1.0f / TextureSize.X, 1.0f / TextureSize.Y);

	SetUniformBufferParameterImmediate(RHICmdList, VertexShader.GetVertexShader(), VertexShader->GetUniformBufferParameter<FDrawRectangleParameters>(), Parameters);

	FPixelShaderUtils::DrawFullscreenQuad(RHICmdList, 1);
}
