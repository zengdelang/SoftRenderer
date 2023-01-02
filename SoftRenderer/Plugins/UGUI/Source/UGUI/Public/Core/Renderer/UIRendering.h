#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "MeshMaterialShader.h"

struct FUIMeshMaterialShaderElementData : public FMeshMaterialShaderElementData
{
	float DisplayGamma;
	float SwitchVerticalAxisMultiplier;
};

class FUIMeshMaterialShaderVS :public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FUIMeshMaterialShaderVS, MeshMaterial);

	LAYOUT_FIELD(FShaderParameter, SwitchVerticalAxisMultiplier);
	
public:
	FUIMeshMaterialShaderVS() = default;
	FUIMeshMaterialShaderVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer) : FMeshMaterialShader(Initializer)
	{
		SwitchVerticalAxisMultiplier.Bind(Initializer.ParameterMap, TEXT("SwitchVerticalAxisMultiplier"));
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);

public:
	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FUIMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
		
		ShaderBindings.Add(SwitchVerticalAxisMultiplier, ShaderElementData.SwitchVerticalAxisMultiplier);
	}
	
};

class FUIMeshMaterialShaderPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FUIMeshMaterialShaderPS, MeshMaterial);

	LAYOUT_FIELD(FShaderParameter, GammaValues);
	LAYOUT_FIELD(FShaderParameter, SwitchVerticalAxisMultiplier);
	
public:
	FUIMeshMaterialShaderPS() = default;
	FUIMeshMaterialShaderPS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer) : FMeshMaterialShader(Initializer)
	{
		GammaValues.Bind(Initializer.ParameterMap, TEXT("GammaValues"));
		SwitchVerticalAxisMultiplier.Bind(Initializer.ParameterMap, TEXT("SwitchVerticalAxisMultiplier"));
	}

	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

public:
	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FUIMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const
	{
		FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);

		const FVector2D Values(2.2f / ShaderElementData.DisplayGamma, 1.0f / ShaderElementData.DisplayGamma);
		ShaderBindings.Add(GammaValues, Values);

		ShaderBindings.Add(SwitchVerticalAxisMultiplier, ShaderElementData.SwitchVerticalAxisMultiplier);
	}
};

class FUIMaterialShaderVS :public FMaterialShader
{
	DECLARE_SHADER_TYPE(FUIMaterialShaderVS, Material);
	
public:
	FUIMaterialShaderVS() {}
	FUIMaterialShaderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
		:FMaterialShader(Initializer)
	{
		SwitchVerticalAxisMultiplier.Bind(Initializer.ParameterMap, TEXT("SwitchVerticalAxisMultiplier"));
	}

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);

	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);

	/**
	 * Sets the vertical axis multiplier to use depending on graphics api
	 */
	void SetVerticalAxisMultiplier(FRHICommandList& RHICmdList, float InMultiplier) const
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundVertexShader(), SwitchVerticalAxisMultiplier, InMultiplier);
	}
	
private:
	/** Parameter used to determine if we need to switch the vertical axis for opengl */
	LAYOUT_FIELD(FShaderParameter, SwitchVerticalAxisMultiplier)
	
};

class FUIMaterialShaderPS : public FMaterialShader
{
	DECLARE_SHADER_TYPE(FUIMaterialShaderPS, Material);
	
public:
	FUIMaterialShaderPS() {}
	FUIMaterialShaderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
		:FMaterialShader(Initializer)
	{
		GammaValues.Bind(Initializer.ParameterMap, TEXT("GammaValues"));
		SwitchVerticalAxisMultiplier.Bind(Initializer.ParameterMap, TEXT("SwitchVerticalAxisMultiplier"));
		bRectClipping.Bind(Initializer.ParameterMap, TEXT("bRectClipping"));
		ClipRect.Bind(Initializer.ParameterMap, TEXT("ClipRect"));
		ClipSoftnessRect.Bind(Initializer.ParameterMap, TEXT("ClipSoftnessRect"), SPF_Optional);
		ViewportUVOffset.Bind(Initializer.ParameterMap, TEXT("ViewportUVOffset"), SPF_Optional);
		MainTextureArray.Bind(Initializer.ParameterMap, TEXT("MainTextureArray"), SPF_Optional);
		MainTextureArraySampler.Bind(Initializer.ParameterMap, TEXT("MainTextureArraySampler"), SPF_Optional);
		MainTexture.Bind(Initializer.ParameterMap, TEXT("MainTexture"), SPF_Optional);
		MainTextureSampler.Bind(Initializer.ParameterMap, TEXT("MainTextureSampler"), SPF_Optional);
	}
	
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material);
	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);

	void SetGammaValues(FRHICommandList& RHICmdList, const FVector2D& InGammaValues) const
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), GammaValues, InGammaValues);
	}

	void SetRectClip(FRHICommandList& RHICmdList, const float& bInRectClipping, const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect) const
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), bRectClipping, bInRectClipping);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ClipRect, InClipRect);
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ClipSoftnessRect, InClipSoftnessRect);
	}

	void SetViewportUIOffset(FRHICommandList& RHICmdList, const FVector2D& InViewportUVOffset) const
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), ViewportUVOffset, InViewportUVOffset);
	}

	/**
	 * Sets the vertical axis multiplier to use depending on graphics api
	 */
	void SetVerticalAxisMultiplier(FRHICommandList& RHICmdList, float InMultiplier) const 
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SwitchVerticalAxisMultiplier, InMultiplier);
	}
	
	void SetMainTexture(FRHICommandList& RHICmdList, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI) const
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTexture, MainTextureSampler, SamplerStateRHI, TextureRHI);
	}
	
	void SetMainTextureArray(FRHICommandList& RHICmdList, FRHISamplerState* SamplerStateRHI, FRHITexture* TextureRHI) const
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureArray, MainTextureArraySampler, SamplerStateRHI, TextureRHI);
	}

private:
	LAYOUT_FIELD(FShaderParameter, GammaValues);
	/** Parameter used to determine if we need to switch the vertical axis for opengl */
	LAYOUT_FIELD(FShaderParameter, SwitchVerticalAxisMultiplier)
	LAYOUT_FIELD(FShaderParameter, bRectClipping);
	LAYOUT_FIELD(FShaderParameter, ClipRect);
	LAYOUT_FIELD(FShaderParameter, ClipSoftnessRect);
	LAYOUT_FIELD(FShaderParameter, ViewportUVOffset);
	LAYOUT_FIELD(FShaderResourceParameter, MainTexture);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSampler);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureArray);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureArraySampler);
	
};
