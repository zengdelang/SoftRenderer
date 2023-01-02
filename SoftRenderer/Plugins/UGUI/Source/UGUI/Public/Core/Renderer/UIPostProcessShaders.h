#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShader.h"

class FUIPostProcessElementShader :public FGlobalShader
{
	DECLARE_TYPE_LAYOUT(FUIPostProcessElementShader, NonVirtual);
	
public:
	/** Indicates that this shader should be cached */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FUIPostProcessElementShader()
	{
	}

	/** Constructor.  Binds all parameters used by the shader */
	FUIPostProcessElementShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		TextureParameter.Bind(Initializer.ParameterMap, TEXT("ElementTexture"));
		TextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("ElementTextureSampler"));
		MaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("MaskTexture"));
		MaskTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("MaskTextureSampler"));
		ShaderParams.Bind(Initializer.ParameterMap, TEXT("ShaderParams"));
	}

	/**
	 * Sets the texture used by this shader
	 *
	 * @param  InTexture  Texture resource to use when this pixel shader is bound
	 * @param  SamplerState  Sampler state to use when sampling this texture
	 */
	void SetTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), TextureParameter, TextureParameterSampler, SamplerState, InTexture);
	}

	/**
	 * Sets the mask texture used by this shader
	 *
	 * @param  InTexture  Texture resource to use when this pixel shader is bound
	 * @param  SamplerState  Sampler state to use when sampling this texture
	 */
	void SetMaskTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MaskTextureParameter, MaskTextureParameterSampler, SamplerState, InTexture);
	}

	/**
	 * Sets shader params used by the shader
	 *
	 * @param  InShaderParams  Shader params to use
	 */
	void SetShaderParams(FRHICommandList& RHICmdList, const FVector4& InShaderParams)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ShaderParams, InShaderParams);
	}

private:
	/** Texture parameter used by the shader */
	LAYOUT_FIELD(FShaderResourceParameter, TextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, TextureParameterSampler);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameterSampler);
	LAYOUT_FIELD(FShaderParameter, ShaderParams);
	
};

class FUIPostProcessDownsamplePS : public FUIPostProcessElementShader
{
	DECLARE_SHADER_TYPE(FUIPostProcessDownsamplePS, Global);

public:
	/** Indicates that this shader should be cached */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FUIPostProcessDownsamplePS()
	{
	}

	/** Constructor.  Binds all parameters used by the shader */
	FUIPostProcessDownsamplePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUIPostProcessElementShader(Initializer)
	{
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4& InUVBounds)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), UVBounds, InUVBounds);
	}

private:
	LAYOUT_FIELD(FShaderParameter, UVBounds);

};

/**
* A vertex shader for rendering a textured screen element.
*/
class FUIScreenVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUIScreenVS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FUIScreenVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
	}
	FUIScreenVS() {}

	void SetParameters(FRHICommandList& RHICmdList, FRHIUniformBuffer* ViewUniformBuffer)
	{
		FGlobalShader::SetParameters<FViewUniformShaderParameters>(RHICmdList, RHICmdList.GetBoundVertexShader(), ViewUniformBuffer);
	}
};

class FUIPostProcessElementVS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUIPostProcessElementVS, Global);

public:
	FUIPostProcessElementVS() {}
	FUIPostProcessElementVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		ViewProjectionParameter.Bind(Initializer.ParameterMap, TEXT("ViewProjectionMatrix"));
		LocalToWorldMatrixParameter.Bind(Initializer.ParameterMap, TEXT("LocalToWorldMatrix"));
		UVScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("UVScaleOffset"));
	}

	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix& ViewProjectionMatrix, const FMatrix& LocalToWorldMatrix)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), ViewProjectionParameter, ViewProjectionMatrix);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), LocalToWorldMatrixParameter, LocalToWorldMatrix);
	}

	void SetUVScaleOffset(FRHICommandList& RHICmdList, const FVector4& InUVScaleOffset)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), UVScaleOffsetParameter, InUVScaleOffset);
	}

private:
	LAYOUT_FIELD(FShaderParameter, ViewProjectionParameter);
	LAYOUT_FIELD(FShaderParameter, LocalToWorldMatrixParameter);
	LAYOUT_FIELD(FShaderParameter, UVScaleOffsetParameter);

};

class FUIPostProcessElementPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUIPostProcessElementPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FUIPostProcessElementPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"), SPF_Mandatory);
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
	}

	FUIPostProcessElementPS() {}

	void SetParameters(FRHICommandList& RHICmdList, const FTexture* Texture)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InTexture, InTextureSampler, Texture);
	}

	void SetParameters(FRHICommandList& RHICmdList, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InTexture, InTextureSampler, SamplerStateRHI, TextureRHI);
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, InTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InTextureSampler);

};

//////////////////////////////////////////// Blur Start ////////////////////////////////////////////
const int32 MAX_BLUR_SAMPLES = 127;

class FUIBlurPostProcessBlurPS : public FUIPostProcessElementShader
{
	DECLARE_SHADER_TYPE(FUIBlurPostProcessBlurPS, Global);
	
public:
	/** Indicates that this shader should be cached */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FUIBlurPostProcessBlurPS()
	{
	}

	/** Constructor.  Binds all parameters used by the shader */
	FUIBlurPostProcessBlurPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUIPostProcessElementShader(Initializer)
	{
		BufferSizeAndDirection.Bind(Initializer.ParameterMap, TEXT("BufferSizeAndDirection"));
		WeightAndOffsets.Bind(Initializer.ParameterMap, TEXT("WeightAndOffsets"));
		SampleCount.Bind(Initializer.ParameterMap, TEXT("SampleCount"));
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
	}

	void SetBufferSizeAndDirection(FRHICommandList& RHICmdList, const FVector2D& InBufferSize, const FVector2D& InDir)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), BufferSizeAndDirection, FVector4(InBufferSize, InDir));
	}

	void SetWeightsAndOffsets(FRHICommandList& RHICmdList, const TArray<FVector4>& InWeightsAndOffsets, int32 NumSamples)
	{
		check(InWeightsAndOffsets.Num() <= MAX_BLUR_SAMPLES);
		SetShaderValueArray<FRHIPixelShader*, FVector4>(RHICmdList, RHICmdList.GetBoundPixelShader(), WeightAndOffsets, InWeightsAndOffsets.GetData(), InWeightsAndOffsets.Num());
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SampleCount, NumSamples);
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4& InUVBounds)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), UVBounds, InUVBounds);
	}

private:
	LAYOUT_FIELD(FShaderParameter, BufferSizeAndDirection);
	LAYOUT_FIELD(FShaderParameter, WeightAndOffsets);
	LAYOUT_FIELD(FShaderParameter, SampleCount);
	LAYOUT_FIELD(FShaderParameter, UVBounds);
	
};


class FUIBlurPostProcessElementVS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUIBlurPostProcessElementVS, Global);
	
public:
	FUIBlurPostProcessElementVS() {}
	FUIBlurPostProcessElementVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		ViewProjectionParameter.Bind(Initializer.ParameterMap, TEXT("ViewProjectionMatrix"));
		LocalToWorldMatrixParameter.Bind(Initializer.ParameterMap, TEXT("LocalToWorldMatrix"));
		UVScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("UVScaleOffset"));
	}
	
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix& ViewProjectionMatrix, const FMatrix& LocalToWorldMatrix)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), ViewProjectionParameter, ViewProjectionMatrix);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), LocalToWorldMatrixParameter, LocalToWorldMatrix);
	}

	void SetUVScaleOffset(FRHICommandList& RHICmdList, const FVector4& InUVScaleOffset)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), UVScaleOffsetParameter, InUVScaleOffset);
	}
	
private:
	LAYOUT_FIELD(FShaderParameter, ViewProjectionParameter);
	LAYOUT_FIELD(FShaderParameter, LocalToWorldMatrixParameter);
	LAYOUT_FIELD(FShaderParameter, UVScaleOffsetParameter);
	
};


class FUIBlurPostProcessElementPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FUIBlurPostProcessElementPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }

	FUIBlurPostProcessElementPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) :
		FGlobalShader(Initializer)
	{
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"), SPF_Optional);
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
		bRectClippingParameter.Bind(Initializer.ParameterMap, TEXT("bRectClipping"));
		ClipRectParameter.Bind(Initializer.ParameterMap, TEXT("ClipRect"));
		ClipSoftnessRectParameter.Bind(Initializer.ParameterMap, TEXT("ClipSoftnessRect"), SPF_Optional);
		InMaskTexture.Bind(Initializer.ParameterMap, TEXT("MaskTexture"), SPF_Optional);
		InMaskTextureSampler.Bind(Initializer.ParameterMap, TEXT("MaskTextureSampler"));
	}
	
	FUIBlurPostProcessElementPS() {}

	void SetParameters(FRHICommandList& RHICmdList, const FTexture* Texture)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InTexture, InTextureSampler, Texture);
	}

	void SetParameters(FRHICommandList& RHICmdList, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InTexture, InTextureSampler, SamplerStateRHI, TextureRHI);
	}

	void SetRectClipParameters(FRHICommandList& RHICmdList, const float& InbRectClipping, const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), bRectClippingParameter, InbRectClipping);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ClipRectParameter, InClipRect);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ClipSoftnessRectParameter, InClipSoftnessRect);
	}

	void SetMaskParameters(FRHICommandList& RHICmdList, const FTexture* Texture)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InMaskTexture, InMaskTextureSampler, Texture);
	}

	void SetMaskParameters(FRHICommandList& RHICmdList, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), InMaskTexture, InMaskTextureSampler, SamplerStateRHI, TextureRHI);
	}
	
private:
	LAYOUT_FIELD(FShaderResourceParameter, InTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InTextureSampler);
	LAYOUT_FIELD(FShaderParameter, bRectClippingParameter);
	LAYOUT_FIELD(FShaderParameter, ClipRectParameter);
	LAYOUT_FIELD(FShaderParameter, ClipSoftnessRectParameter);
	LAYOUT_FIELD(FShaderResourceParameter, InMaskTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InMaskTextureSampler);
	
};
//////////////////////////////////////////// Blur End ////////////////////////////////////////////


//////////////////////////////////////////// Glitch Start ////////////////////////////////////////////
class FUIGlitchPostProcessGlitchPS : public FUIPostProcessElementShader
{
	DECLARE_SHADER_TYPE(FUIGlitchPostProcessGlitchPS, Global);

public:
	/** Indicates that this shader should be cached */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	FUIGlitchPostProcessGlitchPS()
	{
	}

	/** Constructor.  Binds all parameters used by the shader */
	FUIGlitchPostProcessGlitchPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FUIPostProcessElementShader(Initializer)
	{
		UVBounds.Bind(Initializer.ParameterMap, TEXT("UVBounds"));
		Strength.Bind(Initializer.ParameterMap, TEXT("Strength"));
		TimeEtc.Bind(Initializer.ParameterMap, TEXT("TimeEtc"));
		GlitchParams1.Bind(Initializer.ParameterMap, TEXT("GlitchParams1"));
		GlitchParams2.Bind(Initializer.ParameterMap, TEXT("GlitchParams2"));
	}

	void SetUVBounds(FRHICommandList& RHICmdList, const FVector4& InUVBounds)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), UVBounds, InUVBounds);
	}

	void SetStrength(FRHICommandList& RHICmdList, const float InStrength)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), Strength, InStrength);
	}

	void SetTimeEtc(FRHICommandList& RHICmdList, const FVector4& InTimeEtc)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), TimeEtc, InTimeEtc);
	}

	void SetGlitchParams1(FRHICommandList& RHICmdList, const FVector4& InGlitchParams1)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), GlitchParams1, InGlitchParams1);
	}

	void SetGlitchParams2(FRHICommandList& RHICmdList, const FVector4& InGlitchParams2)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), GlitchParams2, InGlitchParams2);
	}

private:

	LAYOUT_FIELD(FShaderParameter, UVBounds);
	LAYOUT_FIELD(FShaderParameter, Strength);
	LAYOUT_FIELD(FShaderParameter, TimeEtc);
	LAYOUT_FIELD(FShaderParameter, GlitchParams1);
	LAYOUT_FIELD(FShaderParameter, GlitchParams2);


};

//////////////////////////////////////////// Glitch End ////////////////////////////////////////////
