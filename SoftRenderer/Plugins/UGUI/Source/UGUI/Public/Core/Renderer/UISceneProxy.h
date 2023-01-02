#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "Core/UICommonDefinitions.h"
#include "UICanvasPrimitiveBuffer.h"
#include "UGUI.h"

DECLARE_CYCLE_STAT(TEXT("UIRender --- IntersectViewFrustum"), STAT_UnrealGUI_IntersectViewFrustum, STATGROUP_UnrealGUI);

class FUISceneProxy
{
public:
	virtual ~FUISceneProxy() {}
	
	virtual bool GetMeshElement(class FMeshElementCollector* Collector, FMeshBatch& MeshBatch, TUniformBuffer<FPrimitiveUniformShaderParameters>* CanvasPrimitiveUniformBuffer, const FSceneViewFamily& ViewFamily) { return false; }

	virtual void GetMainTextureRHIResource(FTextureRHIRef& MainTextureRHIRef, FRHISamplerState*& MainTextureSamplerState) {}
	
	virtual int32 GetRenderPriority() const = 0;

	virtual FRHIVertexBuffer* GetVertexBufferRHI() { return nullptr; }

	virtual uint32 GetNumVertices() { return 0; }

	virtual FPrimitiveSceneProxy* GetPrimitiveSceneProxy() = 0;

	virtual EUIGraphicType GetGraphicType() = 0;

	virtual FUIGraphicData* GetGraphicData() { return nullptr; }

	virtual bool UseTextureArray() { return false; }
	
	virtual bool IsRectClipping() { return false; }
	
	virtual FLinearColor GetClipRect () { return FLinearColor::Black; }

	virtual FLinearColor GetClipSoftnessRect () { return FLinearColor::Black; }

	virtual FMatrix GetLocalToWorldMatrix() { return FMatrix(); }

	virtual bool GetScreenRect(const FMatrix& InViewProjectionMatrix, FVector2D& TopLeftUV, FVector2D& BottomRightUV) { return false; }

	virtual const TUniformBuffer<FPrimitiveUniformShaderParameters>* GetPrimitiveUniformBufferResource(FMeshElementCollector& Collector) { return nullptr; }
	
	virtual void UpdateVirtualWorldTransform_RenderThread(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove) {}

	virtual bool IntersectViewFrustum(const FConvexVolume& ViewFrustum, const bool bUseWorldTransform, const FMatrix& InVirtualWorldTransform) = 0;
	
public:
	bool bUseVirtualWorldTransform = false;
	FMatrix VirtualWorldTransform = FMatrix::Identity;
	uint32 VirtualTransformID = 0;

	FUICanvasPrimitiveBuffer* UICanvasPrimitiveBuffer = nullptr;

};
