#include "Core/Renderer/UIRendering.h"
#include "ShaderParameterUtils.h"

/************************************************************************/
/* FUIMeshMaterialShaderVS                                                       */
/************************************************************************/

IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIMeshMaterialShaderVS, TEXT("/Plugin/UGUI/Private/UIMeshMaterialShaders.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIMeshMaterialShaderPS, TEXT("/Plugin/UGUI/Private/UIMeshMaterialShaders.usf"), TEXT("MainPS"), SF_Pixel);

#ifndef USE_CUSTOM_MATERIAL_SHADER_TYPE
#define USE_CUSTOM_MATERIAL_SHADER_TYPE 0
#endif

bool FUIMeshMaterialShaderVS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
#if USE_CUSTOM_MATERIAL_SHADER_TYPE
	if ((Parameters.MaterialParameters.MaterialRenderShaderType & static_cast<int32>(MRST_UI)) == 1)
	{
		return true;
	}
	return false;
#else
	return true;
#endif
}

void FUIMeshMaterialShaderVS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("USE_INSTANCING"), false);
	OutEnvironment.SetDefine(TEXT("USE_INSTANCING_BONEMAP"), false);
	
	FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

/************************************************************************/
/* FUIMeshMaterialShaderPS                                                       */
/************************************************************************/

bool FUIMeshMaterialShaderPS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
#if USE_CUSTOM_MATERIAL_SHADER_TYPE
	if ((Parameters.MaterialParameters.MaterialRenderShaderType & static_cast<int32>(MRST_UI)) == 1)
	{
		return true;
	}
	return false;
#else
	return true;
#endif
}

void FUIMeshMaterialShaderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("USE_INSTANCING"), false);
	OutEnvironment.SetDefine(TEXT("USE_INSTANCING_BONEMAP"), false);

	static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.HDR.Display.OutputDevice"));
	OutEnvironment.SetDefine(TEXT("USE_709"), CVar ? (CVar->GetValueOnGameThread() == 1) : 1);
	
	FMeshMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

/************************************************************************/
/* FUIMaterialShaderVS                                                  */
/************************************************************************/

IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIMaterialShaderVS, TEXT("/Plugin/UGUI/Private/UIMaterialShaders.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FUIMaterialShaderPS, TEXT("/Plugin/UGUI/Private/UIMaterialShaders.usf"), TEXT("MainPS"), SF_Pixel);

bool FUIMaterialShaderVS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
#if USE_CUSTOM_MATERIAL_SHADER_TYPE
	if ((Parameters.MaterialParameters.MaterialRenderShaderType & static_cast<int32>(MRST_UI)) == 1)
	{
		return true;
	}
	return false;
#else
	return true;
#endif
}

void FUIMaterialShaderVS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

void FUIMaterialShaderVS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIVertexShader* ShaderRHI = RHICmdList.GetBoundVertexShader();
	SetViewParameters(RHICmdList, ShaderRHI, View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters<FRHIVertexShader>(RHICmdList, ShaderRHI, MaterialRenderProxy, *Material, View);
	

	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundVertexShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
}

/************************************************************************/
/* FUIMaterialShaderPS                                                       */
/************************************************************************/



bool FUIMaterialShaderPS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
#if USE_CUSTOM_MATERIAL_SHADER_TYPE
	if ((Parameters.MaterialParameters.MaterialRenderShaderType & static_cast<int32>(MRST_UI)) == 1)
	{
		return true;
	}
	return false;
#else
	return true;
#endif
}

void FUIMaterialShaderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("SUPPORTS_UNREAL_GUI"), true);

	static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.HDR.Display.OutputDevice"));
	OutEnvironment.SetDefine(TEXT("USE_709"), CVar ? (CVar->GetValueOnGameThread() == 1) : 1);
	
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}

void FUIMaterialShaderPS::SetBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material)
{
	const EBlendMode BlendMode = Material->GetBlendMode();
	switch (BlendMode)
	{
	default:
	case BLEND_Opaque:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Masked:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Translucent:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
		break;
	case BLEND_Additive:
		// Add to the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		break;
	case BLEND_Modulate:
		// Modulate with the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_DestColor, BF_Zero>::GetRHI();
		break;
	case BLEND_AlphaComposite:
		// Blend with existing scene color. New color is already pre-multiplied by alpha.
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		break;
	case BLEND_AlphaHoldout:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		break;
	};
}

void FUIMaterialShaderPS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIPixelShader* ShaderRHI = RHICmdList.GetBoundPixelShader();
	SetViewParameters(RHICmdList, ShaderRHI, View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters<FRHIPixelShader>(RHICmdList, RHICmdList.GetBoundPixelShader(), MaterialRenderProxy, *Material, View);

	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
}
