#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "Core/UICommonDefinitions.h"

class FUIPostProcessResource;

struct FUIPostProcessRectParams
{
	FTexture2DRHIRef SourceTexture;
	FUIRect SourceRect;
	FUIRect DestRect;
	FIntPoint SourceTextureSize;
};

class FUIPostProcessor
{
public:
	FUIPostProcessor();
	virtual ~FUIPostProcessor();

public:
	void ReleaseRenderTargets() const;

protected:
	void DownsampleRect(FRHICommandListImmediate& RHICmdList, const FUIPostProcessRectParams& Params, const FIntPoint& DownsampleSize, int32& UIBatches, int32& PostProcessUIBatches);
	void UpsampleRect(FRHICommandListImmediate& RHICmdList, const FUIPostProcessRectParams& Params, const FIntPoint& DownsampleSize, 
		FSamplerStateRHIRef& Sampler, const FMatrix& LocalToWorldMatrix, const FMatrix& ViewProjectionMatrix
		, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32 SrcTextureIndex);

	void DrawRectangle(
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
	
protected:
	FUIPostProcessResource* IntermediateTargets;
	
};
