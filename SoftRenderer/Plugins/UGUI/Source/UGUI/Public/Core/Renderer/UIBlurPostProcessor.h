#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "Core/UICommonDefinitions.h"

class FUIPostProcessResource;

struct FUIBlurPostProcessRectParams
{
	FTexture2DRHIRef SourceTexture;
	FUIRect SourceRect;
	FUIRect DestRect;
	FIntPoint SourceTextureSize;
};

struct FUIBlurRectParams
{
	int32 KernelSize;
	int32 DownsampleAmount;
	float Strength;
};

struct FUIBlurGraphicDataParams
{
	float bRectClipping;
	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;
};

class FUIBlurPostProcessor
{
public:
	FUIBlurPostProcessor();
	virtual ~FUIBlurPostProcessor();

public:
	bool BlurRect(FRHICommandListImmediate& RHICmdList, const FUIBlurRectParams& Params, const FUIBlurPostProcessRectParams& RectParams, const FUIBlurGraphicDataParams& GraphicDataParams, 
		class FUISceneProxy* InSceneProxy, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32& UIBatches, int32& PostProcessUIBatches);
	void ReleaseRenderTargets() const;

private:
	void DownsampleRect(FRHICommandListImmediate& RHICmdList, const FUIBlurPostProcessRectParams& Params, const FIntPoint& DownsampleSize, int32& UIBatches, int32& PostProcessUIBatches);
	void UpsampleRect(FRHICommandListImmediate& RHICmdList, const FUIBlurPostProcessRectParams& Params, const FUIBlurGraphicDataParams& GraphicDataParams, const FIntPoint& DownsampleSize, 
		FSamplerStateRHIRef& Sampler, class FUISceneProxy* InSceneProxy, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV);
	static int32 ComputeBlurWeights(int32 KernelSize, float StdDev, TArray<FVector4>& OutWeightsAndOffsets);

private:
	virtual void DrawRectangle(
		FRHICommandList& RHICmdList,
		float X,
		float Y,
		float SizeX,
		float SizeY,
		float U,
		float V,
		float SizeU,
		float SizeV,
		FIntPoint TargetSize,
		FIntPoint TextureSize,
		const TShaderRefBase<FShader, FShaderMapPointerTable>& VertexShader
	);
	
private:
	FUIPostProcessResource* IntermediateTargets;
	
};
