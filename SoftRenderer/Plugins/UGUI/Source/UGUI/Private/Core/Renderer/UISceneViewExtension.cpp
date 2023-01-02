#include "Core/Renderer/UISceneViewExtension.h"
#include "Core/Renderer/UIMeshElementCollector.h"
#include "Renderer/Private/SceneRendering.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Renderer/UIBlurPostProcessor.h"
#include "Core/Renderer/UIShadingRenderer.h"
#include "Core/Renderer/UIViewInfo.h"
#include "ScenePrivate.h"
#include "SceneView.h"
#include "UGUISubsystem.h"
#include "Core/Renderer/UIVertex.h"
#include "UGUI.h"
#include "Engine/TextureRenderTarget2D.h"

#ifndef USE_CAMERA_CULLING_MASK
#define USE_CAMERA_CULLING_MASK 0
#endif

#if WITH_EDITOR
#include "Engine.h"
#endif

TAutoConsoleVariable<int32> CVarDisableRendering(
	TEXT("UGUI.DisableRendering"),
	0,
	TEXT("0 - Enable rendering.")
	TEXT("1 - Disable rendering."));

TAutoConsoleVariable<int32> CVarDisablePostProcessRendering(
	TEXT("UGUI.DisablePostProcess"),
	0,
	TEXT("0 - Enable post process rendering.")
	TEXT("1 - Disable post process rendering."), ECVF_Scalability);

TAutoConsoleVariable<int32> CVarMaxGaussianBlurCount(
	TEXT("UGUI.MaxGaussianBlurCount"),
	-1,
	TEXT("-1 means infinite times."), ECVF_Scalability);
	
TAutoConsoleVariable<int32> CVarMaxGlitchCount(
	TEXT("UGUI.MaxGlitchCount"),
	-1,
	TEXT("-1 means infinite times."), ECVF_Scalability);

TAutoConsoleVariable<int32> CVarShowUIBatches(
	TEXT("UGUI.ShowUIBatches"),
	0,
	TEXT("0 - Don't show UI batches.")
	TEXT("1 - Show UI batches."));

TGlobalResource<FSinglePrimitiveStructured> GSinglePrimitiveBuffer;

FGlobalDynamicIndexBuffer FUISceneViewExtension::DynamicIndexBuffer;
FGlobalDynamicVertexBuffer FUISceneViewExtension::DynamicVertexBuffer;
TGlobalResource<FGlobalDynamicReadBuffer> FUISceneViewExtension::DynamicReadBuffer;

// 0 is reserved to mean invalid
FThreadSafeCounter FUISceneViewExtension::NextProxyId;

FUISceneViewExtension::FUISceneViewExtension(const FAutoRegister& AutoRegister, UCanvasSubComponent* InCanvas)
	: FSceneViewExtensionBase(AutoRegister)
	, BlurPostProcessor(new FUIBlurPostProcessor())
	, GlitchPostProcessor(new FUIGlitchPostProcessor())
{
	ProjectionMode = ECameraProjectionMode::Perspective;
	
	Canvas = InCanvas;
	if (Canvas.IsValid())
	{
		RootCanvas = Canvas->GetRootCanvas();
		ProjectionMode = Canvas->GetProjectionType();
	}
	
	Priority = 0;
	if (Canvas.IsValid())
	{
		Priority = -Canvas->GetSortingOrder();
	}

	UIBatches = 0;
	BlurUIBatches = 0;
	GlitchUIBatches = 0;
	
	RenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay;
	
	LastDepthStencilSize = FIntPoint(0, 0);

	GameThread_RenderTimes = -1;
	RenderThread_RenderTimes = -1;

	bGammaCorrect = true;
	bSortUIProxies = false;
	bPerformFrustumCull = false;
	bCheckSceneViewVisibility = true;
	
	IntermediateDepthStencil = new FUIDepthStencilResource();
	BeginInitResource(IntermediateDepthStencil);

	CanvasPrimitiveBuffer = new FUICanvasPrimitiveBuffer();
	BeginInitResource(CanvasPrimitiveBuffer);

	RenderTargetResource = nullptr;
	RenderTargetMode = ECanvasRenderTargetMode::BackBuffer;
	
	CanvasLocalToWorld = FMatrix::Identity;

	Views.Reserve(1);
}

FUISceneViewExtension::~FUISceneViewExtension()
{
	// Note this is deleted automatically because it implements FDeferredCleanupInterface.
	IntermediateDepthStencil->CleanUp();
	
	CanvasPrimitiveBuffer->CleanUp();
}

void FUISceneViewExtension::AddUISceneProxy(uint32 ProxyId, uint32 ComponentId, FUISceneProxy* ProxyPtr)
{
    TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension = SharedThis(this);
		ENQUEUE_RENDER_COMMAND(FUISceneViewExtensionAddUISceneProxy)(
			[ViewExtension, ProxyId, ComponentId, ProxyPtr](FRHICommandListImmediate& RHICmdList)
			{
				if (ViewExtension.IsValid())
				{
					const auto ViewExtensionPtr = ViewExtension.Pin();
					if (ViewExtensionPtr.IsValid())
					{
						ViewExtensionPtr->AddUISceneProxy_RenderThread(ProxyId, ComponentId, ProxyPtr);
					}
				}
			}
		);
}

void FUISceneViewExtension::AddUISceneProxy_RenderThread(uint32 ProxyId, uint32 ComponentId, FUISceneProxy* ProxyPtr)
{
	check(IsInRenderingThread());

	if (ProxyPtr)
	{
		ProxyPtr->UICanvasPrimitiveBuffer = CanvasPrimitiveBuffer;

		if (ProxyPtr->bUseVirtualWorldTransform)
		{
			CanvasPrimitiveBuffer->AddPrimitiveUniformBufferCount(ProxyPtr->VirtualTransformID, ProxyPtr->VirtualWorldTransform);
		}
	}
	
	RenderThread_UIProxies.Remove(ComponentId);
	RenderThread_UIProxies.Emplace(FUIMeshProxyElement(ProxyId, ComponentId, ProxyPtr));
	bSortUIProxies = true;
}

void FUISceneViewExtension::RemoveUISceneProxy_RenderThread(uint32 ProxyId, uint32 ComponentId)
{
	check(IsInRenderingThread());

	const FUIMeshProxyElement* Proxy = RenderThread_UIProxies.Get(ComponentId);
	if (Proxy && Proxy->ProxyId == ProxyId)
	{
		if (Proxy->SceneProxyPtr)
		{
			if (Proxy->SceneProxyPtr->bUseVirtualWorldTransform)
			{
				CanvasPrimitiveBuffer->RemovePrimitiveUniformBuffer(Proxy->SceneProxyPtr->VirtualTransformID);
			}
			
			Proxy->SceneProxyPtr->UICanvasPrimitiveBuffer = nullptr;
		}
		
		RenderThread_UIProxies.Remove(ComponentId);
		bSortUIProxies = true;
	}
}

void FUISceneViewExtension::SortRenderPriority()
{
	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension = SharedThis(this);
	ENQUEUE_RENDER_COMMAND(FUISceneViewExtensionSortRenderPriority)(
		[ViewExtension](FRHICommandListImmediate& RHICmdList)
		{
			if (ViewExtension.IsValid())
			{
				const auto ViewExtensionPtr = ViewExtension.Pin();
				if (ViewExtensionPtr.IsValid())
				{
					ViewExtensionPtr->SortRenderPriority_RenderThread();
				}
			}
		}
	);
}

void FUISceneViewExtension::SortRenderPriority_RenderThread()
{
	check(IsInRenderingThread());

	bSortUIProxies = true;
}

void FUISceneViewExtension::FlushGeneratedResources() const
{
	BlurPostProcessor->ReleaseRenderTargets();
	GlitchPostProcessor->ReleaseRenderTargets();

	// Only release the resource not delete it.  Deleting it could cause issues on any RHI thread
	BeginReleaseResource(IntermediateDepthStencil);
	BeginReleaseResource(CanvasPrimitiveBuffer);
}

DECLARE_CYCLE_STAT(TEXT("UIRender --- SetupViewFamily"), STAT_UnrealGUI_SetupViewFamily, STATGROUP_UnrealGUI);
void FUISceneViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_SetupViewFamily);
	
	check(IsInGameThread());
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (CVarShowUIBatches.GetValueOnGameThread() == 1)
	{
		if (GEngine && Canvas.IsValid())
		{
			GEngine->AddOnScreenDebugMessage(static_cast<uint64>(Canvas->GetUniqueID()), 0, FColor::Blue, FString::Format(TEXT("Canvas : {0}\n  Total UI Batches : {1} \n    Blur UI Batches : {2}    Glitch UI Batches : {3}"),
				{ Canvas->GetOuter()->GetName(),UIBatches, BlurUIBatches, GlitchUIBatches })); 
		}
	}
#endif
	
#if WITH_EDITOR
	if (Canvas.IsValid())
	{
		Canvas->UIBatches = UIBatches;
		Canvas->BlurUIBatches = BlurUIBatches;
		Canvas->GlitchUIBatches = GlitchUIBatches;
	}
	
	if (RootCanvas.IsValid() && RootCanvas->GetWorld() && !RootCanvas->GetWorld()->IsGameWorld())
	{
		if (!RootCanvas->bForceUseSelfRenderMode)
		{
			const FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(RootCanvas->GetWorld());
			if (WorldViewportInfo)
			{
				RenderMode = WorldViewportInfo->GetRenderMode();
			}
		}
	}
#endif
	
	if (!RootCanvas.IsValid())
		return;

	if (RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree)
	{
		bPerformFrustumCull = RootCanvas->IsPerformFrustumCull();
		bCheckSceneViewVisibility = RootCanvas->IsCheckSceneViewVisibility();
		
		const USceneComponent* CanvasSceneComp = Cast<USceneComponent>(RootCanvas->GetOuter());
		if (IsValid(CanvasSceneComp))
		{
			CanvasLocalToWorld = CanvasSceneComp->GetComponentTransform().ToMatrixWithScale();
		}
	}
	else if (RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
	{
		RootCanvas->CalculateCanvasMatrices(ViewLocation, ViewRotationMatrix, ProjectionMatrix, ViewProjectionMatrix, ProjectionMode, CanvasLocalToWorld);
	}

#if WITH_EDITOR
	if (Canvas.IsValid() && Canvas->GetWorld() && !Canvas->GetWorld()->IsGameWorld())
	{
		bPerformFrustumCull = false;
		bCheckSceneViewVisibility = false;
		return;
	}
#endif
	
	if (Canvas.IsValid())
	{
		RenderTargetMode = Canvas->GetRenderTargetMode();
		
		if (Canvas->GetRenderTargetMode() != ECanvasRenderTargetMode::BackBuffer)
		{
			if (GameThread_RenderTimes != Canvas->GetRenderTimes() || Canvas->IsUpdateRenderTimes())
			{
				GameThread_RenderTimes = Canvas->GetRenderTimes();
				RenderThread_RenderTimes = GameThread_RenderTimes;
				Canvas->SetUpdateRenderTimes(false);
			}
			
			if (Canvas->IsOverrideSorting() || Canvas->IsRootCanvas())
			{
				UTextureRenderTarget2D* RenderTarget2D = Canvas->GetRenderTarget();
				if (IsValid(RenderTarget2D))
				{
					RenderTargetResource = RenderTarget2D->GameThread_GetRenderTargetResource();
				}
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIRender --- PostRenderView_RenderThread"), STAT_UnrealGUI_PostRenderView_RenderThread, STATGROUP_UnrealGUI);
void FUISceneViewExtension::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_PostRenderView_RenderThread);
	
	const bool bDisableRendering = CVarDisableRendering.GetValueOnRenderThread() > 0;
	if (bDisableRendering)
	{
		return;
	}

	if (RenderTargetMode != ECanvasRenderTargetMode::BackBuffer && (RenderTargetResource == nullptr || RenderThread_RenderTimes == 0))
	{
		return;
	}
	
	check(IsInRenderingThread());
	
	if (RenderThread_UIProxies.Num() <= 0)
		return;
	
	if (bSortUIProxies)
	{
		bSortUIProxies = false;
		RenderThread_UIProxies.Sort();
	}
	
	FUIViewInfo RenderView(&InView); //use a copied view

	bool bDoPerformFrustumCull = bPerformFrustumCull;
	bool bDoCheckSceneViewVisibility = bCheckSceneViewVisibility;
	
	const bool bUseExternalRenderTarget = RenderTargetResource != nullptr;
	const FRenderTarget* BackBuffer = bUseExternalRenderTarget ? RenderTargetResource : RenderView.Family->RenderTarget;
	FIntPoint BufferSize = BackBuffer->GetSizeXY();
	FIntRect ViewRect;

	FVector2D ViewportUIOffset = FVector2D::ZeroVector;
	
	if (RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
	{
		bDoPerformFrustumCull = false;
		bDoCheckSceneViewVisibility = false;

#if USE_CAMERA_CULLING_MASK
		RenderView.CullingMask = 0xFFFFFFFF;
#endif
		
		RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ProjectionMatrix);

		BufferSize.X = FMath::Max(1, BufferSize.X);
		BufferSize.Y = FMath::Max(1, BufferSize.Y);
		ViewRect.Max = BufferSize;
	}
	else
	{
		RenderView.UpdateProjectionMatrix(RenderView.ViewMatrices.GetProjectionNoAAMatrix());
		ViewProjectionMatrix = RenderView.ViewMatrices.GetViewProjectionMatrix();
		ViewRect = InView.UnscaledViewRect;

		ViewportUIOffset = FVector2D(ViewRect.Min.X / static_cast<float>(ViewRect.Width()), ViewRect.Min.Y / static_cast<float>(ViewRect.Height()));
	}

	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	// Scene render targets may not be created yet; avoids NaNs.
	FIntPoint EffectiveBufferSize = SceneContext.GetBufferSizeXY();
	EffectiveBufferSize.X = FMath::Max(EffectiveBufferSize.X, 1);
	EffectiveBufferSize.Y = FMath::Max(EffectiveBufferSize.Y, 1);

	RenderView.SetupCommonViewUniformBufferParameters(
			*RenderView.CachedViewUniformShaderParameters,
			EffectiveBufferSize,
			0,
			ViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);
	
	RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(*RenderView.CachedViewUniformShaderParameters, UniformBuffer_SingleFrame);

	const ERHIFeatureLevel::Type FeatureLevel = RenderView.GetFeatureLevel();
	
	FUIMeshElementCollector MeshCollector(RenderView.GetFeatureLevel());
	MeshCollector.ClearUIViewMeshArrays();
	MeshCollector.AddUIViewMeshArrays(&RenderView, &RenderView.DynamicMeshElements, &RenderView.SimpleElementCollector, &RenderView.DynamicPrimitiveShaderData, FeatureLevel,
		&DynamicIndexBuffer, &DynamicVertexBuffer, &DynamicReadBuffer);

	Views.Reset();
	Views.Add(&RenderView);
	
	FUIShadingRenderer UIShadingRenderer;
	
	auto& BackBufferTexture = BackBuffer->GetRenderTargetTexture();
	float EngineGamma = (!GIsEditor && BackBufferTexture->GetFormat() == PF_FloatRGBA) ? 1.0 : GEngine ? GEngine->GetDisplayGamma() : 2.2f;
	if (bUseExternalRenderTarget)
	{
		if (BackBuffer->GetDisplayGamma() > 0)
		{
			EngineGamma = BackBuffer->GetDisplayGamma();
		}
		else
		{
			EngineGamma = 1;
		}
	}
	
	const float DisplayGamma = bGammaCorrect ? EngineGamma : 1.0f;
	bool bSwitchVerticalAxis = !bUseExternalRenderTarget && RHINeedsToSwitchVerticalAxis(GShaderPlatformForFeatureLevel[GMaxRHIFeatureLevel]);
	
	bool bUpdateDepthStencil = false;
	CanvasPrimitiveBuffer->Update(CanvasLocalToWorld);
	
	// Can't reference scene color in scene textures. Scene color copy is used instead.
	ESceneTextureSetupMode SceneTextureSetupMode = ESceneTextureSetupMode::All;
	EnumRemoveFlags(SceneTextureSetupMode, ESceneTextureSetupMode::SceneColor);
	FUniformBufferRHIRef PassUniformBuffer = CreateSceneTextureUniformBufferDependentOnShadingPath(RHICmdList, FeatureLevel, SceneTextureSetupMode);
	FUniformBufferStaticBindings GlobalUniformBuffers(PassUniformBuffer);
	SCOPED_UNIFORM_BUFFER_GLOBAL_BINDINGS(RHICmdList, GlobalUniformBuffers);
	
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();

	int32 GaussianBlurCount = 0;
	int32 GlitchCount = 0;

	UIBatches = 0;
	BlurUIBatches = 0;
	GlitchUIBatches = 0;

	int32 PassCount = 0;
	
	for (int32 Index = 0, Count = RenderThread_UIProxies.Num(); Index < Count; ++Index)
	{
		auto& UIProxy = RenderThread_UIProxies[Index];
		auto UISceneProxy = UIProxy.SceneProxyPtr;
		if (UISceneProxy == nullptr)
		{
			continue;
		}

		if (bDoPerformFrustumCull && !UISceneProxy->IntersectViewFrustum(RenderView.ViewFrustum, UISceneProxy->bUseVirtualWorldTransform,
			UISceneProxy->bUseVirtualWorldTransform ? CanvasPrimitiveBuffer->GetVirtualWorldTransform(UISceneProxy->VirtualTransformID) : FMatrix()))
		{
			continue;
		}
		
		auto PrimitiveSceneProxy = UISceneProxy->GetPrimitiveSceneProxy();
		if (PrimitiveSceneProxy == nullptr)
		{
			continue;
		}

		if (bDoCheckSceneViewVisibility && !PrimitiveSceneProxy->IsShown(&RenderView))
		{
			continue;
		}
		
		MeshCollector.ClearMeshBatches();

		TUniformBuffer<FPrimitiveUniformShaderParameters>* SceneProxyPrimitiveBuffer = nullptr;
		if (UISceneProxy->bUseVirtualWorldTransform)
		{
			SceneProxyPrimitiveBuffer = CanvasPrimitiveBuffer->GetPrimitiveUniformBuffer(UISceneProxy->VirtualTransformID);
			if (!SceneProxyPrimitiveBuffer)
			{
				SceneProxyPrimitiveBuffer = CanvasPrimitiveBuffer->GetCanvasPrimitiveUniformBuffer();
			}
		}
		else
		{
			SceneProxyPrimitiveBuffer = CanvasPrimitiveBuffer->GetCanvasPrimitiveUniformBuffer();
		}

		if (UISceneProxy->GetGraphicType() == EUIGraphicType::UIMesh)
		{
			const uint32 NumVertices = UISceneProxy->GetNumVertices();
			if (NumVertices > 0)
			{
				FMeshBatch MeshBatch;
				if (UISceneProxy->GetMeshElement((FMeshElementCollector*)&MeshCollector, MeshBatch, SceneProxyPrimitiveBuffer, *RenderView.Family))
				{
					if (!RHICmdList.IsInsideRenderPass())
					{
						RHICmdList.Transition(FRHITransitionInfo(BackBufferTexture, ERHIAccess::Unknown, ERHIAccess::SRVGraphics));

						const ERenderTargetActions ColorAction = (bUseExternalRenderTarget && PassCount == 0) ? ERenderTargetActions::Clear_Store : ERenderTargetActions::Load_Store;
						++PassCount;
						FRHIRenderPassInfo RPInfo(BackBuffer->GetRenderTargetTexture(), ColorAction);
						
						RHICmdList.BeginRenderPass(RPInfo, TEXT("UGUIScreenSpaceElementRender"));
						{
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
						}
						
						RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
						RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0, ViewRect.Max.X, ViewRect.Max.Y, 0.0f);
					}
					
					auto Material = MeshBatch.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
					const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
					auto VertexShader = MaterialShaderMap->GetShader<FUIMaterialShaderVS>();
					auto PixelShader = MaterialShaderMap->GetShader<FUIMaterialShaderPS>();
					if (VertexShader.IsValid() && PixelShader.IsValid())
					{
						PixelShader->SetBlendState(GraphicsPSOInit, Material);

						GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetUIScreenVertexDeclaration();
						GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
						GraphicsPSOInit.DepthStencilTargetFormat = BackBufferTexture->GetFormat();
						GraphicsPSOInit.DepthStencilTargetFlag = BackBufferTexture->GetFlags();
						GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
						UpdateRasterizerState(*Material, GraphicsPSOInit);
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

						VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, MeshBatch.MaterialRenderProxy, Material, MeshBatch);
						PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, MeshBatch.MaterialRenderProxy, Material, MeshBatch);

						VertexShader->SetVerticalAxisMultiplier(RHICmdList, bSwitchVerticalAxis ? -1.0f : 1.0f);
						PixelShader->SetVerticalAxisMultiplier(RHICmdList, bSwitchVerticalAxis ? -1.0f : 1.0f);
						
						const FVector2D GammaValues(2.2f / DisplayGamma, 1.0f / DisplayGamma);
						PixelShader->SetGammaValues(RHICmdList, GammaValues);

						PixelShader->SetViewportUIOffset(RHICmdList, ViewportUIOffset);
						
						PixelShader->SetRectClip(RHICmdList, UISceneProxy->IsRectClipping() ? 1 : 0, UISceneProxy->GetClipRect(), UISceneProxy->GetClipSoftnessRect());
						
						FTextureRHIRef MainTextureRHIRef = GWhiteTexture->TextureRHI;
						FRHISamplerState* MainTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
						UISceneProxy->GetMainTextureRHIResource(MainTextureRHIRef, MainTextureSamplerState);

						if (UISceneProxy->UseTextureArray())
						{
							PixelShader->SetMainTextureArray(RHICmdList, MainTextureSamplerState, MainTextureRHIRef);
						}
						else
						{
							PixelShader->SetMainTexture(RHICmdList, MainTextureSamplerState, MainTextureRHIRef);
						}

						++UIBatches;
						RHICmdList.SetStreamSource(0, UISceneProxy->GetVertexBufferRHI(), 0);
						RHICmdList.DrawIndexedPrimitive(MeshBatch.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0,
							NumVertices, 0, MeshBatch.GetNumPrimitives(), 1);
					}
				}
			}
		}
		else if (UISceneProxy->GetGraphicType() == EUIGraphicType::PostProcessBlur)
		{
			const bool bDisablePostProcessRendering = CVarDisablePostProcessRendering.GetValueOnRenderThread() > 0;
			if (bDisablePostProcessRendering)
			{
				continue;
			}
			
			++GaussianBlurCount;
			const int32 MaxGaussianBlurCount = CVarMaxGaussianBlurCount.GetValueOnRenderThread();
			if (MaxGaussianBlurCount >= 0 && GaussianBlurCount > MaxGaussianBlurCount)
			{
				continue;
			}
			
			const uint32 NumVertices = UISceneProxy->GetNumVertices();
			if (NumVertices > 0)
			{
				const auto BlurGraphicData = static_cast<FUIBlurGraphicData*>(UISceneProxy->GetGraphicData());
				if (BlurGraphicData && BlurGraphicData->Strength > 0)
				{
					FVector2D TopLeftUV = FVector2D::ZeroVector;
					FVector2D BottomRightUV = FVector2D::ZeroVector;
					if (!UISceneProxy->GetScreenRect(ViewProjectionMatrix, TopLeftUV, BottomRightUV))
					{
						continue;
					}

					if (RHICmdList.IsInsideRenderPass())
					{
						RHICmdList.EndRenderPass();
					}

					FUIBlurPostProcessRectParams RectParams;
					FTexture2DRHIRef PostProcessTexture = BackBuffer->GetRenderTargetTexture();
					RectParams.SourceTexture = PostProcessTexture->GetTexture2D();
					RectParams.SourceRect = FUIRect(0, 0, PostProcessTexture->GetSizeX(), PostProcessTexture->GetSizeY());
					const auto& SizeXY = PostProcessTexture->GetSizeXY();
					RectParams.DestRect = FUIRect(TopLeftUV * SizeXY, BottomRightUV * SizeXY);
					RectParams.SourceTextureSize = PostProcessTexture->GetSizeXY();

					FUIBlurRectParams BlurParams;
					BlurParams.KernelSize = BlurGraphicData->KernelSize;
					BlurParams.Strength = BlurGraphicData->Strength;
					BlurParams.DownsampleAmount = BlurGraphicData->DownsampleAmount;

					FUIBlurGraphicDataParams GraphicDataParams;
					GraphicDataParams.bRectClipping = UISceneProxy->IsRectClipping();
					GraphicDataParams.ClipRect = UISceneProxy->GetClipRect();
					GraphicDataParams.ClipSoftnessRect = UISceneProxy->GetClipSoftnessRect();
					
					if (!BlurPostProcessor->BlurRect(RHICmdList, BlurParams, RectParams, GraphicDataParams, UISceneProxy, ViewProjectionMatrix, TopLeftUV, BottomRightUV, UIBatches, BlurUIBatches))
					{
						check(RHICmdList.IsOutsideRenderPass());
						continue;
					}

					FMeshBatch MeshBatch;
					if (UISceneProxy->GetMeshElement((FMeshElementCollector*)&MeshCollector, MeshBatch, SceneProxyPrimitiveBuffer, *RenderView.Family))
					{
						++UIBatches;
						++BlurUIBatches;
						RHICmdList.SetStreamSource(0, UISceneProxy->GetVertexBufferRHI(), 0);
						RHICmdList.DrawIndexedPrimitive(MeshBatch.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0,
							NumVertices, 0, MeshBatch.GetNumPrimitives(), 1);
					}

					RHICmdList.EndRenderPass();

					check(RHICmdList.IsOutsideRenderPass());
				}
			}
		}
		else if (UISceneProxy->GetGraphicType() == EUIGraphicType::PostProcessGlitch)
		{
			const bool bDisablePostProcessRendering = CVarDisablePostProcessRendering.GetValueOnRenderThread() > 0;
			if (bDisablePostProcessRendering)
			{
				continue;
			}

			++GlitchCount;
			const int32 MaxGlitchCount = CVarMaxGlitchCount.GetValueOnRenderThread();
			if (MaxGlitchCount >= 0 && GlitchCount > MaxGlitchCount)
			{
				continue;
			}
			
			const uint32 NumVertices = UISceneProxy->GetNumVertices();
			if (NumVertices > 0)
			{
				const auto GlitchGraphicData = static_cast<FUIGlitchGraphicData*>(UISceneProxy->GetGraphicData());
				if (GlitchGraphicData && GlitchGraphicData->bUseGlitch)
				{
					FVector2D TopLeftUV = FVector2D::ZeroVector;
					FVector2D BottomRightUV = FVector2D::ZeroVector;
					if (!UISceneProxy->GetScreenRect(ViewProjectionMatrix, TopLeftUV, BottomRightUV))
					{
						continue;
					}

					if (RHICmdList.IsInsideRenderPass())
					{
						RHICmdList.EndRenderPass();
					}

					FUIPostProcessRectParams RectParams;
					FTexture2DRHIRef PostProcessTexture = BackBuffer->GetRenderTargetTexture();
					RectParams.SourceTexture = PostProcessTexture->GetTexture2D();
					RectParams.SourceRect = FUIRect(0, 0, PostProcessTexture->GetSizeX(), PostProcessTexture->GetSizeY());
					const auto& SizeXY = PostProcessTexture->GetSizeXY();
					RectParams.DestRect = FUIRect(TopLeftUV * SizeXY, BottomRightUV * SizeXY);
					RectParams.SourceTextureSize = PostProcessTexture->GetSizeXY();

					FUIGlitchRectParams GlitchParams;
					GlitchParams.Strength = GlitchGraphicData->Strength;
					GlitchParams.TimeEtc.X = FPlatformTime::Seconds() - GStartTime;
					GlitchParams.TimeEtc.Y = FApp::GetCurrentTime() - GStartTime;
					GlitchParams.TimeEtc.Z = static_cast<float>(GlitchGraphicData->Method);
					GlitchParams.DownSampleAmount = GlitchGraphicData->DownSampleAmount;
					GlitchParams.MaskTextureRHI = GWhiteTexture->TextureRHI;
					GlitchParams.MaskSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					if (EUIGlitchType::UIGlitchType_AnalogNoiseAndRGBSplit == GlitchGraphicData->Method)
					{
						GlitchParams.GlitchParams1.X = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.AnalogNoiseSpeed;
						GlitchParams.GlitchParams1.Y = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.AnalogNoiseTexScaleX;
						GlitchParams.GlitchParams1.Z = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.AnalogNoiseTexScaleY;
						GlitchParams.GlitchParams1.W = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.AnalogNoiseFading;

						GlitchParams.GlitchParams2.X = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.RGBSplitSpeed;
						GlitchParams.GlitchParams2.Y = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.RGBSplitAmplitude;
						GlitchParams.GlitchParams2.Z = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.RGBSplitAmount;

						if (IsValid(GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.MaskTexture)
							&& NULL != GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.MaskTexture->Resource)
						{
							GlitchParams.MaskTextureRHI = GlitchGraphicData->UIGlitchAnalogNoiseAndRGBSplitSet.MaskTexture->Resource->GetTexture2DRHI();
						}
					}
					else if (EUIGlitchType::UIGlitchType_ImageBlock == GlitchGraphicData->Method)
					{
						GlitchParams.TimeEtc.W = GlitchGraphicData->UIGlitchImageBlockSet.GlitchSpeed;

						GlitchParams.GlitchParams1.X = GlitchGraphicData->UIGlitchImageBlockSet.BlockSizeX;
						GlitchParams.GlitchParams1.Y = GlitchGraphicData->UIGlitchImageBlockSet.BlockSizeY;
						GlitchParams.GlitchParams1.Z = GlitchGraphicData->UIGlitchImageBlockSet.OffsetScaleX;
						GlitchParams.GlitchParams1.W = GlitchGraphicData->UIGlitchImageBlockSet.OffsetScaleY;
					}

					if (!GlitchPostProcessor->GlitchRect(RHICmdList, GlitchParams, RectParams, UISceneProxy->GetLocalToWorldMatrix(), ViewProjectionMatrix, TopLeftUV, BottomRightUV, UIBatches, GlitchUIBatches))
					{
						check(RHICmdList.IsOutsideRenderPass());
						continue;
					}

					FMeshBatch MeshBatch;
					if (UISceneProxy->GetMeshElement((FMeshElementCollector*)&MeshCollector, MeshBatch, SceneProxyPrimitiveBuffer, *RenderView.Family))
					{
						++UIBatches;
						++GlitchUIBatches;
						RHICmdList.SetStreamSource(0, UISceneProxy->GetVertexBufferRHI(), 0);
						RHICmdList.DrawIndexedPrimitive(MeshBatch.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, NumVertices, 0, MeshBatch.GetNumPrimitives(), 1);
					}

					RHICmdList.EndRenderPass();

					check(RHICmdList.IsOutsideRenderPass());
				}
			}
		}
		else
		{
			if (RHICmdList.IsInsideRenderPass())
			{
				RHICmdList.EndRenderPass();
			}

			if (UISceneProxy->GetGraphicType() != EUIGraphicType::UIMesh)
			{
				if (PrimitiveSceneProxy->DoesVFRequirePrimitiveUniformBuffer() && PrimitiveSceneProxy->GetUniformBuffer() == nullptr)
				{
					continue;
				}
			}
			
			MeshCollector.SetPrimitiveSceneProxy(PrimitiveSceneProxy);

			PrimitiveSceneProxy->GetDynamicMeshElements(Views, *RenderView.Family, -1, MeshCollector);
			const int32 BatchNum = MeshCollector.GetMeshBatchNum();
			if (BatchNum <= 0)
			{
				continue;
			}

			for (int32 BatchIndex = 0; BatchIndex < BatchNum; ++BatchIndex)
			{
				FMeshBatch* MeshBatch = MeshCollector.GetMeshBatch(BatchIndex);
				if (MeshBatch == nullptr)
				{
					continue;
				}

				if (UseGPUScene(GMaxRHIShaderPlatform, RenderView.GetFeatureLevel()))
				{
					if (MeshBatch->VertexFactory->GetPrimitiveIdStreamIndex(EVertexInputStreamType::Default) >= 0)
					{
						for (int32 ElementIndex = 0, ElementCount = MeshBatch->Elements.Num(); ElementIndex < ElementCount; ++ElementIndex)
						{
							FMeshBatchElement& MeshElement = MeshBatch->Elements[ElementIndex];

							if (ElementIndex == 0 && !MeshElement.PrimitiveUniformBufferResource && UISceneProxy->GetGraphicType() != EUIGraphicType::FX)
							{
								if (UISceneProxy->bUseVirtualWorldTransform)
								{
									MeshElement.PrimitiveUniformBufferResource = SceneProxyPrimitiveBuffer;
								}
								else
								{
									MeshElement.PrimitiveUniformBufferResource = UISceneProxy->GetPrimitiveUniformBufferResource(MeshCollector);
								}
							}
							
							if (MeshElement.PrimitiveUniformBufferResource)
							{
								// Set the LightmapID to 0, since that's where our light map data resides for this primitive
								FPrimitiveUniformShaderParameters PrimitiveParams = *(const FPrimitiveUniformShaderParameters*)MeshElement.PrimitiveUniformBufferResource->GetContents();
								PrimitiveParams.LightmapDataIndex = 0;

								// Now we just need to fill out the first entry of primitive data in a buffer and bind it
								FSinglePrimitiveStructured& SinglePrimitiveStructured = GSinglePrimitiveBuffer;
								SinglePrimitiveStructured.PrimitiveSceneData = FPrimitiveSceneShaderData(PrimitiveParams);
								SinglePrimitiveStructured.ShaderPlatform = RenderView.GetShaderPlatform();

								SinglePrimitiveStructured.UploadToGPU();

								if (!GPUSceneUseTexture2D(RenderView.GetShaderPlatform()))
								{
									RenderView.PrimitiveSceneDataOverrideSRV = SinglePrimitiveStructured.PrimitiveSceneDataBufferSRV;
								}
								else
								{
									RenderView.PrimitiveSceneDataTextureOverrideRHI = SinglePrimitiveStructured.PrimitiveSceneDataTextureRHI;
								}

								RenderView.InitRHIResources();
								break;
							}
						}
					}
				}
				
				if (UISceneProxy->bUseVirtualWorldTransform && UISceneProxy->GetGraphicType() != EUIGraphicType::FX)
				{
					for (int32 ElementIndex = 0, ElementCount = MeshBatch->Elements.Num(); ElementIndex < ElementCount; ++ElementIndex)
					{
						FMeshBatchElement& MeshElement = MeshBatch->Elements[ElementIndex];
						MeshElement.PrimitiveUniformBufferResource = SceneProxyPrimitiveBuffer;
					}
				}
				
				for (int32 ElementIndex = MeshBatch->Elements.Num() - 1; ElementIndex >= 0; --ElementIndex)
				{
					FMeshBatchElement& BatchElement = MeshBatch->Elements[ElementIndex];
					BatchElement.PrimitiveIdMode = EPrimitiveIdMode::PrimID_ForceZero;

					if (BatchElement.NumPrimitives <= 0)
					{
						MeshBatch->Elements.RemoveAt(ElementIndex, 1, false);
					}
					else if (BatchElement.PrimitiveUniformBuffer == nullptr && BatchElement.PrimitiveUniformBufferResource == nullptr)
					{
						MeshBatch->Elements.RemoveAt(ElementIndex, 1, false);
					}
				}

				if (UISceneProxy->GetGraphicType() == EUIGraphicType::FX)
				{
					DynamicIndexBuffer.Commit();
					DynamicVertexBuffer.Commit();
					DynamicReadBuffer.Commit();
				}

				if (!MeshBatch->bUseForMaterial || MeshBatch->Elements.Num() == 0)
				{
					continue;
				}

				if (!RHICmdList.IsInsideRenderPass())
				{
					RHICmdList.Transition(FRHITransitionInfo(BackBuffer->GetRenderTargetTexture(), ERHIAccess::Unknown, ERHIAccess::SRVGraphics));

					const ERenderTargetActions ColorAction = (bUseExternalRenderTarget && PassCount == 0) ? ERenderTargetActions::Clear_Store : ERenderTargetActions::Load_Store;
					++PassCount;
					FRHIRenderPassInfo RPInfo(BackBuffer->GetRenderTargetTexture(), ColorAction);

					if (!bUpdateDepthStencil)
					{
						bUpdateDepthStencil = true;
						IntermediateDepthStencil->Update(BufferSize);
					}

					//RHICmdList.Transition(FRHITransitionInfo(DepthStencil, ERHIAccess::Unknown, ERHIAccess::DSVWrite));
					RPInfo.DepthStencilRenderTarget.Action = EDepthStencilTargetActions::ClearDepthStencil_DontStoreDepthStencil;
					RPInfo.DepthStencilRenderTarget.DepthStencilTarget = IntermediateDepthStencil->GetRenderTarget();
#if !PLATFORM_ANDROID
					RPInfo.DepthStencilRenderTarget.ExclusiveDepthStencil = FExclusiveDepthStencil::DepthWrite_StencilWrite;
#else
					RPInfo.DepthStencilRenderTarget.ExclusiveDepthStencil = FExclusiveDepthStencil::DepthRead_StencilRead;
#endif

					RHICmdList.BeginRenderPass(RPInfo, TEXT("UIShadingRenderPass"));

					RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
					RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0, ViewRect.Max.X, ViewRect.Max.Y, 0.0f);
				}

				++UIBatches;
				UIShadingRenderer.Render(RHICmdList, RenderView, FeatureLevel, *MeshBatch, DisplayGamma, bSwitchVerticalAxis ? -1 : 1);
			}	
		}		
	}

	if (RenderView.SimpleElementCollector.HasPrimitives(SDPG_World) ||
		RenderView.SimpleElementCollector.HasPrimitives(SDPG_Foreground))
	{
		if (!RHICmdList.IsInsideRenderPass())
		{
			const ERenderTargetActions ColorAction = (bUseExternalRenderTarget && PassCount == 0) ? ERenderTargetActions::Clear_Store : ERenderTargetActions::Load_Store;
			++PassCount;
			FRHIRenderPassInfo RPInfo(BackBuffer->GetRenderTargetTexture(), ColorAction);

			if (!bUpdateDepthStencil)
			{
				bUpdateDepthStencil = true;
				IntermediateDepthStencil->Update(BufferSize);
			}

			//RHICmdList.Transition(FRHITransitionInfo(DepthStencil, ERHIAccess::Unknown, ERHIAccess::DSVWrite));
			RPInfo.DepthStencilRenderTarget.Action = EDepthStencilTargetActions::ClearDepthStencil_DontStoreDepthStencil;
			RPInfo.DepthStencilRenderTarget.DepthStencilTarget = IntermediateDepthStencil->GetRenderTarget();
#if !PLATFORM_ANDROID
			RPInfo.DepthStencilRenderTarget.ExclusiveDepthStencil = FExclusiveDepthStencil::DepthWrite_StencilWrite;
#else
			RPInfo.DepthStencilRenderTarget.ExclusiveDepthStencil = FExclusiveDepthStencil::DepthRead_StencilRead;
#endif

			RHICmdList.BeginRenderPass(RPInfo, TEXT("UIShadingRenderPass_WorldForeground"));

			RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0, ViewRect.Max.X, ViewRect.Max.Y, 0.0f);
		}
		
		FMeshPassProcessorRenderState DrawRenderState(RenderView, RenderView.ViewUniformBuffer);
		DrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI());
		DrawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI());

		RenderView.SimpleElementCollector.DrawBatchedElements(RHICmdList, DrawRenderState, RenderView, EBlendModeFilter::All, SDPG_World);
		RenderView.SimpleElementCollector.DrawBatchedElements(RHICmdList, DrawRenderState, RenderView, EBlendModeFilter::All, SDPG_Foreground);
	}
	
	if (RHICmdList.IsInsideRenderPass())
	{
		RHICmdList.EndRenderPass();
	}

	if (bUseExternalRenderTarget && RenderThread_RenderTimes > 0)
	{
		--RenderThread_RenderTimes;
	}
	
	RenderTargetResource = nullptr;
}

void FUISceneViewExtension::UpdateRasterizerState(const FMaterial& InMaterialResource, FGraphicsPipelineStateInitializer& InGraphicsPSOInit)
{
	const ERasterizerCullMode CullMode = InMaterialResource.IsTwoSided() ? CM_None : CM_CCW;
	const ERasterizerFillMode FillMode = InMaterialResource.IsWireframe() ? FM_Wireframe : FM_Solid;

	switch(FillMode)
	{
	default:
	case FM_Solid:
		switch(CullMode)
		{
		default:
		case CM_CW:   InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid,CM_CW  ,false,false>::GetRHI(); break;
		case CM_CCW:  InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid,CM_CCW ,false,false>::GetRHI(); break;
		case CM_None: InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid,CM_None,false,false>::GetRHI(); break;
		};
		break;
	case FM_Wireframe:
		switch(CullMode)
		{
		default:
		case CM_CW:   InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe,CM_CW  ,false,false>::GetRHI(); break;
		case CM_CCW:  InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe,CM_CCW ,false,false>::GetRHI(); break;
		case CM_None: InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe,CM_None,false,false>::GetRHI(); break;
		};
		break;
	case FM_Point:
		switch(CullMode)
		{
		default:
		case CM_CW:   InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Point,CM_CW  ,false,false>::GetRHI(); break;
		case CM_CCW:  InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Point,CM_CCW ,false,false>::GetRHI(); break;
		case CM_None: InGraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Point,CM_None,false,false>::GetRHI(); break;
		};
		break;
	}
}

#if ENGINE_MINOR_VERSION <= 26 && ENGINE_MAJOR_VERSION <= 4
bool FUISceneViewExtension::IsActiveThisFrameInContext(FSceneViewExtensionContext& Context) const
#else
bool FUISceneViewExtension::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
#endif
{
	if (Context.Viewport == nullptr)
		return true;

	if (RootCanvas.IsValid())
	{
		const auto World = RootCanvas->GetWorld();
		if (IsValid(World))
		{
#if WITH_EDITOR
			if (World->WorldType == EWorldType::PIE && !Context.Viewport->IsPlayInEditorViewport() && RenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
			{
				return false;
			}
#endif
			const FViewportClient* ViewportClient = Context.Viewport->GetClient();
			if (ViewportClient && ViewportClient->GetWorld() == World)
			{
				return true;
			}
		}
	}
	
	return false;
}
