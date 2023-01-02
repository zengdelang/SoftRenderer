#pragma once

#include "CoreMinimal.h"
#include "UIRendering.h"
#include "MeshPassProcessor.h"
#include "MeshPassProcessor.inl"
#include "UGUI.h"

class FUIMeshPassProcessor : public FMeshPassProcessor
{
public:
	FUIMeshPassProcessor(const FScene* InScene, const FSceneView* InView, FMeshPassDrawListContext* InDrawListContext, float InDisplayGamma, float InSwitchVerticalAxisMultiplier)
		: FMeshPassProcessor(InScene, InView->GetFeatureLevel(), InView, InDrawListContext)
		, DisplayGamma(InDisplayGamma)
		, SwitchVerticalAxisMultiplier(InSwitchVerticalAxisMultiplier)
	{
		DrawRenderState.SetViewUniformBuffer(InView->ViewUniformBuffer);
	}

	virtual void AddMeshBatch(const FMeshBatch& MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* PrimitiveSceneProxy, int32 StaticMeshId = -1) override final
	{
		const FMaterialRenderProxy* FallbackMaterialRenderProxyPtr = nullptr;
		const FMaterial& Material = MeshBatch.MaterialRenderProxy->GetMaterialWithFallback(FeatureLevel, FallbackMaterialRenderProxyPtr);
		const FMaterialRenderProxy& MaterialRenderProxy = FallbackMaterialRenderProxyPtr ? *FallbackMaterialRenderProxyPtr : *MeshBatch.MaterialRenderProxy;

		DrawRenderState.SetBlendState(GetBlendState(Material));
		DrawRenderState.SetDepthStencilState(GetDepthStencilState(Material));

		if (MeshBatch.bUseForMaterial)
		{
			Process(MeshBatch, BatchElementMask, StaticMeshId, PrimitiveSceneProxy, MaterialRenderProxy, Material);
		}
	}

private:
	void Process(
		const FMeshBatch& MeshBatch,
		uint64 BatchElementMask,
		int32 StaticMeshId,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& MaterialResource)
	{
		const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

		TMeshProcessorShaders<
			FUIMeshMaterialShaderVS,
			FMeshMaterialShader,
			FMeshMaterialShader,
			FUIMeshMaterialShaderPS> Shaders;

		{
			//Shaders.VertexShader = MaterialResource.GetShader<FUIMeshMaterialShaderVS>(VertexFactory->GetType(), 0, false);
			FMaterialShaderMap* RenderingThreadShaderMap = MaterialResource.GetRenderingThreadShaderMap();
			if (RenderingThreadShaderMap)
			{
				const FMeshMaterialShaderMap* MeshShaderMap = RenderingThreadShaderMap->GetMeshShaderMap(VertexFactory->GetType());
				FShader* Shader = MeshShaderMap ? MeshShaderMap->GetShader(&FUIMeshMaterialShaderVS::StaticType, 0) : nullptr;
				if (!Shader)
				{
					UE_LOG(LogUGUI, Error,
						TEXT("Couldn't find Shader (%s, %d) for Material Resource %s!\n"), (&FUIMeshMaterialShaderVS::StaticType)->GetName(), 0, *MaterialResource.GetFriendlyName()
						);
					return;
				}
				Shaders.VertexShader = TShaderRef<FUIMeshMaterialShaderVS>::Cast(TShaderRef<FShader>(Shader, *RenderingThreadShaderMap));
			}
		}

		{
			Shaders.PixelShader = MaterialResource.GetShader<FUIMeshMaterialShaderPS>(VertexFactory->GetType(), 0, false);
			FMaterialShaderMap* RenderingThreadShaderMap = MaterialResource.GetRenderingThreadShaderMap();
			if (RenderingThreadShaderMap)
			{
				const FMeshMaterialShaderMap* MeshShaderMap = RenderingThreadShaderMap->GetMeshShaderMap(VertexFactory->GetType());
				FShader* Shader = MeshShaderMap ? MeshShaderMap->GetShader(&FUIMeshMaterialShaderPS::StaticType, 0) : nullptr;
				if (!Shader)
				{
					UE_LOG(LogUGUI, Error,
							TEXT("Couldn't find Shader (%s, %d) for Material Resource %s!\n"), (&FUIMeshMaterialShaderPS::StaticType)->GetName(), 0, *MaterialResource.GetFriendlyName()
						);
					return;
				}
				Shaders.PixelShader = TShaderRef<FUIMeshMaterialShaderPS>::Cast(TShaderRef<FShader>(Shader, *RenderingThreadShaderMap));
			}
		}
		
		const FMeshDrawingPolicyOverrideSettings OverrideSettings = ComputeMeshOverrideSettings(MeshBatch);
		const ERasterizerFillMode MeshFillMode = ComputeMeshFillMode(MeshBatch, MaterialResource, OverrideSettings);
		const ERasterizerCullMode MeshCullMode = MaterialResource.IsTwoSided() ? CM_None : CM_CCW;

		FUIMeshMaterialShaderElementData ShaderElementData;
		ShaderElementData.DisplayGamma = DisplayGamma;
		ShaderElementData.SwitchVerticalAxisMultiplier = SwitchVerticalAxisMultiplier;
		
		BuildMeshDrawCommands(
			MeshBatch,
			BatchElementMask,
			PrimitiveSceneProxy,
			MaterialRenderProxy,
			MaterialResource,
			DrawRenderState,
			Shaders,
			MeshFillMode,
			MeshCullMode,
			FMeshDrawCommandSortKey(),
			EMeshPassFeatures::Default,
			ShaderElementData);

	}

private:
	static FRHIBlendState* GetBlendState(const FMaterial& Material)
	{
		const EBlendMode BlendMode = Material.GetBlendMode();
		switch (BlendMode)
		{
		default:
		case BLEND_Opaque:
			return TStaticBlendState<>::GetRHI();
		case BLEND_Masked:
			return TStaticBlendState<>::GetRHI();
		case BLEND_Translucent:
			return TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
		case BLEND_Additive:
			// Add to the existing scene color
			return TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		case BLEND_Modulate:
			// Modulate with the existing scene color
			return TStaticBlendState<CW_RGB, BO_Add, BF_DestColor, BF_Zero>::GetRHI();
		case BLEND_AlphaComposite:
			// Blend with existing scene color. New color is already pre-multiplied by alpha.
			return TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		case BLEND_AlphaHoldout:
			// Blend by holding out the matte shape of the source alpha
			return TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		};
	}

	static FRHIDepthStencilState* GetDepthStencilState(const FMaterial& Material)
	{
		const EBlendMode BlendMode = Material.GetBlendMode();
		switch (BlendMode)
		{
		case BLEND_Opaque:
		case BLEND_Masked:
			return TStaticDepthStencilState<true, CF_DepthNearOrEqual>::GetRHI();
		default:
			return TStaticDepthStencilState<false, CF_Always>::GetRHI();
		};	
	}

private:
	FMeshPassProcessorRenderState DrawRenderState;

	float DisplayGamma;
	float SwitchVerticalAxisMultiplier;
	
};

class UGUI_API FUIShadingRenderer
{
public:
	void Render(FRHICommandListImmediate& RHICmdList, FSceneView& RenderView, const ERHIFeatureLevel::Type FeatureLevel,
		const FMeshBatch& MeshBatch, float DisplayGamma, float SwitchVerticalAxisMultiplier, bool bForceStereoInstancingOff = false);
	
protected:
	FDynamicMeshDrawCommandStorage DynamicMeshDrawCommandStorage;
	FMeshCommandOneFrameArray VisibleMeshDrawCommands;
	
	bool bNeedsShaderInitialization = false;
	
};
