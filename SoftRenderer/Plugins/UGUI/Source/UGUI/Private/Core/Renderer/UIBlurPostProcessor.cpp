#include "Core/Renderer/UIBlurPostProcessor.h"
#include "ScreenRendering.h"
#include "CommonRenderResources.h"
#include "PixelShaderUtils.h"
#include "Core/Renderer/UIPostProcessShaders.h"
#include "Core/Renderer/UIVertex.h"
#include "Renderer/Private/PostProcess/SceneFilterRendering.h"
#include "UGUI.h"
#include "Core/Renderer/UIPostProcessResource.h"
#include "Core/Renderer/UISceneProxy.h"

FUIBlurPostProcessor::FUIBlurPostProcessor()
{
	IntermediateTargets = FUIPostProcessResource::GetUIPostProcessResource();
}

FUIBlurPostProcessor::~FUIBlurPostProcessor()
{
	IntermediateTargets->ReleaseUIPostProcessResource();
}

DECLARE_CYCLE_STAT(TEXT("UIRender --- GaussianBlurRect"), STAT_UnrealGUI_BlurRect, STATGROUP_UnrealGUI);
bool FUIBlurPostProcessor::BlurRect(FRHICommandListImmediate& RHICmdList, const FUIBlurRectParams& Params, const FUIBlurPostProcessRectParams& RectParams, const FUIBlurGraphicDataParams& GraphicDataParams,
	FUISceneProxy* InSceneProxy, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32& UIBatches, int32& PostProcessUIBatches)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_BlurRect);
	
	check(RHICmdList.IsOutsideRenderPass());

	TArray<FVector4> WeightsAndOffsets;
	const int32 SampleCount = ComputeBlurWeights(Params.KernelSize, Params.Strength, WeightsAndOffsets);

	const bool bDownsample = Params.DownsampleAmount > 0;
	
	FIntPoint DestRectSize = RectParams.DestRect.GetSize().IntPoint();
	FIntPoint RequiredSize = bDownsample
		? FIntPoint(FMath::DivideAndRoundUp(DestRectSize.X, Params.DownsampleAmount), FMath::DivideAndRoundUp(DestRectSize.Y, Params.DownsampleAmount))
		: DestRectSize;

	// The max size can get ridiculous with large scale values.  Clamp to size of the backbuffer
	RequiredSize.X = FMath::Min(RequiredSize.X, RectParams.SourceTextureSize.X);
	RequiredSize.Y = FMath::Min(RequiredSize.Y, RectParams.SourceTextureSize.Y);

	SCOPED_DRAW_EVENTF(RHICmdList, UGUIPostProcess, TEXT("UGUI Post Process Blur Background Kernel: %dx%d Size: %dx%d"), SampleCount, SampleCount, RequiredSize.X, RequiredSize.Y);

	const FIntPoint DownsampleSize = RequiredSize;

	IntermediateTargets->Update(RequiredSize);

	if (IntermediateTargets->GetRenderTargetNum() == 0)
	{
		return false;
	}

	if (bDownsample)
	{
		DownsampleRect(RHICmdList, RectParams, DownsampleSize, UIBatches, PostProcessUIBatches);
	}

	FSamplerStateRHIRef BilinearClamp = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

#if 1
	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	check(ShaderMap);

	TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
	TShaderMapRef<FUIBlurPostProcessBlurPS> PixelShader(ShaderMap);

	const int32 SrcTextureWidth = RectParams.SourceTextureSize.X;
	const int32 SrcTextureHeight = RectParams.SourceTextureSize.Y;

	const int32 DestTextureWidth = IntermediateTargets->GetWidth(); 
	const int32 DestTextureHeight = IntermediateTargets->GetHeight();

	const FUIRect& SourceRect = RectParams.SourceRect;
	const FUIRect& DestRect = RectParams.DestRect;

	FVertexDeclarationRHIRef VertexDecl = GFilterVertexDeclaration.VertexDeclarationRHI;
	check(IsValidRef(VertexDecl));

	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

	RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);

	const FVector2D InvBufferSize = FVector2D(1.0f / DestTextureWidth, 1.0f / DestTextureHeight);
	const FVector2D HalfTexelOffset = FVector2D(0.5f / DestTextureWidth, 0.5f / DestTextureHeight);

	for (int32 PassIndex = 0; PassIndex < 2; ++PassIndex)
	{
		// First pass render to the render target with the post process fx
		if (PassIndex == 0)
		{
			FTexture2DRHIRef SourceTexture = bDownsample ? IntermediateTargets->GetRenderTarget(0) : RectParams.SourceTexture;
			FTexture2DRHIRef DestTexture = IntermediateTargets->GetRenderTarget(1);

			RHICmdList.Transition(FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
			RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

			FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
			RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIBlurRectPass0"));
			{
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				
				if (bDownsample)
				{
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = PT_TriangleList;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

					PixelShader->SetWeightsAndOffsets(RHICmdList, WeightsAndOffsets, SampleCount);
					PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);
					PixelShader->SetUVBounds(RHICmdList, FVector4(FVector2D::ZeroVector, FVector2D((float)DownsampleSize.X / DestTextureWidth, (float)DownsampleSize.Y / DestTextureHeight) - HalfTexelOffset));
					PixelShader->SetBufferSizeAndDirection(RHICmdList, InvBufferSize, FVector2D(1, 0));

					++UIBatches;
					++PostProcessUIBatches;
					
					DrawRectangle(
						RHICmdList,
						0, 0,
						DownsampleSize.X, DownsampleSize.Y,
						0, 0,
						DownsampleSize.X, DownsampleSize.Y,
						FIntPoint(DestTextureWidth, DestTextureHeight),
						FIntPoint(DestTextureWidth, DestTextureHeight),
						VertexShader);
				}
				else
				{
					TShaderMapRef<FUIScreenVS> UIScreenVertexShader(ShaderMap);
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = UIScreenVertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = PT_TriangleList;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

					PixelShader->SetWeightsAndOffsets(RHICmdList, WeightsAndOffsets, SampleCount);
					PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);
					
					const FVector2D InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);

					const FVector2D UVStart = FVector2D(DestRect.Left, DestRect.Top) * InvSrcTextureSize;
					const FVector2D UVEnd = FVector2D(DestRect.Right, DestRect.Bottom) * InvSrcTextureSize;
					const FVector2D SizeUV = UVEnd - UVStart;

					PixelShader->SetUVBounds(RHICmdList, FVector4(UVStart, UVEnd));
					PixelShader->SetBufferSizeAndDirection(RHICmdList, InvSrcTextureSize, FVector2D(1, 0));

					++UIBatches;
					++PostProcessUIBatches;
					
					DrawRectangle(
						RHICmdList,
						0, 0,
						RequiredSize.X, RequiredSize.Y,
						UVStart.X, UVStart.Y,
						SizeUV.X, SizeUV.Y,
						FIntPoint(DestTextureWidth, DestTextureHeight),
						FIntPoint(1, 1),
						UIScreenVertexShader);
				}
			}
			RHICmdList.EndRenderPass();
		}
		else
		{
			FTexture2DRHIRef SourceTexture = IntermediateTargets->GetRenderTarget(1);
			FTexture2DRHIRef DestTexture = IntermediateTargets->GetRenderTarget(0);

			RHICmdList.Transition(FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
			RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

			FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
			RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIBlurRect"));
			{
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				PixelShader->SetWeightsAndOffsets(RHICmdList, WeightsAndOffsets, SampleCount);
				PixelShader->SetUVBounds(RHICmdList, FVector4(FVector2D::ZeroVector, FVector2D((float)DownsampleSize.X / DestTextureWidth, (float)DownsampleSize.Y / DestTextureHeight) - HalfTexelOffset));
				PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);
				PixelShader->SetBufferSizeAndDirection(RHICmdList, InvBufferSize, FVector2D(0, 1));

				++UIBatches;
				++PostProcessUIBatches;
				
				DrawRectangle(
					RHICmdList,
					0, 0,
					DownsampleSize.X, DownsampleSize.Y,
					0, 0,
					DownsampleSize.X, DownsampleSize.Y,
					FIntPoint(DestTextureWidth, DestTextureHeight),
					FIntPoint(DestTextureWidth, DestTextureHeight),
					VertexShader);
			}
			RHICmdList.EndRenderPass();
		}
	}
#endif

	UpsampleRect(RHICmdList, RectParams, GraphicDataParams, DownsampleSize, BilinearClamp, InSceneProxy, ViewProjectionMatrix, RectTopLeftUV, RectBottomRightUV);

	return true;
}

void FUIBlurPostProcessor::ReleaseRenderTargets() const
{

}

void FUIBlurPostProcessor::DownsampleRect(FRHICommandListImmediate& RHICmdList, const FUIBlurPostProcessRectParams& Params, const FIntPoint& DownsampleSize, int32& UIBatches, int32& PostProcessUIBatches)
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

void FUIBlurPostProcessor::UpsampleRect(FRHICommandListImmediate& RHICmdList, const FUIBlurPostProcessRectParams& Params, const FUIBlurGraphicDataParams& GraphicDataParams, const FIntPoint& DownsampleSize, FSamplerStateRHIRef& Sampler, 
	FUISceneProxy* InSceneProxy, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV)
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

	// Clip Parameters
	const float bRectClipping = GraphicDataParams.bRectClipping;
	const FLinearColor ClipRect = GraphicDataParams.ClipRect;
	const FLinearColor ClipSoftnessRect = GraphicDataParams.ClipSoftnessRect;
	
	// Mask Parameters
	FTextureRHIRef MaskTextureRHIRef = GWhiteTexture->TextureRHI;
	FRHISamplerState* MaskTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	InSceneProxy->GetMainTextureRHIResource(MaskTextureRHIRef, MaskTextureSamplerState);
	
	const int32 DownsampledWidth = DownsampleSize.X;
	const int32 DownsampledHeight = DownsampleSize.Y;
	
	// Source texture is the texture that was originally downsampled
	FTexture2DRHIRef SrcTexture = IntermediateTargets->GetRenderTarget(0);
	const int32 SrcTextureWidth = IntermediateTargets->GetWidth();
	const int32 SrcTextureHeight = IntermediateTargets->GetHeight();
	
	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FUIBlurPostProcessElementVS> VertexShader(ShaderMap);

	RHICmdList.SetViewport(0, 0, 0, DestTextureWidth, DestTextureHeight, 0.0f);

	// Perform Writable transitions first
	RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
	RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

	FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIUpsampleRect"));
	{
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		TShaderMapRef<FUIBlurPostProcessElementPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetUIScreenVertexDeclaration();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		const float SizeU = (DownsampledWidth == SrcTextureWidth) ? 1.0f : (DownsampledWidth / (float)SrcTextureWidth) - (1.0f / (float)SrcTextureWidth);
		const float SizeV = (DownsampledHeight == SrcTextureHeight) ? 1.0f : (DownsampledHeight / (float)SrcTextureHeight) - (1.0f / (float)SrcTextureHeight);
		VertexShader->SetUVScaleOffset(RHICmdList, FVector4(RectTopLeftUV, FVector2D(SizeU, SizeV) / (RectBottomRightUV - RectTopLeftUV)));
		VertexShader->SetParameters(RHICmdList, ViewProjectionMatrix, InSceneProxy->GetLocalToWorldMatrix());
		
		PixelShader->SetParameters(RHICmdList, Sampler, SrcTexture);
		PixelShader->SetRectClipParameters(RHICmdList, bRectClipping, ClipRect, ClipSoftnessRect);

		PixelShader->SetMaskParameters(RHICmdList, MaskTextureSamplerState, MaskTextureRHIRef);
	}
	
	//RHICmdList.EndRenderPass();
}

#define UGUI_BILINEAR_FILTER_METHOD 1

#if !UGUI_BILINEAR_FILTER_METHOD

static int32 ComputeWeights(int32 KernelSize, float Sigma, TArray<FVector4>& OutWeightsAndOffsets)
{
	OutWeightsAndOffsets.AddUninitialized(KernelSize / 2 + 1);

	int32 SampleIndex = 0;
	for (int32 X = 0; X < KernelSize; X += 2)
	{
		float Dist = X;
		FVector4 WeightAndOffset;
		WeightAndOffset.X = (1.0f / FMath::Sqrt(2 * PI * Sigma * Sigma)) * FMath::Exp(-(Dist * Dist) / (2 * Sigma * Sigma));
		WeightAndOffset.Y = Dist;

		Dist = X + 1;
		WeightAndOffset.Z = (1.0f / FMath::Sqrt(2 * PI * Sigma * Sigma)) * FMath::Exp(-(Dist * Dist) / (2 * Sigma * Sigma));
		WeightAndOffset.W = Dist;

		OutWeightsAndOffsets[SampleIndex] = WeightAndOffset;

		++SampleIndex;
	}

	return KernelSize;
};

#else

static float GetWeight(float Dist, float Strength)
{
	// from https://en.wikipedia.org/wiki/Gaussian_blur
	const float Strength2 = Strength * Strength;
	return (1.0f / FMath::Sqrt(2 * PI * Strength2)) * FMath::Exp(-(Dist * Dist) / (2 * Strength2));
}

static FVector2D GetWeightAndOffset(float Dist, float Sigma)
{
	const float Offset1 = Dist;
	const float Weight1 = GetWeight(Offset1, Sigma);

	const float Offset2 = Dist + 1;
	const float Weight2 = GetWeight(Offset2, Sigma);

	const float TotalWeight = Weight1 + Weight2;

	float Offset = 0;
	if (TotalWeight > 0)
	{
		Offset = (Weight1 * Offset1 + Weight2 * Offset2) / TotalWeight;
	}

	return FVector2D(TotalWeight, Offset);
}

static int32 ComputeWeights(int32 KernelSize, float Sigma, TArray<FVector4>& OutWeightsAndOffsets)
{
	const int32 NumSamples = FMath::DivideAndRoundUp(KernelSize, 2);

	// We need half of the sample count array because we're packing two samples into one float4
	OutWeightsAndOffsets.AddUninitialized(NumSamples % 2 == 0 ? NumSamples / 2 : NumSamples / 2 + 1);
	OutWeightsAndOffsets[0] = FVector4(FVector2D(GetWeight(0, Sigma), 0), GetWeightAndOffset(1, Sigma));
	
	int32 SampleIndex = 1;
	for (int32 X = 3; X < KernelSize; X += 4)
	{
		OutWeightsAndOffsets[SampleIndex] = FVector4(GetWeightAndOffset(X, Sigma), GetWeightAndOffset(X + 2, Sigma));
		++SampleIndex;
	}

	return NumSamples;
};

#endif

int32 FUIBlurPostProcessor::ComputeBlurWeights(int32 KernelSize, float StdDev, TArray<FVector4>& OutWeightsAndOffsets)
{
	return ComputeWeights(KernelSize, StdDev, OutWeightsAndOffsets);
}

void FUIBlurPostProcessor::DrawRectangle(FRHICommandList& RHICmdList, float X, float Y, float SizeX, float SizeY, float U,
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
