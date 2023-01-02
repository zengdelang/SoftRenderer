#pragma once

#include "SceneManagement.h"

class FUIMeshElementCollector : public FMeshElementCollector
{
public:
	FUIMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel) :FMeshElementCollector(InFeatureLevel)
	{

	}

public:
	void SetPrimitiveSceneProxy(FPrimitiveSceneProxy* InSceneProxy)
	{
		PrimitiveSceneProxy = InSceneProxy;
	}

	void ClearUIViewMeshArrays()
	{
		ClearViewMeshArrays();
	}

	void AddUIViewMeshArrays(
		FSceneView* InView,
		TArray<FMeshBatchAndRelevance, SceneRenderingAllocator>* ViewMeshes,
		FSimpleElementCollector* ViewSimpleElementCollector,
		TArray<FPrimitiveUniformShaderParameters>* InDynamicPrimitiveShaderData,
		ERHIFeatureLevel::Type InFeatureLevel,
		FGlobalDynamicIndexBuffer* InDynamicIndexBuffer,
		FGlobalDynamicVertexBuffer* InDynamicVertexBuffer,
		FGlobalDynamicReadBuffer* InDynamicReadBuffer)
	{
		AddViewMeshArrays(InView, ViewMeshes, ViewSimpleElementCollector, InDynamicPrimitiveShaderData, InFeatureLevel,
			InDynamicIndexBuffer, InDynamicVertexBuffer, InDynamicReadBuffer);
	}

	void ClearMeshBatches()
	{
		MeshBatchStorage.Empty();
	}

	FMeshBatch* GetMeshBatch(int32 Index)
	{
		if (MeshBatchStorage.IsValidIndex(Index))
		{
			return &MeshBatchStorage[Index];
		}
		return nullptr;
	}

	int32 GetMeshBatchNum() const
	{
		return MeshBatchStorage.Num();
	}
 
};
