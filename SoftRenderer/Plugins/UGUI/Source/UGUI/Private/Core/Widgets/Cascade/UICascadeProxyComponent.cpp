#include "Core/Widgets/Cascade/UICascadeProxyComponent.h"
#include "ParticleHelper.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UUICascadeProxyComponent

#ifndef ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
#define ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY 0
#endif

class FUICascadeSceneProxy
#if SUPPORT_UI_CASCADE
	: public FParticleSystemSceneProxy, public FUISceneProxy
#else
	: public FPrimitiveSceneProxy, public FUISceneProxy
#endif
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FUICascadeSceneProxy(const UUICascadeProxyComponent* Component, FParticleDynamicData* InDynamicData, bool bCanBeOccluded)
#if SUPPORT_UI_CASCADE
		: FParticleSystemSceneProxy(Component, InDynamicData, bCanBeOccluded)
#else
		: FPrimitiveSceneProxy(Component)
#endif
		, RenderMode(Component->RenderMode)
		, GraphicType(Component->GraphicType)
		, ScreenOverlaySortPriority(Component->TranslucencySortPriority)
	{
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

	virtual ~FUICascadeSceneProxy() override
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

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override
	{
#if SUPPORT_UI_CASCADE
		FPrimitiveViewRelevance Result = FParticleSystemSceneProxy::GetViewRelevance(View);
#else
		FPrimitiveViewRelevance Result = FPrimitiveSceneProxy::GetViewRelevance(View);
#endif
		
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

	virtual FPrimitiveSceneProxy * GetPrimitiveSceneProxy() override
	{
		return this;
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
};

//////////////////////////////////////////////////////////////////////////

UUICascadeProxyComponent::UUICascadeProxyComponent(const FObjectInitializer& ObjectInitializer)
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

	RenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
	GraphicType = EUIGraphicType::FX;

	GraphicEffects = FVector2D::ZeroVector;
	RenderOpacity = 0;

	bCanPlayFx = false;
	bNeedPlayFX = false;
	bReset = false;
	
	bUpdateTranslucentSortPriority = false;
}

FPrimitiveSceneProxy* UUICascadeProxyComponent::CreateSceneProxy()
{
	bUpdateTranslucentSortPriority = false;

#if SUPPORT_UI_CASCADE
	FUICascadeSceneProxy* NewProxy = nullptr;

	//@fixme EmitterInstances.Num() check should be here to avoid proxies for dead emitters but there are some edge cases where it happens for emitters that have just activated...
	//@fixme Get non-instanced path working in ES!
	if ((IsActive() == true)/** && (EmitterInstances.Num() > 0)*/ && Template)
	{
		UE_LOG(LogUGUI, Verbose,
			TEXT("CreateSceneProxy @ %fs %s bIsActive=%d"), GetWorld()->TimeSeconds,
			Template != NULL ? *Template->GetName() : TEXT("NULL"), IsActive());

		if (EmitterInstances.Num() > 0)
		{
			CacheViewRelevanceFlags(Template);
		}

		// Create the dynamic data for rendering this particle system.
		bParallelRenderThreadUpdate = true;
		FParticleDynamicData* ParticleDynamicData = CreateDynamicData(GetScene()->GetFeatureLevel());
		bParallelRenderThreadUpdate = false;

		if (CanBeOccluded())
		{
			Template->CustomOcclusionBounds.IsValid = true;
			NewProxy = new FUICascadeSceneProxy(this, ParticleDynamicData, true);
		}
		else
		{
			NewProxy = new FUICascadeSceneProxy(this, ParticleDynamicData, false);
		}

		check(NewProxy);
		if (ParticleDynamicData)
		{
			for (int32 Index = 0; Index < ParticleDynamicData->DynamicEmitterDataArray.Num(); Index++)
			{
				NewProxy->QueueVertexFactoryCreation(ParticleDynamicData->DynamicEmitterDataArray[Index]);
			}
		}
	}

	return NewProxy;
#else
	// Need to modify the engine source code
	//
	// Add the following code to the ParticleHelper.h file
	// 
	// Original code :
	// //class FParticleSystemSceneProxy final : public FPrimitiveSceneProxy
	// class ENGINE_API FParticleSystemSceneProxy : public FPrimitiveSceneProxy
	//
	//---------------------------------------------------------------------------------
	// 
	// Add the following code to the ParticleSystemComponent.h file
	//
	// 1
	//	    uint8 bParallelRenderThreadUpdate : 1;
	//
	// ---------->
	//
	// #if SUPPORT_UI_CASCADE
	// public:
	// #endif
	// 		uint8 bParallelRenderThreadUpdate : 1;
	// 		
	// 2
	//
	// /** Cache view relevance flags. */
	// void CacheViewRelevanceFlags(class UParticleSystem* TemplateToCache);
	//
	// ---------->
	//
	// #if SUPPORT_UI_CASCADE
	// public:
	// #endif
	// /** Cache view relevance flags. */
	// void CacheViewRelevanceFlags(class UParticleSystem* TemplateToCache);
	//
	//---------------------------------------------------------------------------------
	//
	// Add the following code to the Engine.Build.cs file
	// 
	// PublicDefinitions.Add("SUPPORT_UI_CASCADE=1");
	//
	UE_LOG(LogUGUI, Warning, TEXT("The engine source code needs to be modified, otherwise UICascade component is not supported"));	
	return nullptr;
#endif
}

void UUICascadeProxyComponent::SendRenderDynamicData_Concurrent()
{
	if (bUpdateTranslucentSortPriority)
	{
		if (SceneProxy)
		{
			const int32 NewTranslucentSortPriority = TranslucencySortPriority;
			
			// Enqueue command to send to render thread
			FUICascadeSceneProxy* UICascadeSceneProxy = static_cast<FUICascadeSceneProxy*>(SceneProxy);

			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([UICascadeSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
					{
						UICascadeSceneProxy->UpdateTranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
					});
		}
	}

	bUpdateTranslucentSortPriority = false;
	
	Super::SendRenderDynamicData_Concurrent();
}

#if SUPPORT_UI_CASCADE

bool UUICascadeProxyComponent::CanConsiderInvisible() const
{
	return false;
}

#endif

void UUICascadeProxyComponent::SetGraphicEffects(FVector2D InGraphicEffects, bool bUpdateMaterial)
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

void UUICascadeProxyComponent::SetRenderOpacity(float InRenderOpacity, bool bUpdateMaterial)
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

void UUICascadeProxyComponent::SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> InViewExtension)
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

void UUICascadeProxyComponent::ClearUIMesh()
{
	IUIRenderProxyInterface::ClearUIMesh();

	SetVisibility(false);
}

void UUICascadeProxyComponent::SetUIMaterial(const USceneComponent* CanvasSceneComp, UMaterialInterface* InMaterial, UTexture* InTexture,
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

				FMatrix WorldToCanvas = CanvasSceneComp->GetComponentTransform().Inverse().ToMatrixWithScale();

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

void UUICascadeProxyComponent::UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority)
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

void UUICascadeProxyComponent::SetupVirtualWorldTransform(bool bInUseVirtualWorldTransform, FTransform InVirtualWorldTransform, USceneComponent* CanvasAttachComponent)
{
	if (bInUseVirtualWorldTransform)
	{
		const USceneComponent* ProxyParentComponent = GetAttachParent();
		if (!IsValid(ProxyParentComponent) || !IsValid(CanvasAttachComponent))
		{
			return;
		}
		
		const FTransform ParentToCanvas = (ProxyParentComponent->GetComponentTransform() * CanvasAttachComponent->GetComponentTransform().Inverse());
		SetWorldTransform(FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector) * ParentToCanvas * InVirtualWorldTransform);
	}
	else
	{
		SetRelativeTransform(FTransform(FRotator(-90, 90, 0), FVector::ZeroVector, FVector::OneVector));
	}

	bCanPlayFx = true;

	if (bNeedPlayFX)
	{
		bNeedPlayFX = false;
		Activate(bReset);
	}
}

void UUICascadeProxyComponent::UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent)
{
	SetupVirtualWorldTransform(true, FTransform(InVirtualWorldTransform), CanvasAttachComponent);
}

/////////////////////////////////////////////////////
