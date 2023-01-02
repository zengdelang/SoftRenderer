#include "Core/Renderer/UIGlitchPostProcessor.h"
#include "ScreenRendering.h"
#include "Core/Renderer/UIPostProcessResource.h"
#include "CommonRenderResources.h"
#include "PixelShaderUtils.h"
#include "Core/Renderer/UIPostProcessShaders.h"
#include "Renderer/Private/PostProcess/SceneFilterRendering.h"
#include "UGUI.h"

DECLARE_CYCLE_STAT(TEXT("UIRender --- GlitchBlurRect"), STAT_UnrealGUI_GlitchBlurRect, STATGROUP_UnrealGUI);
bool FUIGlitchPostProcessor::GlitchRect(FRHICommandListImmediate& RHICmdList, const FUIGlitchRectParams& Params, const FUIPostProcessRectParams& RectParams,
	const FMatrix& LocalToWorldMatrix, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32& UIBatches, int32& PostProcessUIBatches)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GlitchBlurRect);
	
	check(RHICmdList.IsOutsideRenderPass());

	const bool bDownsample = Params.DownSampleAmount > 1;
	
	FIntPoint DestRectSize = RectParams.DestRect.GetSize().IntPoint();
	FIntPoint RequiredSize = bDownsample
		? FIntPoint(FMath::DivideAndRoundUp(DestRectSize.X, Params.DownSampleAmount), FMath::DivideAndRoundUp(DestRectSize.Y, Params.DownSampleAmount))
		: DestRectSize;

	// The max size can get ridiculous with large scale values.  Clamp to size of the backbuffer
	RequiredSize.X = FMath::Min(RequiredSize.X, RectParams.SourceTextureSize.X);
	RequiredSize.Y = FMath::Min(RequiredSize.Y, RectParams.SourceTextureSize.Y);

	SCOPED_DRAW_EVENTF(RHICmdList, UGUIPostProcess, TEXT("UGUI Post Process Glitch Background Size: %dx%d"), RequiredSize.X, RequiredSize.Y);

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
	
	TShaderMapRef<FUIGlitchPostProcessGlitchPS> PixelShader(ShaderMap);

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

	{
		FTexture2DRHIRef SourceTexture = bDownsample ? IntermediateTargets->GetRenderTarget(0) : RectParams.SourceTexture;
		FTexture2DRHIRef DestTexture = IntermediateTargets->GetRenderTarget(1);

		RHICmdList.Transition(FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));
		RHICmdList.Transition(FRHITransitionInfo(DestTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

		FRHIRenderPassInfo RPInfo(DestTexture, ERenderTargetActions::Load_Store);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIGlitchRectPass0"));
		{
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			if (bDownsample)
			{
				TShaderMapRef<FUIScreenVS> VertexShader(ShaderMap);
				
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);
				PixelShader->SetMaskTexture(RHICmdList, Params.MaskTextureRHI, Params.MaskSamplerState);
				PixelShader->SetTimeEtc(RHICmdList, Params.TimeEtc);
				PixelShader->SetGlitchParams1(RHICmdList, Params.GlitchParams1);
				PixelShader->SetGlitchParams2(RHICmdList, Params.GlitchParams2);
				
				PixelShader->SetUVBounds(RHICmdList, FVector4(FVector2D::ZeroVector, FVector2D((float)DownsampleSize.X / DestTextureWidth, (float)DownsampleSize.Y / DestTextureHeight) - HalfTexelOffset));
				PixelShader->SetStrength(RHICmdList, Params.Strength);

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
				TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
				
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = VertexDecl;
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				PixelShader->SetTexture(RHICmdList, SourceTexture, BilinearClamp);
				PixelShader->SetMaskTexture(RHICmdList, Params.MaskTextureRHI, Params.MaskSamplerState);
				PixelShader->SetTimeEtc(RHICmdList, Params.TimeEtc);
				PixelShader->SetGlitchParams1(RHICmdList, Params.GlitchParams1);
				PixelShader->SetGlitchParams2(RHICmdList, Params.GlitchParams2);
				PixelShader->SetStrength(RHICmdList, Params.Strength);
				
				const FVector2D InvSrcTextureSize(1.f / SrcTextureWidth, 1.f / SrcTextureHeight);

				const FVector2D UVStart = FVector2D(DestRect.Left, DestRect.Top) * InvSrcTextureSize;
				const FVector2D UVEnd = FVector2D(DestRect.Right, DestRect.Bottom) * InvSrcTextureSize;
				const FVector2D SizeUV = UVEnd - UVStart;

				PixelShader->SetUVBounds(RHICmdList, FVector4(UVStart, UVEnd));

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
					VertexShader);
			}
		}
		RHICmdList.EndRenderPass();
	}

#endif

	UpsampleRect(RHICmdList, RectParams, DownsampleSize, BilinearClamp, LocalToWorldMatrix, ViewProjectionMatrix, RectTopLeftUV, RectBottomRightUV, 1);

	return true;
}