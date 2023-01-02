#include "Core/Renderer/UIMeshProxyComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveViewRelevance.h"
#include "RenderResource.h"
#include "MaterialShared.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "Engine/Engine.h"
#include "SceneManagement.h"
#include "PrimitiveSceneInfo.h"
#include "StaticMeshResources.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialRelevance.h"
#include "UGUI.h"
#include "UGUIWorldSubsystem.h"
#include "Core/Render/UIMeshBatchElement.h"

/////////////////////////////////////////////////////
// UUIMeshProxyComponent

#ifndef ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
#define ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY 0
#endif

DECLARE_CYCLE_STAT(TEXT("UIRender --- GetDynamicMeshElements"), STAT_UnrealGUI_GetDynamicMeshElements, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIRender --- GetMeshElement"), STAT_UnrealGUI_GetMeshElement, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIRender --- GetScreenRect"), STAT_UnrealGUI_GetScreenRect, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIRender --- InitUIMeshBuffers"), STAT_UnrealGUI_InitUIMeshBuffers, STATGROUP_UnrealGUI);

/** Class representing a single section of the proc mesh */
class FUIProcMeshProxy
{
public:
	FPooledDynamicUIMeshVertexBuffer RHIVertexBuffers;
	
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;

	/** Index buffer for this section */
	FPooledDynamicUIMeshBuffer32 IndexBuffer;

	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;

public:
	FUIProcMeshProxy(ERHIFeatureLevel::Type InFeatureLevel)
		: VertexFactory(InFeatureLevel, "FUIProcMeshProxy")
	{

	}
};

/** UI mesh scene proxy */
class FUIMeshSceneProxy final : public FPrimitiveSceneProxy, public FUISceneProxy
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FUIMeshSceneProxy(const UUIMeshProxyComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, RenderMode(Component->RenderMode)
		, GraphicType(Component->GraphicType)
		, ProcMeshProxy(nullptr)
		, ScreenOverlaySortPriority(Component->TranslucencySortPriority)
		, IndexCount(0)
		, VerticesCount(0)
		, bHasInitializedResource(false)
	{
		ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		if (ProxyId == 0)
		{
			ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		}
		
		bUseVirtualWorldTransform = Component->bUseVirtualWorldTransform;
		VirtualWorldTransform = Component->VirtualWorldTransform;
		VirtualTransformID = Component->VirtualTransformID;
		
		bUseTextureArray = Component->bUseTextureArray;
		bRectClipping = Component->bRectClipping;
		ClipRect = Component->ClipRect;
		ClipSoftnessRect = Component->ClipSoftnessRect;

		MergedUIMesh = Component->MergedUIMesh;
		IndexCount = Component->BatchIndexCount;
		VerticesCount = Component->BatchVerticesCount;

		// Grab material
		if (RenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			Material = Component->MaterialInterface;
		}
		else
		{
			Material = Component->GetMaterial(0);
			
			UIMeshLocalBounds = MergedUIMesh->LocalBox;
			UIMeshBounds = UIMeshLocalBounds.TransformBy(GetLocalToWorld());

			FBoxSphereBounds* ProxyLocalBounds = const_cast<FBoxSphereBounds*>(&GetLocalBounds());
			*(ProxyLocalBounds) = UIMeshLocalBounds;

			FBoxSphereBounds* ProxyBounds = const_cast<FBoxSphereBounds*>(&GetBounds());
			*(ProxyBounds) = UIMeshBounds;
		}

		if (Material == nullptr)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

		MainTexture = Component->MainTexture;
		GraphicData = Component->GraphicData;
		
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

	virtual ~FUIMeshSceneProxy() override
	{
		ReleaseProcMeshProxy();

		if (ViewExtension.IsValid())
		{
			const auto ViewExtensionPtr = ViewExtension.Pin();
			if (ViewExtensionPtr.IsValid())
			{
				ViewExtensionPtr->RemoveUISceneProxy_RenderThread(ProxyId, GetPrimitiveComponentId().PrimIDValue);
			}
		}
		ViewExtension.Reset();

		MergedUIMesh.Reset();
		GraphicData.Reset();
	}

	void ReleaseProcMeshProxy()
	{
		if (ProcMeshProxy)
		{
			if (RenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace)
			{
				ProcMeshProxy->IndexBuffer.ReleaseResource();
				ProcMeshProxy->RHIVertexBuffers.ReleaseResource();
			}
			else
			{
				ProcMeshProxy->VertexBuffers.PositionVertexBuffer.ReleaseResource();
				ProcMeshProxy->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
				ProcMeshProxy->VertexBuffers.ColorVertexBuffer.ReleaseResource();
				ProcMeshProxy->IndexBuffer.ReleaseResource();
				ProcMeshProxy->VertexFactory.ReleaseResource();			
			}
			
			delete ProcMeshProxy;
			ProcMeshProxy = nullptr;
		}
	}

	void UpdateBackgroundBlurData() const
	{
		if (GraphicType == EUIGraphicType::PostProcessBlur && GraphicData.IsValid() && MergedUIMesh)
		{
			if (MergedUIMesh->Vertices.Num() > 0)
			{
				FUIBlurGraphicData* BlurGraphicData = static_cast<FUIBlurGraphicData*>(GraphicData.Get());
				BlurGraphicData->Alpha = MergedUIMesh->Vertices[0].Color.A / 255.0;
				BlurGraphicData->bComputeEffectiveKernelSize = true;
			}
			else
			{
				FUIBlurGraphicData* BlurGraphicData = static_cast<FUIBlurGraphicData*>(GraphicData.Get());
				BlurGraphicData->Alpha = 1;
				BlurGraphicData->bComputeEffectiveKernelSize = true;
			}
		}
	}

	void UpdateRenderData_RenderThread(const FRenderDataUpdateInfo& RenderDataUpdateInfo)
	{
		Material = RenderDataUpdateInfo.Material;
		MainTexture = RenderDataUpdateInfo.MainTexture;
		RenderMode = RenderDataUpdateInfo.RenderMode;
		GraphicType = RenderDataUpdateInfo.GraphicType;
		
		ReleaseProcMeshProxy();
		bHasInitializedResource = false;

		bUseVirtualWorldTransform = RenderDataUpdateInfo.bUseVirtualWorldTransform;
		VirtualWorldTransform = RenderDataUpdateInfo.VirtualWorldTransform;
		VirtualTransformID = RenderDataUpdateInfo.VirtualTransformID;
		
		bUseTextureArray = RenderDataUpdateInfo.bUseTextureArray;
		bRectClipping = RenderDataUpdateInfo.bRectClipping;
		ClipRect = RenderDataUpdateInfo.ClipRect;
		ClipSoftnessRect = RenderDataUpdateInfo.ClipSoftnessRect;

		MergedUIMesh = RenderDataUpdateInfo.MergedUIMesh;
		
		IndexCount = RenderDataUpdateInfo.BatchIndexCount;
		VerticesCount = RenderDataUpdateInfo.BatchVerticesCount;
		
		GraphicData = RenderDataUpdateInfo.GraphicData;
		
		if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			UIMeshLocalBounds = MergedUIMesh->LocalBox;
			UIMeshBounds = UIMeshLocalBounds.TransformBy(GetLocalToWorld());
			
			FBoxSphereBounds* ProxyLocalBounds = const_cast<FBoxSphereBounds*>(&GetLocalBounds());
			*(ProxyLocalBounds) = UIMeshLocalBounds;

			FBoxSphereBounds* ProxyBounds = const_cast<FBoxSphereBounds*>(&GetBounds());
			*(ProxyBounds) = UIMeshBounds;
		}

		if (ViewExtension != RenderDataUpdateInfo.ViewExtension)
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
			
			if (RenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace)
			{
				ViewExtension = RenderDataUpdateInfo.ViewExtension;
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

		UpdateTranslucentSortPriority_RenderThread(RenderDataUpdateInfo.TranslucentSortPriority);
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
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GetDynamicMeshElements);
		
		if (GraphicType == EUIGraphicType::PostProcessBlur || GraphicType == EUIGraphicType::PostProcessGlitch)
		{
			return;
		}

		if (RenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			return;
		}

		if (!bHasInitializedResource)
		{
			FUIMeshSceneProxy* ThisPtr = const_cast<FUIMeshSceneProxy*>(this);
			ThisPtr->bHasInitializedResource = true;
			ThisPtr->InitUIMeshBuffers_WorldSpace();
		}
		
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}
		
		if (ProcMeshProxy)
		{
			const FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Material->GetRenderProxy();

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &ProcMeshProxy->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &ProcMeshProxy->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;
					
					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), UIMeshBounds, UIMeshLocalBounds,
						false, false, false, false);
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
					
					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = IndexCount / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.NumInstances = 0;
					BatchElement.MaxVertexIndex = VerticesCount - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = true;

					Collector.AddMesh(ViewIndex, Mesh);
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
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return false;
	}

	virtual uint32 GetMemoryFootprint() const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize() const
	{
		return FPrimitiveSceneProxy::GetAllocatedSize();
	}

	//Begin FUISceneProxy interfaces
	
	virtual bool GetMeshElement(FMeshElementCollector* Collector, FMeshBatch& MeshBatch, TUniformBuffer<FPrimitiveUniformShaderParameters>* CanvasPrimitiveUniformBuffer, const FSceneViewFamily& ViewFamily) override
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GetMeshElement);

		if (!bHasInitializedResource)
		{
			bHasInitializedResource = true;
			InitUIMeshBuffers();
		}
		
		if (!ProcMeshProxy)
		{
			return false;
		}

		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector->RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}
		
		const FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Material->GetRenderProxy();
		
		// Draw the mesh.
		FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
		BatchElement.IndexBuffer = &ProcMeshProxy->IndexBuffer;
		BatchElement.PrimitiveIdMode = PrimID_ForceZero;
		MeshBatch.bWireframe = false;
		MeshBatch.MaterialRenderProxy = MaterialProxy;
		
		/*FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector->AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
		DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), UIMeshBounds, UIMeshLocalBounds,
			false, false, false, false);
		BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;*/

		BatchElement.PrimitiveUniformBufferResource = CanvasPrimitiveUniformBuffer;
		
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = IndexCount / 3;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = VerticesCount - 1;
		MeshBatch.Type = PT_TriangleList;
		
		return true;
	}

	void InitUIMeshBuffers_WorldSpace()
	{
		if (VerticesCount <= 0)
		{
			return;
		}

		if (!MergedUIMesh.IsValid())
		{
			return;
		}
		
		ProcMeshProxy = new FUIProcMeshProxy(GetScene().GetFeatureLevel());

		UIMeshLocalBounds = MergedUIMesh->LocalBox;
		UIMeshBounds = UIMeshLocalBounds.TransformBy(GetLocalToWorld());

		FBoxSphereBounds* ProxyLocalBounds = const_cast<FBoxSphereBounds*>(&GetLocalBounds());
		*(ProxyLocalBounds) = UIMeshLocalBounds;

		FBoxSphereBounds* ProxyBounds = const_cast<FBoxSphereBounds*>(&GetBounds());
		*(ProxyBounds) = UIMeshBounds;
		
		const auto& Vertices = MergedUIMesh->WorldSpaceVertices;
		ProcMeshProxy->VertexBuffers.PositionVertexBuffer.Init(Vertices.Num());
		ProcMeshProxy->VertexBuffers.StaticMeshVertexBuffer.Init(Vertices.Num(), MAX_STATIC_TEXCOORDS);
		ProcMeshProxy->VertexBuffers.ColorVertexBuffer.Init(Vertices.Num());

		for (int32 Index = 0, Count = Vertices.Num(); Index < Count; ++Index)
		{
			const FDynamicMeshVertex& Vertex = Vertices[Index];
			ProcMeshProxy->VertexBuffers.PositionVertexBuffer.VertexPosition(Index) = Vertex.Position;
			ProcMeshProxy->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(Index, Vertex.TangentX.ToFVector(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector());
					
			for (uint32 UVIndex = 0; UVIndex < MAX_STATIC_TEXCOORDS; ++UVIndex)
			{
				ProcMeshProxy->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(Index, UVIndex, Vertex.TextureCoordinate[UVIndex]);
			}
					
			ProcMeshProxy->VertexBuffers.ColorVertexBuffer.VertexColor(Index) = Vertex.Color;
		}

		FStaticMeshVertexBuffers* StaticMeshVertexBuffers = &ProcMeshProxy->VertexBuffers;
		StaticMeshVertexBuffers->PositionVertexBuffer.InitResource();
		StaticMeshVertexBuffers->StaticMeshVertexBuffer.InitResource();
		StaticMeshVertexBuffers->ColorVertexBuffer.InitResource();

		FLocalVertexFactory* VertexFactory = &ProcMeshProxy->VertexFactory;
						
		FLocalVertexFactory::FDataType Data;
		StaticMeshVertexBuffers->PositionVertexBuffer.BindPositionVertexBuffer(VertexFactory, Data);
		StaticMeshVertexBuffers->StaticMeshVertexBuffer.BindTangentVertexBuffer(VertexFactory, Data);
		StaticMeshVertexBuffers->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactory, Data);
		StaticMeshVertexBuffers->StaticMeshVertexBuffer.BindLightMapVertexBuffer(VertexFactory, Data, 0);
		StaticMeshVertexBuffers->ColorVertexBuffer.BindColorVertexBuffer(VertexFactory, Data);
		VertexFactory->SetData(Data);
		
		ProcMeshProxy->IndexBuffer.Indices = &MergedUIMesh->Indices;
		ProcMeshProxy->IndexBuffer.InitResource();

		VertexFactory->InitResource();
	}

	void InitUIMeshBuffers()
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_InitUIMeshBuffers);

		if (VerticesCount <= 0)
		{
			return;
		}
		
		if (!MergedUIMesh.IsValid())
		{
			return;
		}
		
		ProcMeshProxy = new FUIProcMeshProxy(GetScene().GetFeatureLevel());
		
		UIMeshLocalBounds = MergedUIMesh->LocalBox;
		UIMeshBounds = UIMeshLocalBounds.TransformBy(GetLocalToWorld());

		ProcMeshProxy->RHIVertexBuffers.Vertices = &MergedUIMesh->Vertices;
		ProcMeshProxy->RHIVertexBuffers.InitResource();

		ProcMeshProxy->IndexBuffer.Indices = &MergedUIMesh->Indices;
		ProcMeshProxy->IndexBuffer.InitResource();

		UpdateBackgroundBlurData();
	}

	virtual void OnTransformChanged() override
	{
		UIMeshBounds = UIMeshLocalBounds.TransformBy(GetLocalToWorld());

		if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			FBoxSphereBounds* ProxyLocalBounds = const_cast<FBoxSphereBounds*>(&GetLocalBounds());
			*(ProxyLocalBounds) = UIMeshLocalBounds;

			FBoxSphereBounds* ProxyBounds = const_cast<FBoxSphereBounds*>(&GetBounds());
			*(ProxyBounds) = UIMeshBounds;
		}
	}
	
	virtual int32 GetRenderPriority() const override
	{
		return ScreenOverlaySortPriority;
	}

	virtual FRHIVertexBuffer* GetVertexBufferRHI() override
	{
		if (ProcMeshProxy)
		{
			return ProcMeshProxy->RHIVertexBuffers.VertexBufferRHI;
		}
		return nullptr;
	}

	virtual uint32 GetNumVertices() override
	{
		return VerticesCount;
	}

	virtual FPrimitiveSceneProxy* GetPrimitiveSceneProxy() override
	{
		return this;
	}

	virtual EUIGraphicType GetGraphicType() override
	{
		return GraphicType;
	}

	virtual FUIGraphicData* GetGraphicData() override
	{
		if (GraphicData.IsValid())
		{
			GraphicData->UpdateGraphicData();
		}
		return GraphicData.Get();
	}

	virtual bool UseTextureArray() override
	{
		return bUseTextureArray;
	}

	virtual bool IsRectClipping() override
	{
		return bRectClipping;
	}
	
	virtual FLinearColor GetClipRect () override
	{
		return ClipRect;
	}

	virtual FLinearColor GetClipSoftnessRect () override
	{
		return ClipSoftnessRect;
	}

	virtual FMatrix GetLocalToWorldMatrix() override
	{
		return GetLocalToWorld();
	}

	virtual bool GetScreenRect(const FMatrix& InViewProjectionMatrix, FVector2D& TopLeftUV, FVector2D& BottomRightUV) override
	{
		SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_GetScreenRect);

		if (!bHasInitializedResource)
		{
			bHasInitializedResource = true;
			InitUIMeshBuffers();
		}
		
		float TopLeftX = MAX_flt;
		float TopLeftY = MAX_flt;
		float BottomRightX = -MAX_flt;
		float BottomRightY = -MAX_flt;

		if (MergedUIMesh.IsValid())
		{
			const auto LocalToProjectionMatrix = GetLocalToWorld() * InViewProjectionMatrix;
			const auto& Vertices = MergedUIMesh->Vertices;
			for (int32 Index = 0, Count = Vertices.Num(); Index < Count; ++Index)
			{
				const FVector4& ClipSpacePos = LocalToProjectionMatrix.TransformPosition(Vertices[Index].Position);
				if (ClipSpacePos.W > 0.0f)
				{
					// the result of this will be x and y coords in -1..1 projection space
					const float RHW = 1.0f / ClipSpacePos.W;

					// Move from projection space to normalized 0..1 UI space
					float NormalizedX = (ClipSpacePos.X * RHW * 0.5f) + 0.5f;
					//const float NormalizedY = 1.f - (ClipSpacePos.Y * RHW * 0.5f) - 0.5f;
					float NormalizedY = -(ClipSpacePos.Y * RHW * 0.5f) + 0.5f;

					const float NormalizedZ = ClipSpacePos.Z * RHW;

					if (NormalizedZ >= 0 && NormalizedZ <= 1)
					{
						NormalizedX = FMath::Clamp(NormalizedX, 0.f, 1.f);
						NormalizedY = FMath::Clamp(NormalizedY, 0.f, 1.f);

						if (NormalizedX < TopLeftX)
						{
							TopLeftX = NormalizedX;
						}

						if (NormalizedX > BottomRightX)
						{
							BottomRightX = NormalizedX;
						}

						if (NormalizedY < TopLeftY)
						{
							TopLeftY = NormalizedY;
						}

						if (NormalizedY > BottomRightY)
						{
							BottomRightY = NormalizedY;
						}
					}
				}
			}

			const float DeltaX = BottomRightX - TopLeftX;
			const float DeltaY = BottomRightY - TopLeftY;
			if (DeltaX <= 1 && DeltaX > 0 && DeltaY <= 1 && DeltaY > 0)
			{
				TopLeftUV.X = TopLeftX;
				TopLeftUV.Y = TopLeftY;
				BottomRightUV.X = BottomRightX;
				BottomRightUV.Y = BottomRightY;
				return true;
			}
		}

		return false;
	}

	virtual void GetMainTextureRHIResource(FTextureRHIRef& MainTextureRHIRef, FRHISamplerState*& MainTextureSamplerState) override
	{
		ETextureSamplerFilter Filter = ETextureSamplerFilter::Bilinear;
		
		if (MainTexture)
		{
			if (MainTexture->Resource)
			{
				const FTexture* TextureResource = MainTexture->Resource;
				TextureResource->LastRenderTime = FApp::GetCurrentTime();
				
				MainTextureRHIRef = TextureResource->TextureRHI;
				if (MainTextureRHIRef == nullptr)
				{
					MainTextureRHIRef = GTransparentBlackTexture->TextureRHI;
				}
			}

			Filter = GetSamplerFilter(MainTexture);
		}

		switch (Filter)
		{
		case ETextureSamplerFilter::Point:
			MainTextureSamplerState = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			break;
		case ETextureSamplerFilter::AnisotropicPoint:
			MainTextureSamplerState = TStaticSamplerState<SF_AnisotropicPoint, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			break;
		case ETextureSamplerFilter::Trilinear:
			MainTextureSamplerState = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			break;
		case ETextureSamplerFilter::AnisotropicLinear:
			MainTextureSamplerState = TStaticSamplerState<SF_AnisotropicLinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			break;
		default:
			MainTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			break;
		}
	}

	static ETextureSamplerFilter GetSamplerFilter(const UTexture* Texture)
	{
		// Default to point filtering.
		ETextureSamplerFilter Filter;

		switch (Texture->Filter)
		{
		case TF_Nearest: 
			Filter = ETextureSamplerFilter::Point; 
			break;
		case TF_Bilinear:
			Filter = ETextureSamplerFilter::Bilinear; 
			break;
		case TF_Trilinear: 
			Filter = ETextureSamplerFilter::Trilinear; 
			break;
		// TF_Default
		default:
			Filter = ETextureSamplerFilter::Bilinear; 
		}

		return Filter;
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

		if (!bHasInitializedResource)
		{
			bHasInitializedResource = true;
			InitUIMeshBuffers();
		}
		
		if (!bUseWorldTransform)
		{
			return ViewFrustum.IntersectBox(UIMeshBounds.Origin, UIMeshBounds.BoxExtent);
		}

		const FBoxSphereBounds& WorldBounds = UIMeshBounds.TransformBy(InVirtualWorldTransform);
		return ViewFrustum.IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent);
	}
	
	//End FUISceneProxy interfaces

private:
	FMaterialRelevance MaterialRelevance;
	UMaterialInterface* Material;

	UTexture* MainTexture;

	ECanvasRenderMode RenderMode;
	EUIGraphicType GraphicType;
	
	TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> GraphicData;
	
	TSharedPtr<FMergedUIMesh, ESPMode::ThreadSafe> MergedUIMesh;
	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;

	/** The primitive's bounds. */
	FBoxSphereBounds UIMeshBounds;

	/** The primitive's local space bounds. */
	FBoxSphereBounds UIMeshLocalBounds;
	
	FUIProcMeshProxy* ProcMeshProxy;

	int32 ScreenOverlaySortPriority;

	uint32 IndexCount;
	uint32 VerticesCount;
	uint32 ProxyId;

	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;
	uint8 bRectClipping : 1;
	
	uint8 bHasInitializedResource : 1;
	uint8 bUseTextureArray : 1;
	
};

//////////////////////////////////////////////////////////////////////////

UUIMeshProxyComponent::UUIMeshProxyComponent(const FObjectInitializer& ObjectInitializer)
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

	MainTexture = nullptr;
	MaterialInterface = nullptr;
	
	RenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
	GraphicType = EUIGraphicType::UIMesh;
	
	bRectClipping = false;
	ClipRect = FLinearColor(0, 0, 0, 0);
	ClipSoftnessRect = FLinearColor(0, 0, 0, 0);
		
	bUseTextureArray = false;
	bRefreshRenderData = false;
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
	bUpdateSceneProxyTransform = false;
}

DECLARE_CYCLE_STAT(TEXT("UIRender --- CreateSceneProxy"), STAT_UnrealGUI_CreateSceneProxy, STATGROUP_UnrealGUI);
FPrimitiveSceneProxy* UUIMeshProxyComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_CreateSceneProxy);

	bRefreshRenderData = false;
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
	bUpdateSceneProxyTransform = false;
	
	if (!MergedUIMesh.IsValid())
		return nullptr;
	
	return new FUIMeshSceneProxy(this);
}

int32 UUIMeshProxyComponent::GetNumMaterials() const
{
	return OverrideMaterials.Num();
}

void UUIMeshProxyComponent::CreateRenderState_Concurrent(FRegisterComponentContext* Context)
{
	UActorComponent::CreateRenderState_Concurrent(Context);

	// If the primitive isn't hidden and the detail mode setting allows it, add it to the scene.
	if (ShouldComponentAddToScene())
	{
		if (Context != nullptr)
		{
			Context->AddPrimitive(this);
		}
		else
		{
			GetWorld()->Scene->AddPrimitive(this);
		}
	}
}

void UUIMeshProxyComponent::SendRenderTransform_Concurrent()
{
	if (bUpdateSceneProxyTransform && ShouldRender())
	{
		bUpdateSceneProxyTransform = false;
		
		// Update the scene info's transform for this primitive.
		GetWorld()->Scene->UpdatePrimitiveTransform(this);
	}

	UActorComponent::SendRenderTransform_Concurrent();
}

void UUIMeshProxyComponent::SendRenderDynamicData_Concurrent()
{
	if (bRefreshRenderData)
	{
		bRefreshRenderData = false;

		if (SceneProxy)
		{
			FUIMeshSceneProxy* ProcMeshSceneProxy = static_cast<FUIMeshSceneProxy*>(SceneProxy);

			FRenderDataUpdateInfo RenderDataUpdateInfo;
			RenderDataUpdateInfo.RenderMode = RenderMode;
			RenderDataUpdateInfo.ViewExtension = ViewExtension;
			RenderDataUpdateInfo.GraphicType = GraphicType;
			RenderDataUpdateInfo.GraphicData = GraphicData;
			RenderDataUpdateInfo.MainTexture = MainTexture;
			
			RenderDataUpdateInfo.Material = MaterialInterface;
			if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
			{
				RenderDataUpdateInfo.Material = GetMaterial(0);
			}
			
			if (RenderDataUpdateInfo.Material == nullptr)
			{
				RenderDataUpdateInfo.Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			RenderDataUpdateInfo.bUseVirtualWorldTransform = bUseVirtualWorldTransform;
			RenderDataUpdateInfo.VirtualWorldTransform = VirtualWorldTransform;
			RenderDataUpdateInfo.VirtualTransformID = VirtualTransformID;

			RenderDataUpdateInfo.bUseTextureArray = bUseTextureArray;
			RenderDataUpdateInfo.bRectClipping = bRectClipping;
			RenderDataUpdateInfo.ClipRect = ClipRect;
			RenderDataUpdateInfo.ClipSoftnessRect = ClipSoftnessRect;

			RenderDataUpdateInfo.MergedUIMesh = MergedUIMesh;
			RenderDataUpdateInfo.BatchVerticesCount = BatchVerticesCount;
			RenderDataUpdateInfo.BatchIndexCount = BatchIndexCount;

			RenderDataUpdateInfo.TranslucentSortPriority = TranslucencySortPriority;
			
			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([ProcMeshSceneProxy, RenderDataUpdateInfo = MoveTemp(RenderDataUpdateInfo)](FRHICommandListImmediate& RHICmdList)
					{
						ProcMeshSceneProxy->UpdateRenderData_RenderThread(RenderDataUpdateInfo);
					});	
		}
	}
	else if (bUpdateTranslucentSortPriority)
	{
		if (SceneProxy)
		{
			const int32 NewTranslucentSortPriority = TranslucencySortPriority;
			
			// Enqueue command to send to render thread
			FUIMeshSceneProxy* ProcMeshSceneProxy = static_cast<FUIMeshSceneProxy*>(SceneProxy);

			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([ProcMeshSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
					{
						ProcMeshSceneProxy->UpdateTranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
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
			FUIMeshSceneProxy* UIMeshSceneProxy = (FUIMeshSceneProxy*)SceneProxy;
			
			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateVirtualWorldTransform)
			([InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform, UIMeshSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				UIMeshSceneProxy->UpdateVirtualWorldTransform_RenderThread(InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform);
			});
		}
	}

	bUpdateTranslucentSortPriority = false;
	
	Super::SendRenderDynamicData_Concurrent();
}

void UUIMeshProxyComponent::SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> InViewExtension)
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
		if (SceneProxy)
		{
			bRefreshRenderData = true;
			MarkRenderDynamicDataDirty();
		}
		else
		{
			MarkRenderStateDirty();
		}
	}
}

void UUIMeshProxyComponent::SetGraphicType(EUIGraphicType InGraphicType)
{
	if (GraphicType != InGraphicType)
	{
		GraphicType = InGraphicType;

		if (SceneProxy)
		{
			bRefreshRenderData = true;
			MarkRenderDynamicDataDirty();
		}
		else
		{
			MarkRenderStateDirty();
		}
	}
}

void UUIMeshProxyComponent::SetGraphicData(TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> InGraphicData)
{
	if (GraphicData != InGraphicData)
	{
		GraphicData = InGraphicData;

		if (SceneProxy)
		{
			bRefreshRenderData = true;
			MarkRenderDynamicDataDirty();
		}
		else
		{
			MarkRenderStateDirty();
		}
	}
}

void UUIMeshProxyComponent::ClearUIMesh()
{
	IUIRenderProxyInterface::ClearUIMesh();

	GraphicData.Reset();
	UIMeshBatchElement.Reset();
	MergedUIMesh.Reset();

	DirtyRenderers.Reset();
	
	bRefreshRenderData = false;

	if (UMaterialInstanceDynamic* MaterialInstanceDynamic = Cast<UMaterialInstanceDynamic>(GetMaterial(0)))
	{
		if (MaterialInstanceDynamic == MaterialInterface)
		{
			OverrideMaterials[0] = nullptr;
		}
		else
		{
			if (UIWorldSubsystem.IsValid())
			{
				UIWorldSubsystem->AddUnusedMaterial(MaterialInstanceDynamic->Parent, MaterialInstanceDynamic);
			}
			
			SetMaterial(0, nullptr);
		}
	}

	MainTexture = nullptr;
	MaterialInterface = nullptr;
	
	if (UIWorldSubsystem.IsValid())
	{
		UIWorldSubsystem->AddUnusedMesh(this);
	}
	
	SetVisibility(false);
}

void UUIMeshProxyComponent::SetUIMaterial(const USceneComponent* CanvasSceneComp,  UMaterialInterface* InMaterial, UTexture* InTexture,
	const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy)
{
	SetVisibility(true);

	if (!UIWorldSubsystem.IsValid())
	{
		if (const auto World = GetWorld())
		{
			UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
		}
	}

	if (UMaterialInstanceDynamic* MaterialInstanceDynamic = Cast<UMaterialInstanceDynamic>(GetMaterial(0)))
	{
		if (MaterialInstanceDynamic == MaterialInterface)
		{
			OverrideMaterials[0] = nullptr;
		}
		else if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			if (MaterialInstanceDynamic->Parent != MaterialInterface)
			{
				if (UIWorldSubsystem.IsValid())
				{
					UIWorldSubsystem->AddUnusedMaterial(MaterialInstanceDynamic->Parent, MaterialInstanceDynamic);
				}
				
				OverrideMaterials[0] = nullptr;
			}
		}
		else
		{
			OverrideMaterials[0] = nullptr;
		}
	}
	
	bRectClipping = bInRectClipping;
	ClipRect = InClipRect;
	ClipSoftnessRect = InClipSoftnessRect;
	bUseTextureArray = bInUseTextureArray;

	MainTexture = InTexture;
	MaterialInterface = InMaterial;
	
	if (MaterialInterface)
	{
		MaterialInterface->AddToCluster(this, true);
	}
	
	if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
	{
		if (UMaterialInstanceDynamic* MaterialInstanceDynamic = Cast<UMaterialInstanceDynamic>(MaterialInterface))
		{
			MaterialInstanceDynamic->SetTextureParameterValue(TEXT("MainTexture"), MainTexture);
			MaterialInstanceDynamic->SetScalarParameterValue(TEXT("bRectClipping"), bInRectClipping);
			MaterialInstanceDynamic->SetVectorParameterValue(TEXT("ClipRect"), ClipRect);
			MaterialInstanceDynamic->SetVectorParameterValue(TEXT("ClipSoftnessRect"), ClipSoftnessRect);
			SetMaterial(0, MaterialInstanceDynamic);
		}
		else if (UMaterialInstanceDynamic* NewMaterialInstance = Cast<UMaterialInstanceDynamic>(GetMaterial(0)))
		{
			NewMaterialInstance->SetTextureParameterValue(TEXT("MainTexture"), MainTexture);
			NewMaterialInstance->SetScalarParameterValue(TEXT("bRectClipping"), bInRectClipping);
			NewMaterialInstance->SetVectorParameterValue(TEXT("ClipRect"), ClipRect);
			NewMaterialInstance->SetVectorParameterValue(TEXT("ClipSoftnessRect"), ClipSoftnessRect);
		}
		else if (UIWorldSubsystem.IsValid())
		{
			if (UMaterialInstanceDynamic* NewInstance = UIWorldSubsystem->GetUnusedMaterial(MaterialInterface))
			{
				NewInstance->SetTextureParameterValue(TEXT("MainTexture"), MainTexture);
				NewInstance->SetScalarParameterValue(TEXT("bRectClipping"), bInRectClipping);
				NewInstance->SetVectorParameterValue(TEXT("ClipRect"), ClipRect);
				NewInstance->SetVectorParameterValue(TEXT("ClipSoftnessRect"), ClipSoftnessRect);
	
				SetMaterial(0, NewInstance);
			}
		}
		else
		{
			SetMaterial(0, nullptr);
		}
	}
	
	if (bRefreshRenderProxy)
	{
		DirtyRenderers.Reset();
		
		MergedUIMesh = MakeShareable(new FMergedUIMesh());

		if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			UIMeshBatchElement->MergeVertexBuffer(MergedUIMesh->WorldSpaceVertices, MergedUIMesh->LocalBox, BatchVerticesCount, WorldToCanvasTransform);
			bUpdateSceneProxyTransform = true;
			MarkRenderTransformDirty();
		}
		else
		{
			UIMeshBatchElement->MergeRHIBuffer(MergedUIMesh->Vertices, RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay, MergedUIMesh->LocalBox, BatchVerticesCount, WorldToCanvasTransform);
		}
		
		UIMeshBatchElement->MergeIndexBuffer(MergedUIMesh->Indices, BatchIndexCount);
		
		if (SceneProxy)
		{
			bRefreshRenderData = true;
			MarkRenderDynamicDataDirty();
		}
		else
		{
			MarkRenderStateDirty();
		}
	}
}

void UUIMeshProxyComponent::UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority)
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

void UUIMeshProxyComponent::UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent)
{
	bUseVirtualWorldTransform = !bRemove;
	VirtualTransformID = InVirtualTransformID;
	VirtualWorldTransform = InVirtualWorldTransform;

	bUpdateVirtualWorldTransform = true;
	MarkRenderDynamicDataDirty();
}

void UUIMeshProxyComponent::SetupCanvas(const UCanvasSubComponent* Canvas, TSharedPtr<FUIRenderProxyInfo> InProxyInfo, TSharedPtr<FUIMeshBatchElement> InUIMeshBatchElement,
                                        const int32 InBatchVerticesCount, const int32 InBatchIndexCount, bool& bRefreshRenderProxy)
{
	ProxyInfo = InProxyInfo;
	if (ProxyInfo.IsValid())
	{
		ProxyInfo->OwnerRenderProxy = this;
	}

	if (UIMeshBatchElement.IsValid() && ProxyInfo->CanvasRenderers.Num() != UIMeshBatchElement->UIMeshBatchSections.Num())
	{
		bRefreshRenderProxy = true;
	}
	
	UIMeshBatchElement = InUIMeshBatchElement;
	BatchVerticesCount = InBatchVerticesCount;
	BatchIndexCount = InBatchIndexCount;
	
	if (IsValid(Canvas))
	{
		if (Canvas->bUseVirtualWorldTransform && IsValid(Canvas->AttachTransform))
		{
			WorldToCanvasTransform = Canvas->AttachTransform->GetComponentTransform().Inverse();

			bUseVirtualWorldTransform = true;
			VirtualWorldTransform = Canvas->VirtualWorldTransform.ToMatrixWithScale();
			VirtualTransformID = Canvas->GetUniqueID();
		}
		else
		{
			bUseVirtualWorldTransform = false;
			WorldToCanvasTransform = GetComponentTransform().Inverse();
		}
	}
	else
	{
		bUseVirtualWorldTransform = false;
		VirtualTransformID = 0;
	}
}

void UUIMeshProxyComponent::UpdateDirtyRenderers()
{
	if (!MergedUIMesh.IsValid())
		return;

	if (!UIMeshBatchElement.IsValid())
		return;
	
	for (auto& Renderer : DirtyRenderers)
	{
		if (!Renderer.IsValid())
		{
			continue;
		}

		const auto& RenderProxyInfo = Renderer->OwnerRenderProxyInfo;
		if (!RenderProxyInfo.IsValid())
		{
			continue;
		}

		if (const int32* SectionIndexPtr = RenderProxyInfo.Pin()->CanvasRenderers.Find(Renderer))
		{
			if (!UIMeshBatchElement->UIMeshBatchSections.IsValidIndex(*SectionIndexPtr))
			{
				continue;
			}

			FUIMeshBatchSection& Section = UIMeshBatchElement->UIMeshBatchSections[*SectionIndexPtr];
			
			if (Renderer->bUpdateUV1)
			{
				Renderer->bUpdateUV1 = false;
				
				const FUIVertex* VertexPtr = Renderer->Mesh->GetVertex(0);
				if (!VertexPtr)
				{
					continue;
				}

				const FVector2D UV1 = VertexPtr->UV1;
				
				const TSharedPtr<FMergedUIMesh, ESPMode::ThreadSafe> NewMergedUIMesh = MakeShareable(new FMergedUIMesh());

				NewMergedUIMesh->LocalBox = MergedUIMesh->LocalBox;
				
				NewMergedUIMesh->Indices.SetNumZeroed(MergedUIMesh->Indices.Num());
				FMemory::Memcpy(NewMergedUIMesh->Indices.GetData(), MergedUIMesh->Indices.GetData(), MergedUIMesh->Indices.Num() * sizeof(uint32));
				
				if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
				{
					auto& WorldSpaceVertices = NewMergedUIMesh->WorldSpaceVertices;
					WorldSpaceVertices.SetNumZeroed(MergedUIMesh->WorldSpaceVertices.Num());
					FMemory::Memcpy(WorldSpaceVertices.GetData(), MergedUIMesh->WorldSpaceVertices.GetData(), WorldSpaceVertices.Num() * sizeof(FDynamicMeshVertex));

					for (int32 Index = Section.VertexStartIndex, TotalCount = WorldSpaceVertices.Num(), Count = Section.VertexStartIndex + Section.Mesh->GetCurrentVerticesCount(); Index < Count && Index < TotalCount; ++Index)
					{
						WorldSpaceVertices[Index].TextureCoordinate[1] = UV1;
					}
				}
				else
				{
					auto& Vertices = NewMergedUIMesh->Vertices;
					Vertices.SetNumZeroed(MergedUIMesh->Vertices.Num());
					FMemory::Memcpy(Vertices.GetData(), MergedUIMesh->Vertices.GetData(), Vertices.Num() * sizeof(FUIVertex));

					for (int32 Index = Section.VertexStartIndex, TotalCount = Vertices.Num(), Count = Section.VertexStartIndex + Section.Mesh->GetCurrentVerticesCount(); Index < Count && Index < TotalCount; ++Index)
					{
						Vertices[Index].UV1 = UV1;
					}
				}

				MergedUIMesh = NewMergedUIMesh;

				bRefreshRenderData = true;
				MarkRenderDynamicDataDirty();
			}

			if (Renderer->bUpdateInheritedAlpha)
			{
				Renderer->bUpdateInheritedAlpha = false;

				Section.InheritedAlpha = Renderer->InheritedAlpha;
				Section.Color = Renderer->Color;
				
				const TSharedPtr<FMergedUIMesh, ESPMode::ThreadSafe> NewMergedUIMesh = MakeShareable(new FMergedUIMesh());

				NewMergedUIMesh->LocalBox = MergedUIMesh->LocalBox;
				
				NewMergedUIMesh->Indices.SetNumZeroed(MergedUIMesh->Indices.Num());
				FMemory::Memcpy(NewMergedUIMesh->Indices.GetData(), MergedUIMesh->Indices.GetData(), MergedUIMesh->Indices.Num() * sizeof(uint32));

#if PLATFORM_LITTLE_ENDIAN
				const VectorRegister ColorRegister = MakeVectorRegister(Section.Color.B, Section.Color.G, Section.Color.R, Section.Color.A * Section.InheritedAlpha);
#else // PLATFORM_LITTLE_ENDIAN
				const VectorRegister ColorRegister = MakeVectorRegister(Section.Color.A * Section.InheritedAlpha, Section.Color.R, Section.Color.G, Section.Color.B);
#endif
				
				if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
				{
					auto& WorldSpaceVertices = NewMergedUIMesh->WorldSpaceVertices;
					WorldSpaceVertices.SetNumZeroed(MergedUIMesh->WorldSpaceVertices.Num());
					FMemory::Memcpy(WorldSpaceVertices.GetData(), MergedUIMesh->WorldSpaceVertices.GetData(), WorldSpaceVertices.Num() * sizeof(FDynamicMeshVertex));

					int32 VertexIndex = 0;
					const auto& RenderMesh = Renderer->Mesh;
					for (int32 Index = Section.VertexStartIndex, TotalCount = WorldSpaceVertices.Num(), Count = Section.VertexStartIndex + Section.Mesh->GetCurrentVerticesCount(); Index < Count && Index < TotalCount; ++Index)
					{
						FastBlendLinearColorToFColor(ColorRegister, (void*)&RenderMesh->GetVertex(VertexIndex)->Color, &WorldSpaceVertices[Index].Color);
						++VertexIndex;
					}
				}
				else
				{
					auto& Vertices = NewMergedUIMesh->Vertices;
					Vertices.SetNumZeroed(MergedUIMesh->Vertices.Num());
					FMemory::Memcpy(Vertices.GetData(), MergedUIMesh->Vertices.GetData(), Vertices.Num() * sizeof(FUIVertex));

					int32 VertexIndex = 0;
					const auto& RenderMesh = Renderer->Mesh;
					for (int32 Index = Section.VertexStartIndex, TotalCount = Vertices.Num(), Count = Section.VertexStartIndex + Section.Mesh->GetCurrentVerticesCount(); Index < Count && Index < TotalCount; ++Index)
					{
						FastBlendLinearColorToFColor(ColorRegister, (void*)&RenderMesh->GetVertex(VertexIndex)->Color, &Vertices[Index].Color);
						++VertexIndex;
					}
				}

				MergedUIMesh = NewMergedUIMesh;

				bRefreshRenderData = true;
				MarkRenderDynamicDataDirty();
			}
		}
	}

	DirtyRenderers.Reset();
}

/////////////////////////////////////////////////////
