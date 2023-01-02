#include "Core/Renderer/UIShadingRenderer.h"

void FUIShadingRenderer::Render(FRHICommandListImmediate& RHICmdList, FSceneView& RenderView, const ERHIFeatureLevel::Type FeatureLevel, 
	const FMeshBatch& MeshBatch, float DisplayGamma, float SwitchVerticalAxisMultiplier, bool bForceStereoInstancingOff)
{
	DynamicMeshDrawCommandStorage.MeshDrawCommands.Empty();
	VisibleMeshDrawCommands.Empty();
	FGraphicsMinimalPipelineStateSet GraphicsMinimalPipelineStateSet;
	bNeedsShaderInitialization = false;

	FDynamicPassMeshDrawListContext DynamicMeshPassContext(DynamicMeshDrawCommandStorage, VisibleMeshDrawCommands, GraphicsMinimalPipelineStateSet, bNeedsShaderInitialization);
	{
		FUIMeshPassProcessor PassMeshProcessor(
			nullptr,
			&RenderView,
			&DynamicMeshPassContext,
			DisplayGamma, SwitchVerticalAxisMultiplier);

		MeshBatch.MaterialRenderProxy->UpdateUniformExpressionCacheIfNeeded(FeatureLevel);

		PassMeshProcessor.AddMeshBatch(MeshBatch, ~0ull, nullptr);
	}

	// We assume all dynamic passes are in stereo if it is enabled in the view, so we apply ISR to them
	const uint32 InstanceFactor = (!bForceStereoInstancingOff && RenderView.IsInstancedStereoPass()) ? 2 : 1;
	DrawDynamicMeshPassPrivate(RenderView, RHICmdList, VisibleMeshDrawCommands, DynamicMeshDrawCommandStorage,
		GraphicsMinimalPipelineStateSet, bNeedsShaderInitialization, InstanceFactor);
	
}
