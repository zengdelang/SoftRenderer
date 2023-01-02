#include "Core/Widgets/Mesh/UIStaticMeshProxyComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "PhysicalMaterials/PhysicalMaterialMask.h"
#include "Materials/MaterialInstanceDynamic.h"

/////////////////////////////////////////////////////
// UUIStaticMeshProxyComponent

#ifndef ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
#define ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY 0
#endif

class FUIStaticMeshSceneProxy final : public FStaticMeshSceneProxy, public FUISceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FUIStaticMeshSceneProxy(UUIStaticMeshProxyComponent* Component)
		: FStaticMeshSceneProxy(Component, false)
		, RenderMode(Component->RenderMode)
		, GraphicType(Component->GraphicType)
		, ScreenOverlaySortPriority(Component->TranslucencySortPriority)
	{
		bUseVirtualWorldTransform = Component->bUseVirtualWorldTransform;
		VirtualWorldTransform = Component->VirtualWorldTransform;
		VirtualTransformID = Component->VirtualTransformID;
		
		ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		if (ProxyId == 0)
		{
			ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		}
		
		if (RenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			ViewExtension = Component->ViewExtension;
			if (ViewExtension.IsValid())
			{
				const auto ViewExtensionPtr = ViewExtension.Pin();
				if (ViewExtensionPtr.IsValid())
				{
					ViewExtensionPtr->AddUISceneProxy(ProxyId, GetPrimitiveComponentId().PrimIDValue, this);
				}
			}
		}

		GraphicData = nullptr;
		if (Component->GraphicData.IsValid())
		{
			const auto GraphicDataPtr = Component->GraphicData.Pin();
			if (GraphicDataPtr.IsValid())
			{
				GraphicData = GraphicDataPtr->CopyGraphicData();
			}
		}
	}

	virtual ~FUIStaticMeshSceneProxy() override
	{
		if (ViewExtension.IsValid())
		{
			const auto ViewExtensionPtr = ViewExtension.Pin();
			if (ViewExtensionPtr.IsValid())
			{
				ViewExtensionPtr->RemoveUISceneProxy_RenderThread(ProxyId, GetPrimitiveComponentId().PrimIDValue);
			}
		}
		ViewExtension.Reset();

		if (GraphicData)
		{
			delete GraphicData;
		}
	}

	void UpdateTranslucentSortPriority_RenderThread(int32 NewTranslucentSortPriority)
	{
		if (ScreenOverlaySortPriority != NewTranslucentSortPriority)
		{
			ScreenOverlaySortPriority = NewTranslucentSortPriority;

			// Need to modify the engine source code, add the following code to the PrimitiveSceneProxy.h file
			// 
			// #if ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
			//           inline void SetTranslucencySortPriority(int16 NewTranslucencySortPriority) { TranslucencySortPriority = NewTranslucencySortPriority; }
			// #endif
			// 
#if ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
			SetTranslucencySortPriority(ScreenOverlaySortPriority);
#endif

			if (ViewExtension.IsValid())
			{
				const auto ViewExtensionPtr = ViewExtension.Pin();
				if (ViewExtensionPtr.IsValid())
				{
					ViewExtensionPtr->SortRenderPriority_RenderThread();
				}
			}
		}
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		FStaticMeshSceneProxy::GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result = FStaticMeshSceneProxy::GetViewRelevance(View);
		Result.bDrawRelevance = Result.bDrawRelevance && RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace;
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return false;
	}
	
	//Begin FUISceneProxy interfaces
	
	virtual int32 GetRenderPriority() const override
	{
		return ScreenOverlaySortPriority;
	}

	virtual FPrimitiveSceneProxy* GetPrimitiveSceneProxy() override
	{
		return this;
	}

	virtual EUIGraphicType GetGraphicType() override
	{
		return GraphicType;
	}

	virtual const TUniformBuffer<FPrimitiveUniformShaderParameters>* GetPrimitiveUniformBufferResource(FMeshElementCollector& Collector) override
	{
		bool bHasPrecomputedVolumetricLightmap = false;
		FMatrix PreviousLocalToWorld;
		int32 SingleCaptureIndex;
		bool bOutputVelocity = false;
		GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
		
		FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
		DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), false, bHasPrecomputedVolumetricLightmap, false, false);
		return &DynamicPrimitiveUniformBuffer.UniformBuffer;
	}

	virtual void UpdateVirtualWorldTransform_RenderThread(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove) override
	{
		VirtualWorldTransform = InVirtualWorldTransform;
		VirtualTransformID = InVirtualTransformID;
			
		if (UICanvasPrimitiveBuffer)
		{
			if (!bUseVirtualWorldTransform)
			{
				if (!bRemove)
				{
					UICanvasPrimitiveBuffer->AddPrimitiveUniformBufferCount(VirtualTransformID, VirtualWorldTransform);			
				}
			}
			else
			{
				if (bRemove)
				{
					UICanvasPrimitiveBuffer->RemovePrimitiveUniformBuffer(VirtualTransformID);			
				}
				else
				{
					UICanvasPrimitiveBuffer->UpdatePrimitiveUniformBuffer(VirtualTransformID, VirtualWorldTransform);
				}
			}
		}

		bUseVirtualWorldTransform = !bRemove;
	}

	virtual bool IntersectViewFrustum(const FConvexVolume& ViewFrustum, const bool bUseWorldTransform, const FMatrix& InVirtualWorldTransform) override
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_IntersectViewFrustum);

		if (!bUseWorldTransform)
		{
			const FBoxSphereBounds& WorldBounds = GetBounds();
			return ViewFrustum.IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent);
		}

		const FBoxSphereBounds& WorldBounds = GetLocalBounds().TransformBy(InVirtualWorldTransform);
		return ViewFrustum.IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent);
	}
	
	//End FUISceneProxy interfaces

private:
	ECanvasRenderMode RenderMode;
	
	EUIGraphicType GraphicType;
	FUIGraphicData* GraphicData;

	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;

	int32 ScreenOverlaySortPriority;
	uint32 ProxyId;
};

//////////////////////////////////////////////////////////////////////////

UUIStaticMeshProxyComponent::UUIStaticMeshProxyComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReceiveMobileCSMShadows = false;

	bRenderInDepthPass = false;
	bReceivesDecals = false;

	UPrimitiveComponent::SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UPrimitiveComponent::SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	UPrimitiveComponent::SetCollisionResponseToChannel(COLLISION_GIZMO, ECollisionResponse::ECR_Ignore);
	UPrimitiveComponent::SetEnableGravity(false);
	
	bApplyImpulseOnDamage = false;
	bReplicatePhysicsToAutonomousProxy = false;

	bCanEverAffectNavigation = false;
	bVisibleInRayTracing = false;
	bVisibleInReflectionCaptures = false;
	bVisibleInRealTimeSkyCaptures = false;
	bRenderInMainPass = false;

#if WITH_EDITORONLY_DATA
	bEnableAutoLODGeneration = false;
#endif 
	
	SetGenerateOverlapEvents(false);

	CastShadow = false;

	CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	bUseAsOccluder = false;

	bUseViewOwnerDepthPriorityGroup = true;

	RenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
	GraphicType = EUIGraphicType::StaticMesh;
	
	GraphicEffects = FVector2D::ZeroVector;
	RenderOpacity = 0;
	
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
}

FPrimitiveSceneProxy* UUIStaticMeshProxyComponent::CreateSceneProxy()
{
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
	
	if (GetStaticMesh() == nullptr || GetStaticMesh()->RenderData == nullptr)
	{
		return nullptr;
	}

	const FStaticMeshLODResourcesArray& LODResources = GetStaticMesh()->RenderData->LODResources;
	if (LODResources.Num() == 0 || LODResources[FMath::Clamp<int32>(GetStaticMesh()->MinLOD.Default, 0, LODResources.Num() - 1)].VertexBuffers.StaticMeshVertexBuffer.GetNumVertices() == 0)
	{
		return nullptr;
	}
	LLM_SCOPE(ELLMTag::StaticMesh);

	FPrimitiveSceneProxy* Proxy = new FUIStaticMeshSceneProxy(this);
#if STATICMESH_ENABLE_DEBUG_RENDERING
	SendRenderDebugPhysics(Proxy);
#endif

	return Proxy;
}

void UUIStaticMeshProxyComponent::SendRenderDynamicData_Concurrent()
{
	if (bUpdateTranslucentSortPriority)
	{
		if (SceneProxy)
		{
			const int32 NewTranslucentSortPriority = TranslucencySortPriority;
			
			// Enqueue command to send to render thread
			FUIStaticMeshSceneProxy* UIStaticMeshSceneProxy = static_cast<FUIStaticMeshSceneProxy*>(SceneProxy);

			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([UIStaticMeshSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
					{
						UIStaticMeshSceneProxy->UpdateTranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
					});
		}
	}

	if (bUpdateVirtualWorldTransform)
	{
		bUpdateVirtualWorldTransform = false;

		if (SceneProxy)
		{
			const bool bRemoveVirtualWorldTransform = !bUseVirtualWorldTransform;
			auto InVirtualTransformID = VirtualTransformID;
			auto InVirtualWorldTransform = VirtualWorldTransform;
			
			// Enqueue command to send to render thread
			FUIStaticMeshSceneProxy* UIStaticMeshSceneProxy = (FUIStaticMeshSceneProxy*)SceneProxy;
			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateVirtualWorldTransform)
			([InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform, UIStaticMeshSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				UIStaticMeshSceneProxy->UpdateVirtualWorldTransform_RenderThread(InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform);
			});
		}
	}

	bUpdateTranslucentSortPriority = false;
	
	Super::SendRenderDynamicData_Concurrent();
}

void UUIStaticMeshProxyComponent::SetGraphicEffects(FVector2D InGraphicEffects, bool bUpdateMaterial)
{
	GraphicEffects = InGraphicEffects;

	if (bUpdateMaterial)
	{
		for (int32 Index = 0, Count = GetNumMaterials(); Index < Count; ++Index)
		{
			const auto Material = GetMaterial(Index);
			UMaterialInstanceDynamic* UIMaterial = Cast<UMaterialInstanceDynamic>(Material);

			if (!IsValid(UIMaterial))
			{
				if (!Material)
				{
					continue;
				}

				UIMaterial = UMaterialInstanceDynamic::Create(Material, this);
				UIMaterial->SetFlags(RF_Transient);

				SetMaterial(Index, UIMaterial);
			}

			if (IsValid(UIMaterial))
			{
				UIMaterial->SetVectorParameterValue(FName("GraphicEffects"), FLinearColor(GraphicEffects.X, GraphicEffects.Y, 0, 0));
			}
		}
	}
}

void UUIStaticMeshProxyComponent::SetRenderOpacity(float InRenderOpacity, bool bUpdateMaterial)
{
	RenderOpacity = InRenderOpacity;

	if (bUpdateMaterial)
	{
		for (int32 Index = 0, Count = GetNumMaterials(); Index < Count; ++Index)
		{
			const auto Material = GetMaterial(Index);
			UMaterialInstanceDynamic* UIMaterial = Cast<UMaterialInstanceDynamic>(Material);

			if (!IsValid(UIMaterial))
			{
				if (!Material)
				{
					continue;
				}

				UIMaterial = UMaterialInstanceDynamic::Create(Material, this);
				UIMaterial->SetFlags(RF_Transient);

				SetMaterial(Index, UIMaterial);
			}

			if (IsValid(UIMaterial))
			{
				UIMaterial->SetScalarParameterValue(FName("RenderOpacity"), RenderOpacity);
			}
		}
	}
}

void UUIStaticMeshProxyComponent::SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> InViewExtension)
{
	bool bIsDirty = false;

	if (RenderMode != InRenderMode)
	{
		RenderMode = InRenderMode;
		bIsDirty = true;
	}

	if (ViewExtension != InViewExtension)
	{
		ViewExtension = InViewExtension;
		bIsDirty = true;
	}

	if (bIsDirty)
	{
		MarkRenderStateDirty();
	}
}

void UUIStaticMeshProxyComponent::ClearUIMesh()
{
	IUIRenderProxyInterface::ClearUIMesh();

	SetVisibility(false);
}

void UUIStaticMeshProxyComponent::SetUIMaterial(const USceneComponent* CanvasSceneComp, UMaterialInterface* InMaterial, UTexture* InTexture,
                                                const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy)
{
	if (!IsValid(CanvasSceneComp))
		return;

	SetVisibility(true);
	
	for (int32 Index = 0, Count = GetNumMaterials(); Index < Count; ++Index)
	{
		const auto Material = GetMaterial(Index);
		UMaterialInstanceDynamic* UIMaterial = Cast<UMaterialInstanceDynamic>(Material);

		if (!IsValid(UIMaterial))
		{
			if (!Material)
			{
				continue;
			}

			UIMaterial = UMaterialInstanceDynamic::Create(Material, this);
			UIMaterial->SetFlags(RF_Transient);

			SetMaterial(Index, UIMaterial);
		}
		
		if (IsValid(UIMaterial))
		{
			UIMaterial->SetScalarParameterValue(FName("bRectClipping"), bInRectClipping ? 1 : 0);

			if (bInRectClipping)
			{
				UIMaterial->SetVectorParameterValue(FName("ClipRect"), InClipRect);

				// TODO InClipSoftnessRect
				
				const FMatrix WorldToCanvas = CanvasSceneComp->GetComponentTransform().Inverse().ToMatrixWithScale();

				const FLinearColor MatColumn1 = FLinearColor(WorldToCanvas.M[0][0], WorldToCanvas.M[1][0], WorldToCanvas.M[2][0], WorldToCanvas.M[3][0]);
				UIMaterial->SetVectorParameterValue(FName("WorldToCanvas00"), MatColumn1);

				const FLinearColor MatColumn2 = FLinearColor(WorldToCanvas.M[0][1], WorldToCanvas.M[1][1], WorldToCanvas.M[2][1], WorldToCanvas.M[3][1]);
				UIMaterial->SetVectorParameterValue(FName("WorldToCanvas01"), MatColumn2);

				const FLinearColor MatColumn3 = FLinearColor(WorldToCanvas.M[0][2], WorldToCanvas.M[1][2], WorldToCanvas.M[2][2], WorldToCanvas.M[3][2]);
				UIMaterial->SetVectorParameterValue(FName("WorldToCanvas02"), MatColumn3);

				const FLinearColor MatColumn4 = FLinearColor(WorldToCanvas.M[0][3], WorldToCanvas.M[1][3], WorldToCanvas.M[2][3], WorldToCanvas.M[3][3]);
				UIMaterial->SetVectorParameterValue(FName("WorldToCanvas03"), MatColumn4);
			}

			UIMaterial->SetVectorParameterValue(FName("GraphicEffects"), FLinearColor(GraphicEffects.X, GraphicEffects.Y, 0, 0));
			UIMaterial->SetScalarParameterValue(FName("RenderOpacity"), RenderOpacity);
		}
	}
}

void UUIStaticMeshProxyComponent::UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority)
{
	if (SceneProxy)
	{
		if (RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay
			|| RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree)
		{
			bUpdateTranslucentSortPriority = true;
			TranslucencySortPriority = NewTranslucentSortPriority;
			MarkRenderDynamicDataDirty();
		}
		else
		{
			// Need to modify the engine source code, add the following code to the PrimitiveSceneProxy.h file
			// 
			// #if ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
			//           inline void SetTranslucencySortPriority(int16 NewTranslucencySortPriority) { TranslucencySortPriority = NewTranslucencySortPriority; }
			// #endif
			//
#if ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
			bUpdateTranslucentSortPriority = true;
			TranslucencySortPriority = NewTranslucentSortPriority;
			MarkRenderDynamicDataDirty();
#else
			SetTranslucentSortPriority(NewTranslucentSortPriority);
#endif
		}
	}
	else
	{
		SetTranslucentSortPriority(NewTranslucentSortPriority);
	}
}

void UUIStaticMeshProxyComponent::SetupVirtualWorldTransform(bool bInUseVirtualWorldTransform, FTransform InVirtualWorldTransform, USceneComponent* CanvasAttachComponent)
{
	if (!bInUseVirtualWorldTransform)
		return;

	const USceneComponent* ProxyParentComponent = GetAttachParent();
	if (!IsValid(ProxyParentComponent) || !IsValid(CanvasAttachComponent))
	{
		return;
	}
	
	bUseVirtualWorldTransform = true;
	VirtualTransformID = GetUniqueID();

	const FTransform ParentToCanvas = (ProxyParentComponent->GetComponentTransform() * CanvasAttachComponent->GetComponentTransform().Inverse());
	VirtualWorldTransform = (FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector) * ParentToCanvas * InVirtualWorldTransform).ToMatrixWithScale();
}

void UUIStaticMeshProxyComponent::UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent)
{
	bUseVirtualWorldTransform = !bRemove;
	VirtualTransformID = InVirtualTransformID;
	SetupVirtualWorldTransform(true, FTransform(InVirtualWorldTransform), CanvasAttachComponent);

	bUpdateVirtualWorldTransform = true;
	MarkRenderDynamicDataDirty();
}

/////////////////////////////////////////////////////
