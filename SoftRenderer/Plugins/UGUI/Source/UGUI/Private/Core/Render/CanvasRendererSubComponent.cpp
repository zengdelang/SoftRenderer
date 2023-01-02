#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/BehaviourComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Renderer/UIMeshProxyComponent.h"
#include "Core/Render/VertexHelper.h"
#include "Core/Widgets/GraphicElementInterface.h"

/////////////////////////////////////////////////////
// UCanvasRendererSubComponent

UCanvasRendererSubComponent::UCanvasRendererSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ParentCanvas = nullptr;
	Material = nullptr;
	Texture = nullptr;
	
	AbsoluteDepth = 0;

	GraphicType = EUIGraphicType::UIMesh;

	Color = FLinearColor::White;
	InheritedAlpha = 1;

	bRefreshRenderProxy = false;
	bRectClipping = false;
	bShouldCull = false;
	bAntiAliasing = false;
	bTextElement = false;
	bHidePrimitive = false;

	bUseCustomRenderProxy = false;

	bUpdateUV1 = false;
	bUpdateInheritedAlpha = false;
}

#if WITH_EDITORONLY_DATA

void UCanvasRendererSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCanvasRendererSubComponent, bHidePrimitive))
	{
		DirtyParentCanvasData();
	}
}

#endif

void UCanvasRendererSubComponent::OnDynamicAdd()
{
	Super::OnDynamicAdd();

	UBehaviourComponent* OuterBehaviourComponent = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(OuterBehaviourComponent))
	{
		const TScriptInterface<IGraphicElementInterface> Graphic = OuterBehaviourComponent->GetComponentByInterface(UGraphicElementInterface::StaticClass(), true);
		if (Graphic)
		{
			Graphic->GetCanvasRenderer();
			Graphic->SetAllDirty();
		}
	}
}

void UCanvasRendererSubComponent::Awake()
{
	Super::Awake();

	SetParentCanvas(GetOwnerCanvas());
}

void UCanvasRendererSubComponent::OnDestroy()
{
	// Clear any dirty reference to renderer.
	SetParentCanvas(nullptr);
 
	// Must be done after clear.
	Clear();
	
	Super::OnDestroy();
}

void UCanvasRendererSubComponent::OnZOrderChanged()
{
	Super::OnZOrderChanged();

	if (ParentCanvas)
	{
		ParentCanvas->GetCanvasData().RemoveRenderer(this);
		ParentCanvas->GetCanvasData().AddRenderer(this);
	}
}

void UCanvasRendererSubComponent::OnCanvasHierarchyChanged()
{
	Super::OnCanvasHierarchyChanged();

	if (!HasBeenAwaken())
		return;
	
	SetParentCanvas(GetOwnerCanvas());
}

void UCanvasRendererSubComponent::OnTransformParentChanged()
{
	Super::OnTransformParentChanged();

	UCanvasSubComponent* OldParentCanvas = ParentCanvas;
	SetParentCanvas(GetOwnerCanvas());
	
	if (OldParentCanvas == ParentCanvas)
	{
		if (OldParentCanvas)
		{
			OldParentCanvas->GetCanvasData().RemoveRenderer(this);
		}

		if (ParentCanvas)
		{
			ParentCanvas->GetCanvasData().AddRenderer(this);
		}
	}
}

void UCanvasRendererSubComponent::OnTransformChanged()
{
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetParentCanvas(UCanvasSubComponent* NewParentCanvas)
{
	if (ParentCanvas != NewParentCanvas)
	{
		if (ParentCanvas)
		{
			ParentCanvas->GetCanvasData().RemoveRenderer(this);
		}
		
		ParentCanvas = NewParentCanvas;

		if (ParentCanvas)
		{
			ParentCanvas->GetCanvasData().AddRenderer(this);
		}
	}
}

void UCanvasRendererSubComponent::DirtyParentCanvasData()
{
	bRefreshRenderProxy = true;

	if (IsValid(ParentCanvas))
	{
		ParentCanvas->GetCanvasData().DirtyFlag |= ECanvasDirtyFlag::CDF_BatchDirty;
	}
}

bool UCanvasRendererSubComponent::CanRender() const
{
	return IsActiveAndEnabled() && !bShouldCull && !bHidePrimitive && Color.A > 1.e-4f &&
		InheritedAlpha > 1.e-4f && (bUseCustomRenderProxy || (Mesh.IsValid() && Mesh->GetCurrentVerticesCount() > 0));
}

void UCanvasRendererSubComponent::FillMesh(FVertexHelper& VertexHelper)
{
	if (!Mesh.IsValid())
	{
		Mesh = MakeShareable(new FVertexHelper());
	}
	Mesh->MoveVertexHelper(VertexHelper);
	
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::Clear()
{
	if (Mesh.IsValid())
	{
		Mesh->Empty();
	}

	SetMaterial(nullptr);
	SetTexture(nullptr);

	bUpdateUV1 = false;
	bUpdateInheritedAlpha = false;

	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetShouldCull(bool bCull)
{
	if (bShouldCull == bCull)
		return;

	bShouldCull = bCull;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetAntiAliasing(bool bInAntiAliasing)
{
	if (bAntiAliasing == bInAntiAliasing)
		return;

	bAntiAliasing = bInAntiAliasing;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetTextElement(bool bInTextElement)
{
	if (bTextElement == bInTextElement)
		return;

	bTextElement = bInTextElement;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetHidePrimitive(bool bInHidePrimitive)
{
	if (bHidePrimitive == bInHidePrimitive)
		return;

	bHidePrimitive = bInHidePrimitive;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::EnableRectClipping(FRect InClipRect, FRect InClipSoftnessRect)
{
	const float XMin = ClipRect.R;
	const float YMin = ClipRect.G;
	const float XMax = ClipRect.B;
	const float YMax = ClipRect.A;

	const float NewXMin = InClipRect.XMin;
	const float NewYMin = InClipRect.YMin;
	const float NewXMax = InClipRect.GetXMax();
	const float NewYMax = InClipRect.GetYMax();

	const float SoftnessXMin = ClipSoftnessRect.R;
	const float SoftnessYMin = ClipSoftnessRect.G;
	const float SoftnessXMax = ClipSoftnessRect.B;
	const float SoftnessYMax = ClipSoftnessRect.A;

	const float SoftnessNewXMin = InClipSoftnessRect.XMin;
	const float SoftnessNewYMin = InClipSoftnessRect.YMin;
	const float SoftnessNewXMax = InClipSoftnessRect.GetXMax();
	const float SoftnessNewYMax = InClipSoftnessRect.GetYMax();
	
	if (bRectClipping)
	{
		if (XMin == NewXMin && YMin == NewYMin &&
			XMax == NewXMax && YMax == NewYMax &&
			SoftnessXMin == SoftnessNewXMin && SoftnessYMin == SoftnessNewYMin &&
			SoftnessXMax == SoftnessNewXMax && SoftnessYMax == SoftnessNewYMax)
		{
			return;
		}
	}

	bRectClipping = true;
	
	ClipRect.R = NewXMin;
	ClipRect.G = NewYMin;
	ClipRect.B = NewXMax;
	ClipRect.A = NewYMax;

	ClipSoftnessRect.R = FMath::Clamp(SoftnessNewXMin, NewXMin, NewXMax);
	ClipSoftnessRect.G = FMath::Clamp(SoftnessNewYMin, NewYMin, NewYMax);
	ClipSoftnessRect.B = FMath::Clamp(SoftnessNewXMax, NewXMin, NewXMax);
	ClipSoftnessRect.A = FMath::Clamp(SoftnessNewYMax, NewYMin, NewYMax);
	
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::DisableRectClipping()
{
	if (!bRectClipping)
		return;

	bRectClipping = false;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetColor(FLinearColor InColor)
{
	if (Color.Equals(InColor))
		return;

	Color = InColor;

	if (OwnerRenderProxyInfo.IsValid())
	{
		if (UUIMeshProxyComponent* UIMeshProxyComp = OwnerRenderProxyInfo.Pin()->OwnerRenderProxy.Get())
		{
			bUpdateInheritedAlpha = true;
			UIMeshProxyComp->DirtyRenderers.Add(this);
				
			if (IsValid(ParentCanvas))
			{
				ParentCanvas->GetCanvasData().DirtyRenderProxies.Add(UIMeshProxyComp);
			}
		}
	}
}

void UCanvasRendererSubComponent::SetAlpha(float InAlpha)
{
	if (FMath::IsNearlyEqual(Color.A, InAlpha))
		return;

	Color.A = InAlpha;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetMaterial(UMaterialInterface* InMaterial)
{
	if (Material != InMaterial)
	{
		Material = InMaterial;
		DirtyParentCanvasData();
	}
}

void UCanvasRendererSubComponent::SetTexture(UTexture* InTexture)
{
	if (Texture != InTexture)
	{
		Texture = InTexture;
		DirtyParentCanvasData();
	}
}

void UCanvasRendererSubComponent::SetGraphicType(EUIGraphicType InGraphicType)
{
	if (GraphicType != InGraphicType)
	{
		GraphicType = InGraphicType;
		DirtyParentCanvasData();
	}
}

void UCanvasRendererSubComponent::SetGraphicData(TWeakPtr<FUIGraphicData> InGraphicData)
{
	GraphicData = InGraphicData;
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::SetUseCustomRenderProxy(bool bInUseCustomRenderProxy)
{
	if (bUseCustomRenderProxy != bInUseCustomRenderProxy)
	{
		bUseCustomRenderProxy = bInUseCustomRenderProxy;
		DirtyParentCanvasData();
	}
}

void UCanvasRendererSubComponent::SetCustomRenderProxyComponent(IUIRenderProxyInterface* InRenderProxy)
{
	CustomRenderProxyComponent = Cast<USceneComponent>(InRenderProxy);
	DirtyParentCanvasData();
}

void UCanvasRendererSubComponent::UpdateMeshUV1(FVector2D InUV1)
{
	if (Mesh.IsValid() && Mesh->GetCurrentVerticesCount() > 0)
	{
		Mesh->UpdateAllUV1(InUV1);

		if (OwnerRenderProxyInfo.IsValid())
		{
			if (UUIMeshProxyComponent* UIMeshProxyComp = OwnerRenderProxyInfo.Pin()->OwnerRenderProxy.Get())
			{
				bUpdateUV1 = true;
				UIMeshProxyComp->DirtyRenderers.Add(this);
				
				if (IsValid(ParentCanvas))
				{
					ParentCanvas->GetCanvasData().DirtyRenderProxies.Add(UIMeshProxyComp);
				}
			}
		}
	}
}

void UCanvasRendererSubComponent::SetInheritedAlpha(float InInheritedAlpha)
{
	if (FMath::IsNearlyEqual(InheritedAlpha, InInheritedAlpha))
		return;

	InheritedAlpha = InInheritedAlpha;
	
	if (OwnerRenderProxyInfo.IsValid())
	{
		if (UUIMeshProxyComponent* UIMeshProxyComp = OwnerRenderProxyInfo.Pin()->OwnerRenderProxy.Get())
		{
			bUpdateInheritedAlpha = true;
			UIMeshProxyComp->DirtyRenderers.Add(this);
				
			if (IsValid(ParentCanvas))
			{
				ParentCanvas->GetCanvasData().DirtyRenderProxies.Add(UIMeshProxyComp);
			}
		}
	}
}

/////////////////////////////////////////////////////
