#pragma once

#include "SceneView.h"

class FUIViewInfo : public FSceneView
{
public:
	TUniquePtr<FViewUniformShaderParameters> CachedViewUniformShaderParameters;

	/** Only one of the resources(TextureBuffer or Texture2D) will be used depending on the Mobile.UseGPUSceneTexture cvar */
	FShaderResourceViewRHIRef PrimitiveSceneDataOverrideSRV;
	FTexture2DRHIRef PrimitiveSceneDataTextureOverrideRHI;

	/** Gathered in initviews from all the primitives with dynamic view relevance, used in each mesh pass. */
	TArray<FMeshBatchAndRelevance, SceneRenderingAllocator> DynamicMeshElements;

	FSimpleElementCollector SimpleElementCollector;

	/** Tracks dynamic primitive data for upload to GPU Scene, when enabled. */
	TArray<FPrimitiveUniformShaderParameters> DynamicPrimitiveShaderData;
	
public:
	explicit FUIViewInfo(const FSceneView* InView)
		: FSceneView(*InView)
	{
		CachedViewUniformShaderParameters = MakeUnique<FViewUniformShaderParameters>();
		PrimitiveSceneDataOverrideSRV = nullptr;
		PrimitiveSceneDataTextureOverrideRHI = nullptr;
	}

	/** Initializes the RHI resources used by this view. */
	void InitRHIResources()
	{
		check(IsInRenderingThread());

		if (UseGPUScene(GMaxRHIShaderPlatform, GetFeatureLevel()))
		{
			if (PrimitiveSceneDataOverrideSRV)
			{
				(*CachedViewUniformShaderParameters).PrimitiveSceneData = PrimitiveSceneDataOverrideSRV;
			}

			if (PrimitiveSceneDataTextureOverrideRHI)
			{
				(*CachedViewUniformShaderParameters).PrimitiveSceneDataTexture = PrimitiveSceneDataTextureOverrideRHI;
			}

			ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(*CachedViewUniformShaderParameters, UniformBuffer_SingleFrame);
		}
	}
	
};
