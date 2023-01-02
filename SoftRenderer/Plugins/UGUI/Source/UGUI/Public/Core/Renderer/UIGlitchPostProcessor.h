#pragma once

#include "CoreMinimal.h"
#include "RHI.h"
#include "UIPostProcessor.h"

class FUIPostProcessResource;

struct FUIGlitchRectParams
{
	// (RealTime, WorldTime, GlitchType, GlitchSpeed)
	FVector4 TimeEtc;
	int32 DownSampleAmount;
	FVector4 GlitchParams1;
	FVector4 GlitchParams2;
	FRHITexture* MaskTextureRHI;
	FRHISamplerState* MaskSamplerState;
	float Strength;
};

class FUIGlitchPostProcessor : public FUIPostProcessor
{
public:
	FUIGlitchPostProcessor() {};
	virtual ~FUIGlitchPostProcessor() override {};

public:
	bool GlitchRect(FRHICommandListImmediate& RHICmdList, const FUIGlitchRectParams& Params, const FUIPostProcessRectParams& RectParams, 
		const FMatrix& LocalToWorldMatrix, const FMatrix& ViewProjectionMatrix, const FVector2D& RectTopLeftUV, const FVector2D& RectBottomRightUV, int32& UIBatches, int32& PostProcessUIBatches);

};
