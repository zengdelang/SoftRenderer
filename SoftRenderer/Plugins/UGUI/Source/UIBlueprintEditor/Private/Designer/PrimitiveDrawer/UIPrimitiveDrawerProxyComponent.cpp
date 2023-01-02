#include "Designer/PrimitiveDrawer/UIPrimitiveDrawerProxyComponent.h"
#include "PrimitiveSceneProxy.h"
#include "PrimitiveSceneInfo.h"
#include "UGUISubsystem.h"
#include "Core/Layout/RectTransformPreviewComponent.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "PhysicalMaterials/PhysicalMaterialMask.h"
#include "Core/Render/CanvasSubComponent.h"

/////////////////////////////////////////////////////
// UUIPrimitiveDrawerProxyComponent

#ifndef ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY
#define ENABLE_RENDER_THREAD_SET_TRANSLUCENCY_PRIORITY 0
#endif

struct FUIDesignerRect
{
public:
	float XMin;
	float YMin;
	float XMax;
	float YMax;

	FVector2D AnchorMin;
	FVector2D AnchorMax;

	FVector2D SizeDelta;
	FRect ParentRect;
	
	FTransform LocalToWorld;
	FTransform ParentLocalToWorld;
	
	uint8 bIsValid : 1;
	uint8 bMultiSelected : 1;
	uint8 bSelfSelected : 1;
	uint8 bSiblingSelected : 1;
	uint8 bIsRootCanvas: 1;
	uint8 bIsParentSelected : 1;

public:
	FUIDesignerRect()
	{
		XMin = 0;
		YMin = 0;
		XMax = 0;
		YMax = 0;

		AnchorMin = FVector2D::ZeroVector;
		AnchorMax = FVector2D::ZeroVector;

		SizeDelta = FVector2D::ZeroVector;
		ParentRect = FRect();
		
		LocalToWorld = FTransform::Identity;
		ParentLocalToWorld = FTransform::Identity;
		
		bIsValid = false;

		bMultiSelected = false;
		bSelfSelected = false;
		bSiblingSelected = false;
		bIsRootCanvas = false;
		bIsParentSelected = false;
	}

	void Initialize(const FEditorUIDesignerRect& Rect)
	{
		bIsValid = Rect.Component.IsValid();
		
		XMin = Rect.XMin;
		YMin = Rect.YMin;
		XMax = Rect.XMax;
		YMax = Rect.YMax;

		AnchorMin = Rect.AnchorMin;
		AnchorMax = Rect.AnchorMax;

		SizeDelta = Rect.SizeDelta;
		ParentRect = Rect.ParentRect;
		
		bMultiSelected = Rect.bMultiSelected;
		bSelfSelected = Rect.bSelfSelected;
		bSiblingSelected = Rect.bSiblingSelected;
		bIsRootCanvas = Rect.bIsRootCanvas;
		bIsParentSelected = Rect.bIsParentSelected;
		
		LocalToWorld = Rect.LocalToWorld;
		ParentLocalToWorld = Rect.ParentLocalToWorld;
	}

	void DrawRect(FPrimitiveDrawInterface* PDI) const
	{
		if (!bIsValid)
			return;

		if (bMultiSelected && !bIsRootCanvas)
			return;
		
		FVector Verts[4];
		// Left-Bottom
		Verts[0] = FVector(XMin, YMin, 0);
		// Left-Top
		Verts[1] = FVector(XMin, YMax, 0);
		// Right-Top
		Verts[2] = FVector(XMax, YMax, 0);
		// Right-Bottom
		Verts[3] = FVector(XMax, YMin, 0);

		for (int32 X = 0; X < 4; ++X)
		{
			Verts[X] = LocalToWorld.TransformPosition(Verts[X]);
		}

		FLinearColor LineColor = FLinearColor::Gray;
		if (bIsParentSelected)
		{
			LineColor = FLinearColor(0.8, 0.8, 0.8);
		}
		else if (bSiblingSelected || bSelfSelected)
		{
			LineColor = FLinearColor(0.4, 0.4, 0.4);;
		}

		PDI->DrawLine(Verts[0], Verts[1], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(Verts[1], Verts[2], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(Verts[2], Verts[3], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(Verts[3], Verts[0], LineColor, SDPG_Foreground, 0, 0, true);
	}

	void DrawNegativeSizeRect(FPrimitiveDrawInterface* PDI) const
	{
		if (!bIsValid)
			return;

		FVector Verts[4];
		// Left-Bottom
		Verts[0] = FVector(XMin, YMin, 0);
		// Left-Top
		Verts[1] = FVector(XMin, YMax, 0);
		// Right-Top
		Verts[2] = FVector(XMax, YMax, 0);
		// Right-Bottom
		Verts[3] = FVector(XMax, YMin, 0);

		for (int32 X = 0; X < 4; ++X)
		{
			Verts[X] = LocalToWorld.TransformPosition(Verts[X]);
		}

		if (SizeDelta.X < 0 || SizeDelta.Y < 0)
		{
			PDI->DrawLine(Verts[0], Verts[2], FLinearColor::Red, SDPG_Foreground, 0, 0, true);
			PDI->DrawLine(Verts[1], Verts[3], FLinearColor::Red, SDPG_Foreground, 0, 0, true);
		}
	}

	void DrawAnchor(FPrimitiveDrawInterface* PDI, const FSceneView* View) const
	{
		if (!bIsValid)
			return;
		
		if (bIsRootCanvas)
			return;
		
		const FMatrix& Transform = View->ViewMatrices.GetViewProjectionMatrix();

		float OrthoZoomFactor = 1.0f;
		const bool bIsPerspective = View->ViewMatrices.GetProjectionMatrix().M[3][3] < 1.0f ? true : false;
		if (!bIsPerspective)
		{
			OrthoZoomFactor = 1.0f / View->ViewMatrices.GetProjectionMatrix().M[0][0];
		}

		const uint32 ViewportSizeX = View->UnscaledViewRect.Width();
		constexpr float Size = 30;
		
		FVector AnchorPoint[12];
		AnchorPoint[0] = FVector(ParentRect.XMin + ParentRect.Width * AnchorMin.X, ParentRect.YMin + ParentRect.Height * AnchorMin.Y, 0);
		AnchorPoint[3] = FVector(ParentRect.XMin + ParentRect.Width * AnchorMax.X, ParentRect.YMin + ParentRect.Height * AnchorMax.Y, 0);
		AnchorPoint[6] = FVector(AnchorPoint[0].X, AnchorPoint[3].Y, 0);
		AnchorPoint[9] = FVector(AnchorPoint[3].X, AnchorPoint[0].Y, 0);
		
		{
			const FVector AnchorPos = ParentLocalToWorld.TransformPosition(AnchorPoint[0]);

			float DeltaX = 0;
			float DeltaX2 = 0;
			float DeltaY = 0;
			float DeltaY2 = 0;
			GetAnchorDelta(DeltaX, DeltaX2, DeltaY, DeltaY2, Size, AnchorPos, Transform, OrthoZoomFactor, ViewportSizeX);

			AnchorPoint[0] = AnchorPos;
			AnchorPoint[1] = FVector(AnchorPoint[0].X - DeltaX, AnchorPoint[0].Y - DeltaY, 0);
			AnchorPoint[2] = FVector(AnchorPoint[0].X - DeltaX2, AnchorPoint[0].Y - DeltaY2, 0);
		}

		{
			const FVector AnchorPos = ParentLocalToWorld.TransformPosition(AnchorPoint[3]);

			float DeltaX = 0;
			float DeltaX2 = 0;
			float DeltaY = 0;
			float DeltaY2 = 0;
			GetAnchorDelta(DeltaX, DeltaX2, DeltaY, DeltaY2, Size, AnchorPos, Transform, OrthoZoomFactor, ViewportSizeX);

			AnchorPoint[3] = AnchorPos;
			AnchorPoint[4] = FVector(AnchorPoint[3].X + DeltaX, AnchorPoint[3].Y + DeltaY, 0);
			AnchorPoint[5] = FVector(AnchorPoint[3].X + DeltaX2, AnchorPoint[3].Y + DeltaY2, 0);
		}

		{
			const FVector AnchorPos = ParentLocalToWorld.TransformPosition(AnchorPoint[6]);

			float DeltaX = 0;
			float DeltaX2 = 0;
			float DeltaY = 0;
			float DeltaY2 = 0;
			GetAnchorDelta(DeltaX, DeltaX2, DeltaY, DeltaY2, Size, AnchorPos, Transform, OrthoZoomFactor, ViewportSizeX);

			AnchorPoint[6] = AnchorPos;
			AnchorPoint[7] = FVector(AnchorPoint[6].X - DeltaX, AnchorPoint[6].Y + DeltaY, 0);
			AnchorPoint[8] = FVector(AnchorPoint[6].X - DeltaX2, AnchorPoint[6].Y + DeltaY2, 0);
		}

		{
			const FVector AnchorPos = ParentLocalToWorld.TransformPosition(AnchorPoint[9]);

			float DeltaX = 0;
			float DeltaX2 = 0;
			float DeltaY = 0;
			float DeltaY2 = 0;
			GetAnchorDelta(DeltaX, DeltaX2, DeltaY, DeltaY2, Size, AnchorPos, Transform, OrthoZoomFactor, ViewportSizeX);

			AnchorPoint[9] = AnchorPos;
			AnchorPoint[10] = FVector(AnchorPoint[9].X + DeltaX, AnchorPoint[9].Y - DeltaY, 0);
			AnchorPoint[11] = FVector(AnchorPoint[9].X + DeltaX2, AnchorPoint[9].Y - DeltaY2, 0);
		}

		FLinearColor LineColor = FLinearColor(0.4, 0.4, 0.4);
		if (bSelfSelected)
		{
			LineColor = FLinearColor::White;
		}
		
		// Anchor Bottom-Left
		PDI->DrawLine(AnchorPoint[0], AnchorPoint[1], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[0], AnchorPoint[2], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[1], AnchorPoint[2], LineColor, SDPG_Foreground, 0, 0, true);

		// Anchor Top-Right
		PDI->DrawLine(AnchorPoint[3], AnchorPoint[4], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[3], AnchorPoint[5], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[4], AnchorPoint[5], LineColor, SDPG_Foreground, 0, 0, true);

		// Anchor Top-Left
		PDI->DrawLine(AnchorPoint[6], AnchorPoint[7], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[6], AnchorPoint[8], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[7], AnchorPoint[8], LineColor, SDPG_Foreground, 0, 0, true);

		// Anchor Top-Right
		PDI->DrawLine(AnchorPoint[9], AnchorPoint[10], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[9], AnchorPoint[11], LineColor, SDPG_Foreground, 0, 0, true);
		PDI->DrawLine(AnchorPoint[10], AnchorPoint[11], LineColor, SDPG_Foreground, 0, 0, true);
	}

protected:
	static void GetAnchorDelta(float& DeltaX, float& DeltaX2, float& DeltaY, float& DeltaY2, const float Size, const FVector& AnchorPos, const FMatrix& Transform, const float OrthoZoomFactor, const float ViewportSizeX)
	{
		const FVector4 TransformedPosition = Transform.TransformFVector4(AnchorPos);

		// Generate vertices for the point such that the post-transform point size is constant.
		const uint32 ViewportMajorAxis = ViewportSizeX;
		const float FinalSize = Size * OrthoZoomFactor / ViewportMajorAxis * TransformedPosition.W;
		
		DeltaX = FinalSize * FMath::Cos(30 / 180.0f * PI);
		DeltaX2 = FinalSize * FMath::Cos(60 / 180.0f * PI);
		DeltaY = FinalSize * FMath::Sin(30 / 180.0f * PI);
		DeltaY2 = FinalSize * FMath::Sin(60 / 180.0f * PI);
	}
};

struct FUIDesignerInfo
{
public:
	FUIDesignerRect RootRect;
	FUIDesignerRect ParentRect;
	FUIDesignerRect CurSelectedRect;

	TArray<FUIDesignerRect> CurSelectedSiblingRects;
	TArray<FUIDesignerRect> NegativeSizeRects;
};

class FUIPrimitiveDrawerSceneProxy : public FPrimitiveSceneProxy, public FUISceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FUIPrimitiveDrawerSceneProxy(UUIPrimitiveDrawerProxyComponent* Component)
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
			
		ProxyId = FUISceneViewExtension::NextProxyId.Increment();
		if (ProxyId == 0)
		{
			ProxyId = FUISceneViewExtension::NextProxyId.Increment();
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

		if (Component->GraphicData.IsValid())
		{
			const auto GraphicDataPtr = Component->GraphicData.Pin();
			if (GraphicDataPtr.IsValid())
			{
				GraphicData = GraphicDataPtr->CopyGraphicData();
			}
		}

		FEditorUIDesignerInfo* EditorUIDesignerInfo = URectTransformPreviewComponent::GetEditorUIDesignerInfo(Component->GetWorld());
		if (EditorUIDesignerInfo)
		{
			UIDesignerInfo.RootRect.Initialize(EditorUIDesignerInfo->RootRect);
			UIDesignerInfo.ParentRect.Initialize(EditorUIDesignerInfo->ParentRect);
			UIDesignerInfo.CurSelectedRect.Initialize(EditorUIDesignerInfo->CurSelectedRect);

			for (const auto& SiblingRect : EditorUIDesignerInfo->CurSelectedSiblingRects)
			{
				FUIDesignerRect DesignerRect;
				DesignerRect.Initialize(SiblingRect);
				UIDesignerInfo.CurSelectedSiblingRects.Emplace(DesignerRect);
			}

			for (const auto& NegativeSizeRect : EditorUIDesignerInfo->NegativeSizeRects)
			{
				if (NegativeSizeRect.Component.IsValid() &&
					(NegativeSizeRect.Component->SizeDelta.X < 0 || NegativeSizeRect.Component->SizeDelta.Y < 0))
				{
					FUIDesignerRect DesignerRect;
					DesignerRect.Initialize(NegativeSizeRect);
					UIDesignerInfo.NegativeSizeRects.Emplace(DesignerRect);
				}
			}
		}
	}

	virtual ~FUIPrimitiveDrawerSceneProxy() override
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
		
		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];

				for (const auto& DesignerRect : UIDesignerInfo.NegativeSizeRects)
				{
					DesignerRect.DrawNegativeSizeRect(PDI);
				}
				
				for (const auto& DesignerRect : UIDesignerInfo.CurSelectedSiblingRects)
				{
					DesignerRect.DrawRect(PDI);
					DesignerRect.DrawAnchor(PDI, View);
				}

				UIDesignerInfo.RootRect.DrawRect(PDI);

				UIDesignerInfo.ParentRect.DrawRect(PDI);
				
				UIDesignerInfo.CurSelectedRect.DrawRect(PDI);
				UIDesignerInfo.CurSelectedRect.DrawAnchor(PDI, View);
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

	FUIDesignerInfo UIDesignerInfo;

	int32 ScreenOverlaySortPriority;
	uint32 ProxyId;

	uint8 bDraw : 1;
	
};

//////////////////////////////////////////////////////////////////////////

UUIPrimitiveDrawerProxyComponent::UUIPrimitiveDrawerProxyComponent(const FObjectInitializer& ObjectInitializer)
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

	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
}

FPrimitiveSceneProxy* UUIPrimitiveDrawerProxyComponent::CreateSceneProxy()
{
	bUpdateTranslucentSortPriority = false;
	bUpdateVirtualWorldTransform = false;
	return new FUIPrimitiveDrawerSceneProxy(this);
}

void UUIPrimitiveDrawerProxyComponent::SendRenderDynamicData_Concurrent()
{
	if (bUpdateTranslucentSortPriority)
	{
		if (SceneProxy)
		{
			const int32 NewTranslucentSortPriority = TranslucencySortPriority;
			
			// Enqueue command to send to render thread
			FUIPrimitiveDrawerSceneProxy* UIPrimitiveDrawerSceneProxy = static_cast<FUIPrimitiveDrawerSceneProxy*>(SceneProxy);

			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateSingleRendererData)
				([UIPrimitiveDrawerSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
					{
						UIPrimitiveDrawerSceneProxy->UpdateTranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
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
			FUIPrimitiveDrawerSceneProxy* UIPrimitiveDrawerSceneProxy = (FUIPrimitiveDrawerSceneProxy*)SceneProxy;
			
			ENQUEUE_RENDER_COMMAND(FUIMeshUpdateVirtualWorldTransform)
			([InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform, UIPrimitiveDrawerSceneProxy](FRHICommandListImmediate& RHICmdList)
			{
				UIPrimitiveDrawerSceneProxy->UpdateVirtualWorldTransform_RenderThread(InVirtualTransformID, InVirtualWorldTransform, bRemoveVirtualWorldTransform);
			});
		}
	}

	bUpdateTranslucentSortPriority = false;
	
	Super::SendRenderDynamicData_Concurrent();
}

void UUIPrimitiveDrawerProxyComponent::SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> InViewExtension)
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

void UUIPrimitiveDrawerProxyComponent::ClearUIMesh()
{
	IUIRenderProxyInterface::ClearUIMesh();

	SetVisibility(false);
}

void UUIPrimitiveDrawerProxyComponent::SetUIMaterial(const USceneComponent* CanvasSceneComp,
	UMaterialInterface* InMaterial, UTexture* InTexture, const FLinearColor& InClipRect,
	const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy)
{
	SetVisibility(true);
}

void UUIPrimitiveDrawerProxyComponent::UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority)
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

void UUIPrimitiveDrawerProxyComponent::UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove, USceneComponent* CanvasAttachComponent)
{
	bUseVirtualWorldTransform = !bRemove;
	VirtualTransformID = InVirtualTransformID;
	VirtualWorldTransform = InVirtualWorldTransform;

	bUpdateVirtualWorldTransform = true;
	MarkRenderDynamicDataDirty();
}

/////////////////////////////////////////////////////
