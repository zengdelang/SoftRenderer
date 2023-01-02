#include "Designer/UISelectedRectDrawerProxyComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "UGUISubsystem.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "PhysicalMaterials/PhysicalMaterialMask.h"
#include "Core/Render/CanvasSubComponent.h"
#include "UObject/ConstructorHelpers.h"

/////////////////////////////////////////////////////
// UUIPrimitiveDrawerProxyComponent

#ifndef ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
#define ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY 0
#endif

class FUISelectedRectDrawerSceneProxy : public FPrimitiveSceneProxy, public FUISceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FUISelectedRectDrawerSceneProxy(UUISelectedRectDrawerProxyComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, RenderMode(Component->RenderMode)
		, GraphicType(Component->GraphicType)
		, ScreenOverlaySortPriority(Component->TranslucencySortPriority)
		, bDraw (true)
	{
		bUseVirtualWorldTransform = Component->bUseVirtualWorldTransform;
		VirtualWorldTransform = Component->VirtualWorldTransform;
		VirtualTransformID = Component->VirtualTransformID;
		
		GraphicData = nullptr;

		TopLeft = Component->TopLeft;
		BottomRight = Component->BottomRight;
		DotAlpha = Component->DotAlpha;
		bShow = Component->bShow;
		
		ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		if (ProxyId == 0)
		{
			ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		}

		if(Component->DotSprite)
		{
			Texture = Component->DotSprite;
		}
		else
		{
			Texture = nullptr;
		}

		if (Component->CircleSprite)
		{
			CircleTexture = Component->CircleSprite;
		}
		else
		{
			CircleTexture = nullptr;
		}
		
#if WITH_EDITOR
		const FWorldViewportInfo* EditorViewportInfoPtr = UUGUISubsystem::GetWorldViewportInfo(Component);
		if (EditorViewportInfoPtr)
		{
			bDraw = EditorViewportInfoPtr->GetRenderMode() != ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay;
		}
#endif

		if (!bDraw)
		{
			return;
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
	}

	virtual ~FUISelectedRectDrawerSceneProxy() override
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

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return false;
	}
	
	virtual uint32 GetMemoryFootprint(void) const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (!bDraw)
			return;

		if (!bShow)
			return;

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];

				PDI->DrawLine(FVector(TopLeft, 0), FVector(TopLeft.X, BottomRight.Y, 0), FLinearColor::White, SDPG_World, 0, 0, true);
				PDI->DrawLine(FVector(TopLeft.X, BottomRight.Y, 0), FVector(BottomRight, 0), FLinearColor::White, SDPG_World, 0, 0, true);
				PDI->DrawLine(FVector(BottomRight, 0), FVector(BottomRight.X, TopLeft.Y, 0), FLinearColor::White, SDPG_World, 0, 0, true);
				PDI->DrawLine(FVector(BottomRight.X, TopLeft.Y, 0), FVector(TopLeft, 0), FLinearColor::White, SDPG_World, 0, 0, true);

				FTexture* CircleTextureResource = (CircleTexture != nullptr) ? CircleTexture->Resource : nullptr;
				if (CircleTextureResource)
				{
					if (DotAlpha < 1)
					{
						PDI->DrawSprite(
							FVector((TopLeft + BottomRight) * 0.5, 0),
							16,
							16,
							CircleTextureResource,
							FLinearColor(1, 1, 1, FMath::Clamp(1.0 - DotAlpha, 0.4, 0.8)),
							SDPG_Foreground,
							0,32,0,32, SE_BLEND_TranslucentAlphaOnly
							);
					}
				}
				
				FTexture* TextureResource = (Texture != nullptr) ? Texture->Resource : nullptr;
				if (TextureResource)
				{
					PDI->DrawSprite(
								FVector(TopLeft, 0),
								8,
								8,
								TextureResource,
								FLinearColor(0.055, 0.45, 0.87, DotAlpha),
								SDPG_Foreground,
								0,16,0,16, SE_BLEND_TranslucentAlphaOnly
								);
					
					PDI->DrawSprite(
								FVector(TopLeft.X, BottomRight.Y, 0),
								8,
								8,
								TextureResource,
								FLinearColor(0.055, 0.45, 0.87, DotAlpha),
								SDPG_Foreground,
								0,16,0,16, SE_BLEND_TranslucentAlphaOnly
								);

					PDI->DrawSprite(
								FVector(BottomRight, 0),
								8,
								8,
								TextureResource,
								FLinearColor(0.055, 0.45, 0.87, DotAlpha),
								SDPG_Foreground,
								0,16,0,16, SE_BLEND_TranslucentAlphaOnly
								);

					PDI->DrawSprite(
								FVector(BottomRight.X, TopLeft.Y, 0),
								8,
								8,
								TextureResource,
								FLinearColor(0.055, 0.45, 0.87, DotAlpha),
								SDPG_Foreground,
								0,16,0,16, SE_BLEND_TranslucentAlphaOnly
								);
				}
			}
		}
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
		bool bHasPrecomputedVolumetricLightmap;
		FMatrix PreviousLocalToWorld;
		int32 SingleCaptureIndex;
		bool bOutputVelocity;
		GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
		
		FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
		DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), bOutputVelocity);
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

		const FBoxSphereBounds& WorldBounds = GetBounds().TransformBy(InVirtualWorldTransform);
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

	const UTexture2D* Texture;
	const UTexture2D* CircleTexture;
	
	uint8 bDraw : 1;

	FVector2D TopLeft;
	FVector2D BottomRight;
	float DotAlpha;
	bool bShow = false;
	
};

//////////////////////////////////////////////////////////////////////////

UUISelectedRectDrawerProxyComponent::UUISelectedRectDrawerProxyComponent(const FObjectInitializer& ObjectInitializer)
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

	SetGenerateOverlapEvents(false);

	CastShadow = false;

	CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	bUseAsOccluder = false;

	bUseViewOwnerDepthPriorityGroup = true;

	RenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
	GraphicType = EUIGraphicType::StaticMesh;

	GraphicEffects = FVector2D::ZeroVector;
	RenderOpacity = 0;

	static ConstructorHelpers::FObjectFinderOptional<UTexture2D> DefaultDotSprite_Finder(TEXT("/UGUI/DefaultResources/Editor/Dot.Dot"));
	DotSprite = DefaultDotSprite_Finder.Get();

	static ConstructorHelpers::FObjectFinderOptional<UTexture2D> DefaultCircleSprite_Finder(TEXT("/UGUI/DefaultResources/Editor/Circle.Circle"));
	CircleSprite = DefaultCircleSprite_Finder.Get();

	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
}

FPrimitiveSceneProxy* UUISelectedRectDrawerProxyComponent::CreateSceneProxy()
{
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
	return new FUISelectedRectDrawerSceneProxy(this);
}

void UUISelectedRectDrawerProxyComponent::SendRenderDynamicData_Concurrent()
{
	if (bUpdateTranslucentSortPriority)
	{
		if (SceneProxy)
		{
			const int32 NewTranslucentSortPriority = TranslucencySortPriority;
			
			// Enqueue command to send to render thread
			FUISelectedRectDrawerSceneProxy* UISelectedRectDrawerSceneProxy = static_cast<FUISelectedRectDrawerSceneProxy*>(SceneProxy);

			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([UISelectedRectDrawerSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
					{
						UISelectedRectDrawerSceneProxy->UpdateTranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
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
			FUISelectedRectDrawerSceneProxy* UISelectedRectDrawerSceneProxy = (FUISelectedRectDrawerSceneProxy*)SceneProxy;
			
			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateVirtualWorldTransform)
			([InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform, UISelectedRectDrawerSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				UISelectedRectDrawerSceneProxy->UpdateVirtualWorldTransform_RenderThread(InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform);
			});
		}
	}

	bUpdateTranslucentSortPriority = false;
	
	Super::SendRenderDynamicData_Concurrent();
}

void UUISelectedRectDrawerProxyComponent::SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> InViewExtension)
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

void UUISelectedRectDrawerProxyComponent::ClearUIMesh()
{
	IUIRenderProxyInterface::ClearUIMesh();
	SetVisibility(false);
}

void UUISelectedRectDrawerProxyComponent::SetUIMaterial(const USceneComponent* CanvasSceneComp,
	UMaterialInterface* InMaterial, UTexture* InTexture, const FLinearColor& InClipRect,
	const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy)
{
	SetVisibility(true);
}

void UUISelectedRectDrawerProxyComponent::UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority)
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

void UUISelectedRectDrawerProxyComponent::UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent)
{
	bUseVirtualWorldTransform = !bRemove;
	VirtualTransformID = InVirtualTransformID;
	VirtualWorldTransform = InVirtualWorldTransform;

	bUpdateVirtualWorldTransform = true;
	MarkRenderDynamicDataDirty();
}

/////////////////////////////////////////////////////
