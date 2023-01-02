#include "Core/Render/CanvasSubComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasManager.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/Renderer/UISceneViewExtension.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Core/Render/UIMeshStorage.h"
#include "UGUISubsystem.h"
#include "UGUIWorldSubsystem.h"
#include "Core/WidgetActor.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"

#ifndef USE_CAMERA_CULLING_MASK
#define USE_CAMERA_CULLING_MASK 0
#endif

/////////////////////////////////////////////////////
// UCanvasSubComponent

FOnWillRenderCanvases UCanvasSubComponent::OnWillRenderCanvases;

UCanvasSubComponent::UCanvasSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay;
	ProjectionType = ECameraProjectionMode::Type::Perspective;

	RenderTargetMode = ECanvasRenderTargetMode::BackBuffer;
	RenderTarget = nullptr;
	RenderTimes = -1;
	
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> DefaultMaterial_Finder(TEXT("/UGUI/DefaultResources/Materials/UGUI_Standard_Inst.UGUI_Standard_Inst"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> DefaultAAMaterial_Finder(TEXT("/UGUI/DefaultResources/Materials/UGUI_SDF_AA_Standard_Inst.UGUI_SDF_AA_Standard_Inst"));
	static ConstructorHelpers::FObjectFinderOptional<UMaterialInterface> DefaultTextMaterial_Finder(TEXT("/UGUI/DefaultResources/Materials/Text/UGUI_Text_SDF_Standard_Inst.UGUI_Text_SDF_Standard_Inst"));
	
	DefaultMaterial = DefaultMaterial_Finder.Get();
	DefaultAAMaterial = DefaultAAMaterial_Finder.Get();
	DefaultTextMaterial = DefaultTextMaterial_Finder.Get();
	
	ParentCanvas = nullptr;

	NestedRenderDepth = -1;
	AbsoluteNestedRenderDepth = 0;
	CanvasInstructionIndex = -1;

	MinMeshPriority = -1;
	MeshPriorityCount = 0;
	
	ScaleFactor = 1;
	ReferencePixelsPerUnit = 1;
	SortingOrder = 0;

	bUpdateNestedRenderDepth = false;
	bHasValidViewportSize = true;
	bOverrideSorting = false;
	bPixelPerfect = false;
	bOverridePixelPerfect = false;
	bClearMeshComponents = false;
	bNeedUpdateRectTransform = false;
	bPerformFrustumCull = true;
	bCheckSceneViewVisibility = true;
	bUpdateRenderTimes = false;

#if WITH_EDITOR
	bForceUseSelfRenderMode = false;
#endif

#if WITH_EDITORONLY_DATA
	CanvasPixelPerfectInheritMode = ECanvasPixelPerfectInheritMode::CanvasPixelPerfectInheritMode_Inherit;
#endif

#if WITH_EDITOR
	UIBatches = 0;
	BlurUIBatches = 0;
	GlitchUIBatches = 0;
#endif

	bUseVirtualWorldTransform = false;
	VirtualWorldTransform = FTransform::Identity;
}

#if WITH_EDITORONLY_DATA

void UCanvasSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	OnRenderModeChanged.Broadcast();

	const FName MemberPropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : FName();
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCanvasSubComponent, SortingOrder))
	{
		SetSortingOrder(SortingOrder);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCanvasSubComponent, RenderMode))
	{
		const auto NewRenderMode = RenderMode;
		RenderMode = ECanvasRenderMode::CanvasRenderMode_MAX;
		SetRenderMode(NewRenderMode);
	}
	else if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UCanvasSubComponent, CanvasPixelPerfectInheritMode))
	{
		if (CanvasPixelPerfectInheritMode == ECanvasPixelPerfectInheritMode::CanvasPixelPerfectInheritMode_Inherit)
		{
			bOverridePixelPerfect = false;
		}
		else if (CanvasPixelPerfectInheritMode == ECanvasPixelPerfectInheritMode::CanvasPixelPerfectInheritMode_On)
		{
			bOverridePixelPerfect = true;
			bPixelPerfect = true;
		}
		else if (CanvasPixelPerfectInheritMode == ECanvasPixelPerfectInheritMode::CanvasPixelPerfectInheritMode_Off)
		{
			bOverridePixelPerfect = true;
			bPixelPerfect = false;
		}
	}
}

#endif

void UCanvasSubComponent::OnDynamicAdd()
{
	Super::OnDynamicAdd();

	AttachTransform = Cast<URectTransformComponent>(GetOuter());

	if (IsValid(AttachTransform))
	{
		AttachTransform->SetOwnerCanvas(this, true, true);
	}
}

void UCanvasSubComponent::Awake()
{
	Super::Awake();
	
	if (IsValid(AttachTransform))
	{
		AttachTransform->SetOwnerCanvas(this, true, false);
	}

	CanvasData.UpdateRootComponent(this, AttachTransform);
	
	AttachTransform = Cast<URectTransformComponent>(GetOuter());

	const auto World = GetWorld();
	if (IsValid(World))
	{
		const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
		if (IsValid(UIWorldSubsystem))
		{
			UIMeshStorage.UIWorldSubsystem = UIWorldSubsystem;
		}
	}

	SyncTransformParent();
}

void UCanvasSubComponent::OnEnable()
{
	Super::OnEnable();

	if (!IsValid(GetParentCanvas()) || bOverrideSorting)
	{
		FCanvasManager::AddCanvas(this);
	}

	// Add the canvas to the proper parent and reassign and children canvas's to this.
	UpdateCanvasRectTransform();

	CanvasData.DirtyFlag |= CDF_BatchDirty;

	if (IsValid(GetParentCanvas()))
	{
		GetParentCanvas()->CanvasData.NotifyCanvasStateChanged(this, true);
	}

	const auto World = GetWorld();
	if (IsValid(World))
	{
		const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
		if (IsValid(UIWorldSubsystem))
		{
			ViewportResizeDelegateHandle = UIWorldSubsystem->ViewportResizedEvent.AddUObject(this, &UCanvasSubComponent::OnViewportResized);
		}
	}

#if WITH_EDITOR
	EditorViewportResizeDelegateHandle = UUGUISubsystem::OnEditorViewportInfoChanged.AddUObject(this, &UCanvasSubComponent::OnEditorViewportResized);
#endif

#if WITH_EDITORONLY_DATA
	const auto BehaviourComp = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(BehaviourComp) && IsValid(BehaviourComp->GetWorld()) && BehaviourComp->GetWorld()->WorldType != EWorldType::Type::EditorPreview)
	{
		const AWidgetActor* OwnerActor = Cast<AWidgetActor>(AttachTransform->GetOwner());
		if (IsValid(OwnerActor) && !OwnerActor->bDisableCanvasUpdateRectTransform)
		{
			BehaviourComp->SetIsRootCanvas(true);
		}
	}
#endif
}

void UCanvasSubComponent::OnDisable()
{
	Super::OnDisable();

	if (ViewportResizeDelegateHandle.IsValid())
	{
		const auto World = GetWorld();
		if (IsValid(World))
		{
			const auto UIWorldSubsystem = World->GetSubsystem<UUGUIWorldSubsystem>();
			if (IsValid(UIWorldSubsystem))
			{
				UIWorldSubsystem->ViewportResizedEvent.Remove(ViewportResizeDelegateHandle);
			}
		}

		ViewportResizeDelegateHandle.Reset();
	}

#if WITH_EDITOR
	UUGUISubsystem::OnEditorViewportInfoChanged.Remove(EditorViewportResizeDelegateHandle);
#endif

	if (!IsValid(GetParentCanvas()) || bOverrideSorting)
	{
		FCanvasManager::RemoveCanvas(this);
	}

	if (IsValid(GetParentCanvas()))
	{
		GetParentCanvas()->CanvasData.NotifyCanvasStateChanged(this, false);
	}

#if WITH_EDITORONLY_DATA
	const auto BehaviourComp = Cast<UBehaviourComponent>(GetOuter());
	if (IsValid(BehaviourComp) && IsValid(BehaviourComp->GetWorld()) && BehaviourComp->GetWorld()->WorldType != EWorldType::Type::EditorPreview)
	{
		BehaviourComp->SetIsRootCanvas(false);
	}
#endif

	if (ViewExtension.IsValid())
	{
		ViewExtension->FlushGeneratedResources();
		ViewExtension.Reset();
	}

	CacheAllUIMesh(true);
}

void UCanvasSubComponent::OnDynamicRemove()
{
	Super::OnDynamicRemove();

	if (IsValid(AttachTransform))
	{
		UCanvasSubComponent* NewOwnerCanvas = nullptr;
		if (const URectTransformComponent* ParentTransform = Cast<URectTransformComponent>(AttachTransform->GetAttachParent()))
		{
			NewOwnerCanvas = ParentTransform->GetOwnerCanvas();
		}
		
		AttachTransform->SetOwnerCanvas(NewOwnerCanvas, true, true);
	}
}

void UCanvasSubComponent::OnDestroy()
{
	CanvasData.Empty();
	
	if (IsValid(GetParentCanvas()) && !bOverrideSorting)
	{
		GetParentCanvas()->CanvasData.RemoveCanvas(this);
	}
	
	Super::OnDestroy();
}

void UCanvasSubComponent::OnZOrderChanged()
{
	Super::OnZOrderChanged();

	if (!bOverrideSorting)
	{
		if (ParentCanvas)
		{
			const auto OverrideCanvas = GetOrderOverrideCanvas();
			if (IsValid(OverrideCanvas))
			{
				OverrideCanvas->CanvasData.DirtyFlag |= CDF_MeshPriorityDirty;
			}
			
			ParentCanvas->GetCanvasData().RemoveCanvas(this);
			ParentCanvas->GetCanvasData().AddCanvas(this);
		}
	}
}

void UCanvasSubComponent::OnTransformParentChanged()
{
	Super::OnTransformParentChanged();
	
	UCanvasSubComponent* OldParentCanvas = ParentCanvas;
	SyncTransformParent();

	if (OldParentCanvas == ParentCanvas)
	{
		if (OldParentCanvas)
		{
			OldParentCanvas->CanvasData.RemoveCanvas(this);
		}

		if (ParentCanvas)
		{
			ParentCanvas->CanvasData.AddCanvas(this);
		}
	}
}

void UCanvasSubComponent::OnCanvasHierarchyChanged()
{
	Super::OnCanvasHierarchyChanged();
	
	SyncTransformParent();
}

void UCanvasSubComponent::OnRemoveFromCanvasManager()
{
	if (ViewExtension.IsValid())
	{
		ViewExtension->FlushGeneratedResources();
		ViewExtension.Reset();
	}

	bClearMeshComponents = true;
}

void UCanvasSubComponent::OnAddToCanvasManager()
{
	bNeedUpdateRectTransform = true;

	MarkNestedCanvasBatchDirty();
	
	bUpdateNestedRenderDepth = false;
	NestedRenderDepth = -1;
	AbsoluteNestedRenderDepth = 0;
}

void UCanvasSubComponent::MarkNestedCanvasBatchDirty()
{
	CanvasData.DirtyFlag |= CDF_BatchDirty;

	for (const auto& ChildCanvas : CanvasData.NestedCanvases)
	{
		if (IsValid(ChildCanvas) && ChildCanvas->IsActiveAndEnabled())
		{
			ChildCanvas->MarkNestedCanvasBatchDirty();
		}
	}
}

void UCanvasSubComponent::OnSortInCanvasManager(int32 Index)
{
	if (ViewExtension.IsValid())
	{
		ViewExtension->SetPriority(-Index);
	}
}

void UCanvasSubComponent::OnViewportResized()
{
	UpdateCanvasRectTransform();	
}

#if WITH_EDITOR

void UCanvasSubComponent::OnEditorViewportResized(const UWorld* InWorld)
{
	if (InWorld == GetWorld())
	{
		UpdateCanvasRectTransform();
	}
}

#endif

void UCanvasSubComponent::UpdateCanvasRectTransform()
{
#if WITH_EDITOR
	ECanvasRenderMode FinalRenderMode = RenderMode;
#else
	const ECanvasRenderMode FinalRenderMode = RenderMode;
#endif
	
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		const FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(GetWorld());
		if (WorldViewportInfo)
		{
			FinalRenderMode = WorldViewportInfo->GetRenderMode();
		}
		else
		{
			FinalRenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
		}
	}
#endif
	
	if (FinalRenderMode == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay && 
		!IsValid(GetParentCanvas()) && IsValid(AttachTransform))
	{
		const AWidgetActor* OwnerActor = Cast<AWidgetActor>(AttachTransform->GetOwner());
		if (IsValid(OwnerActor) && OwnerActor->bDisableCanvasUpdateRectTransform)
		{
			return;
		}
		
		// Set scale of canvas
		AttachTransform->SetLocalScale(FVector(ScaleFactor, ScaleFactor, ScaleFactor));
		
		// Calculate screen size
		// Set size of canvas
		const FVector2D ScreenSizeScaled = UUGUISubsystem::GetViewportSize(this) / FMath::Max(0.0001f, ScaleFactor);

		bHasValidViewportSize = !ScreenSizeScaled.IsNearlyZero();
		
		if (AttachTransform->GetSizeDelta() != ScreenSizeScaled)
		{
			AttachTransform->SetAnchorAndSizeAndPivot(FVector2D::ZeroVector, FVector2D::ZeroVector, ScreenSizeScaled, FVector2D(0.5f, 0.5f));
		}
		else
		{
			AttachTransform->SetAnchorAndPivot(FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D(0.5f, 0.5f));
		}
	}
	
	for (const auto& ChildCanvas : CanvasData.NestedCanvases)
	{
		if (IsValid(ChildCanvas) && ChildCanvas->IsActiveAndEnabled())
		{
			ChildCanvas->UpdateCanvasRectTransform();
		}
	}
}

TSharedPtr<FUISceneViewExtension, ESPMode::ThreadSafe>& UCanvasSubComponent::GetViewExtension()
{
#if WITH_EDITOR
	ECanvasRenderMode TargetRenderMode = RenderMode;
	if (IsValid(GetWorld()))
	{
		if (GetWorld()->WorldType == EWorldType::Type::EditorPreview && TargetRenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			TargetRenderMode = ECanvasRenderMode::CanvasRenderMode_ScreenSpaceFree;
		}
	}
#else
	const ECanvasRenderMode TargetRenderMode = RenderMode;
#endif
	
	if ((IsRootCanvas() || IsOverrideSorting()) && TargetRenderMode != ECanvasRenderMode::CanvasRenderMode_WorldSpace && !ViewExtension.IsValid())
	{
		const auto World = GetWorld();
		if (IsValid(World) && IsValid(GEngine))
		{
			ViewExtension = FSceneViewExtensions::NewExtension<FUISceneViewExtension>(this);
			ViewExtension->SetRenderMode(RenderMode);
		}
	}
	return ViewExtension;
}

bool UCanvasSubComponent::CalculateCanvasMatrices(FVector& OutViewLocation, FMatrix& OutViewRotationMatrix,
	FMatrix& OutProjectionMatrix, FMatrix& OutViewProjectionMatrix, FMatrix& OutLocalToWorldMatrix)
{
	return CalculateCanvasMatrices(OutViewLocation, OutViewRotationMatrix, OutProjectionMatrix, OutViewProjectionMatrix, ProjectionType, OutLocalToWorldMatrix);
}

bool UCanvasSubComponent::CalculateCanvasMatrices(FVector& OutViewLocation, FMatrix& OutViewRotationMatrix,
                                                  FMatrix& OutProjectionMatrix, FMatrix& OutViewProjectionMatrix, ECameraProjectionMode::Type InProjectionMode, FMatrix& OutLocalToWorldMatrix)
{
	check(IsInGameThread());
	
	if (!IsValid(AttachTransform))
	{
		return false;
	}
	
	const auto& Rect = AttachTransform->GetRect();
	
	float DistanceOffset;
	if (InProjectionMode == ECameraProjectionMode::Orthographic)
	{
		DistanceOffset = 1000;
	}
	else
	{
		DistanceOffset = Rect.Width * 0.5f / FMath::Tan(FMath::DegreesToRadians(90 * 0.5f));
	}

	CalculateProjectionMatrix(OutProjectionMatrix, InProjectionMode);
	OutProjectionMatrix = AdjustProjectionMatrixForRHI(OutProjectionMatrix);
	
	const FTransform CameraRelativeTransform(RelativeRotationCache.RotatorToQuat(FRotator(90, -90, 0)), FVector(0, 0, -DistanceOffset), FVector::OneVector);
	const FTransform LocalToWorldTransform = AttachTransform->GetComponentToWorld();
	const FTransform CameraWorldTransform = CameraRelativeTransform * LocalToWorldTransform;
	OutLocalToWorldMatrix = LocalToWorldTransform.ToMatrixWithScale();

	OutViewLocation = CameraWorldTransform.GetLocation();
	OutViewRotationMatrix = FInverseRotationMatrix(CameraWorldTransform.Rotator()) * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));
	
	OutViewProjectionMatrix = FTranslationMatrix(-OutViewLocation) * OutViewRotationMatrix * OutProjectionMatrix;
	return true;
}

void UCanvasSubComponent::CalculateProjectionMatrix(FMatrix& OutProjectionMatrix, ECameraProjectionMode::Type InProjectionMode)
{
	if (InProjectionMode == ECameraProjectionMode::Orthographic)
	{
		const auto& LocalScale = AttachTransform->GetRelativeScale3D();
		const auto& Rect = AttachTransform->GetRect();
 		const float HalfOrthoWidth = Rect.Width * 0.5f * LocalScale.X;
		const float ScaledOrthoHeight = Rect.Height * 0.5f * LocalScale.Y;

		constexpr float NearPlane = 0.0f;
		constexpr float FarPlane = WORLD_MAX;

		constexpr float ZScale = 1.0f / (FarPlane - NearPlane);
		constexpr float ZOffset = -NearPlane;

		OutProjectionMatrix = FReversedZOrthoMatrix(
			HalfOrthoWidth,
			ScaledOrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		const auto& ViewportSize = UUGUISubsystem::GetViewportSize(this);

		constexpr float XAxisMultiplier = 1.0f;
		const float YAxisMultiplier = ViewportSize.X / static_cast<float>(FMath::Max(ViewportSize.Y, 1.f));

		constexpr float FOV = FMath::Max(0.001f, 90.0f) * static_cast<float>(PI) / 360.0f;
		
		OutProjectionMatrix = FReversedZPerspectiveMatrix(
			FOV,
			FOV,
			XAxisMultiplier,
			YAxisMultiplier,
			GNearClippingPlane,
			GNearClippingPlane
		);
	}
}

void UCanvasSubComponent::AddNestedCanvas(UCanvasSubComponent* ChildCanvas)
{
	if (IsValid(ChildCanvas))
	{
		ChildCanvas->bHasValidViewportSize = true;

		CanvasData.AddCanvas(ChildCanvas);

		bClearMeshComponents = true;
	}
}

void UCanvasSubComponent::RemoveNestedCanvas(UCanvasSubComponent* ChildCanvas)
{
	CanvasData.RemoveCanvas(ChildCanvas);
	bClearMeshComponents = true;
}

void UCanvasSubComponent::DispatchOnRectTransformDimensionsChange(USceneComponent* SceneComp) const
{
	if (!IsValid(SceneComp))
		return;

	const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
	if (IsValid(BehaviourComp))
	{
		BehaviourComp->OnRectTransformDimensionsChange();

		const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
		for (const auto& SubComponent : AllSubComponents)
		{
			if (IsValid(SubComponent))
			{
				SubComponent->OnRectTransformDimensionsChange();
			}
		}

		bool bNeedLoop = true;
		while(bNeedLoop)
		{
			bNeedLoop = false;
			BehaviourComp->ResetAttachChildChanged();

			const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
			for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
			{
				DispatchOnRectTransformDimensionsChange(BehaviourChildren[Index]);
				
				if (BehaviourComp->IsAttachChildChanged())
				{
					bNeedLoop = true;
					break;
				}
			}
		}
	}
	else
	{
		for (const auto& ChildComp : SceneComp->GetAttachChildren())
		{
			DispatchOnRectTransformDimensionsChange(ChildComp);
		}
	}
}

void UCanvasSubComponent::UpdateCanvasOverrideSorting()
{
	if (IsValid(GetParentCanvas()))
	{
		if (bOverrideSorting)
		{
			GetParentCanvas()->RemoveNestedCanvas(this);
			FCanvasManager::AddCanvas(this);
		}
		else
		{
			GetParentCanvas()->AddNestedCanvas(this);
			FCanvasManager::RemoveCanvas(this);
		}

		if (IsValid(AttachTransform))
		{
			AttachTransform->SetOwnerCanvas(AttachTransform->GetOwnerCanvas(), false, true);
		}
	}
}

void UCanvasSubComponent::SyncTransformParent()
{
	if (!IsValid(AttachTransform))
		return;
	
	UCanvasSubComponent* NewParentCanvas = nullptr;
	if (const URectTransformComponent* SceneParent = Cast<URectTransformComponent>(AttachTransform->GetAttachParent()))
	{
		NewParentCanvas = SceneParent->GetOwnerCanvas();	
	}
	
	// No need to check for dirty parent hierarchy here as this is updating the parent.
	if (ParentCanvas != NewParentCanvas)
	{
		if (ParentCanvas && !bOverrideSorting)
		{
			ParentCanvas->RemoveNestedCanvas(this);
		}
		else
		{
			FCanvasManager::RemoveCanvas(this);
		}

		// Set the parent so we can properly get the renderMode when sorting.
		ParentCanvas = NewParentCanvas;

#if WITH_EDITORONLY_DATA
		const auto BehaviourComp = Cast<UBehaviourComponent>(GetOuter());
		if (IsValid(BehaviourComp) && IsValid(BehaviourComp->GetWorld()) && BehaviourComp->GetWorld()->WorldType != EWorldType::Type::EditorPreview)
		{
			BehaviourComp->SetIsRootCanvas(ParentCanvas == nullptr);
		}
#endif

		if (!NewParentCanvas || bOverrideSorting)
		{
			FCanvasManager::AddCanvas(this);
		}
		else
		{
			NewParentCanvas->AddNestedCanvas(this);
		}

		OnParentCanvasChanged.Broadcast();
	}
}

UBehaviourSubComponent* UCanvasSubComponent::FindSubComponent(USceneComponent* SceneComp, UClass* SubClass)
{
	const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
	if (IsValid(BehaviourComp))
	{
		return Cast<UBehaviourSubComponent>(BehaviourComp->GetComponent(SubClass, true));
	}

	return nullptr;
}

UMaterialInterface* UCanvasSubComponent::GetDefaultMaterial() const
{
#if WITH_EDITORONLY_DATA
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Type::EditorPreview)
	{
		if (EditorPreviewDefaultMaterial && GUObjectArray.ObjectToIndex(EditorPreviewDefaultMaterial) >= 0)
		{
			return EditorPreviewDefaultMaterial;
		}
	}
#endif
	if (DefaultMaterial && GUObjectArray.ObjectToIndex(DefaultMaterial) >= 0)
		return DefaultMaterial;
	return nullptr;
}

void UCanvasSubComponent::SetDefaultMaterial(UMaterialInterface* InDefaultMaterial)
{
	if (DefaultMaterial != InDefaultMaterial)
	{
		DefaultMaterial = InDefaultMaterial;
		CanvasData.DirtyFlag |= CDF_BatchDirty;
	}
}

UMaterialInterface* UCanvasSubComponent::GetDefaultAAMaterial() const
{
#if WITH_EDITORONLY_DATA
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Type::EditorPreview)
	{
		if (EditorPreviewDefaultAAMaterial && GUObjectArray.ObjectToIndex(EditorPreviewDefaultAAMaterial) >= 0)
		{
			return EditorPreviewDefaultAAMaterial;
		}
	}
#endif
	if (DefaultAAMaterial && GUObjectArray.ObjectToIndex(DefaultAAMaterial) >= 0)
		return DefaultAAMaterial;
	return nullptr;
}

void UCanvasSubComponent::SetDefaultAAMaterial(UMaterialInterface* InDefaultAAMaterial)
{
	if (DefaultAAMaterial != InDefaultAAMaterial)
	{
		DefaultAAMaterial = InDefaultAAMaterial;
		CanvasData.DirtyFlag |= CDF_BatchDirty;
	}
}

UMaterialInterface* UCanvasSubComponent::GetDefaultTextMaterial() const
{
#if WITH_EDITORONLY_DATA
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Type::EditorPreview)
	{
		if (EditorPreviewDefaultTextMaterial && GUObjectArray.ObjectToIndex(EditorPreviewDefaultTextMaterial) >= 0)
		{
			return EditorPreviewDefaultTextMaterial;
		}
	}
#endif
	if (DefaultTextMaterial && GUObjectArray.ObjectToIndex(DefaultTextMaterial) >= 0)
		return DefaultTextMaterial;
	return nullptr;
}

void UCanvasSubComponent::SetDefaultTextMaterial(UMaterialInterface* InDefaultTextMaterial)
{
	if (DefaultTextMaterial != InDefaultTextMaterial)
	{
		DefaultTextMaterial = InDefaultTextMaterial;
		CanvasData.DirtyFlag |= CDF_BatchDirty;
	}
}

UMaterialInterface* UCanvasSubComponent::GetDefaultMaterialForUIMesh(bool bUseAntiAliasing, bool bTextMaterial) const
{
	if (bTextMaterial)
	{
		return GetDefaultTextMaterial();
	}
	
	if (bUseAntiAliasing)
	{
		return GetDefaultAAMaterial();
	}

	return GetDefaultMaterial();
}

void UCanvasSubComponent::SetRenderMode(ECanvasRenderMode InRenderMode)
{
	if (IsValid(GetParentCanvas()) && !bOverrideSorting)
	{
		GetParentCanvas()->SetRenderMode(InRenderMode);
	}
	else if (RenderMode != InRenderMode)
	{
		RenderMode = InRenderMode;

		UpdateCanvasRectTransform();

		CanvasData.DirtyFlag |= CDF_BatchDirty;
		
		if (RenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			if (ViewExtension.IsValid())
			{
				ViewExtension->FlushGeneratedResources();
				ViewExtension.Reset();
			}
		}
		else
		{
			if (ViewExtension.IsValid())
			{
				ViewExtension->SetRenderMode(RenderMode);
			}
		}

		OnRenderModeChanged.Broadcast();
	}
}

void UCanvasSubComponent::SetScaleFactor(float InScaleFactor)
{
	if (IsValid(GetParentCanvas()))
	{
		GetParentCanvas()->SetScaleFactor(InScaleFactor);
		return;
	}

	InScaleFactor = FMath::Max(InScaleFactor, 0.0001f);
	if (InScaleFactor == ScaleFactor || FMath::IsNaN(InScaleFactor))
		return;

	ScaleFactor = InScaleFactor;
	OnScaleFactorChanged.Broadcast();
	
	UpdateCanvasRectTransform();

	DispatchOnRectTransformDimensionsChange(AttachTransform);
}

void UCanvasSubComponent::SetReferencePixelsPerUnit(float InReferencePixelsPerUnit)
{
	if (IsValid(GetParentCanvas()))
	{
		GetParentCanvas()->SetReferencePixelsPerUnit(InReferencePixelsPerUnit);
		return;
	}

	InReferencePixelsPerUnit = FMath::Max(InReferencePixelsPerUnit, 0.0001f);
	if (InReferencePixelsPerUnit == ReferencePixelsPerUnit)
		return;

	ReferencePixelsPerUnit = InReferencePixelsPerUnit;
	DispatchOnRectTransformDimensionsChange(AttachTransform);
}

void UCanvasSubComponent::SetSortingOrder(int32 InSortingOrder)
{
	// Don't allow setting of sorting Order for non root / overridden sorting
	if (IsValid(GetParentCanvas()) && !bOverrideSorting)
		return;

	SortingOrder = InSortingOrder;

	FCanvasManager::SortCanvasList();
}

int32 UCanvasSubComponent::GetRenderOrder() const
{
	return FCanvasManager::GetRenderOrder(this);
}

void UCanvasSubComponent::SetOverrideSorting(bool bNewOverrideSorting)
{
	if (bNewOverrideSorting != bOverrideSorting)
	{
		bOverrideSorting = bNewOverrideSorting;
		UpdateCanvasOverrideSorting();
	}
}

void UCanvasSubComponent::SetVirtualWorldTransform(FTransform InVirtualWorldTransform)
{
	if (bUseVirtualWorldTransform)
	{
		if (VirtualWorldTransform.Equals(InVirtualWorldTransform))
			return;
	}
	
	VirtualWorldTransform = InVirtualWorldTransform;
	bUseVirtualWorldTransform = true;

	const auto NewVirtualWorldTransform = InVirtualWorldTransform.ToMatrixWithScale();

	for (int32 Index = 0, Count = UIMeshStorage.UIMeshList.Num(); Index < Count; ++Index)
	{
		if (IUIRenderProxyInterface* UIRenderProxy = Cast<IUIRenderProxyInterface>(UIMeshStorage.UIMeshList[Index]))
		{
			UIRenderProxy->UpdateVirtualWorldTransform(GetUniqueID(), NewVirtualWorldTransform, false, AttachTransform);
		}
	}
}

void UCanvasSubComponent::ClearVirtualWorldTransform()
{
	if (bUseVirtualWorldTransform)
	{
		bUseVirtualWorldTransform = false;

		for (int32 Index = 0, Count = UIMeshStorage.UIMeshList.Num(); Index < Count; ++Index)
		{
			if (IUIRenderProxyInterface* UIRenderProxy = Cast<IUIRenderProxyInterface>(UIMeshStorage.UIMeshList[Index]))
			{
				UIRenderProxy->UpdateVirtualWorldTransform(GetUniqueID(), FMatrix::Identity, true, AttachTransform);
			}
		}
	}
}

void UCanvasSubComponent::SetPixelPerfect(bool bInPixelPerfect)
{
	if (bPixelPerfect != bInPixelPerfect)
	{
		bPixelPerfect = bInPixelPerfect;
		
		DispatchOnRectTransformDimensionsChange(AttachTransform);
	}
}

void UCanvasSubComponent::SetOverridePixelPerfect(bool bInOverridePixelPerfect)
{
	if (bOverridePixelPerfect == bInOverridePixelPerfect)
		return;

	bOverridePixelPerfect = bInOverridePixelPerfect;
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateBatches"), STAT_UnrealGUI_UpdateBatches, STATGROUP_UnrealGUI);
int32 UCanvasSubComponent::UpdateBatches(bool bUpdateRectTransform, int32 StartPriority)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateBatches);
	
	if (!bHasValidViewportSize)
	{
		return StartPriority;
	}
	
	if (bClearMeshComponents)
	{
		bClearMeshComponents = false;
		
		if (UIMeshStorage.CanClearMesh(GetOrderOverrideCanvas()))
		{
			ClearAllMeshes();
		}
	}
	
	if (bNeedUpdateRectTransform || bUpdateRectTransform)
	{
		bNeedUpdateRectTransform = false;
		UpdateCanvasRectTransform();
	}
	
	CanvasData.UpdateTree();

	for (const auto& ChildCanvas : CanvasData.NestedCanvases)
	{
		if (IsValid(ChildCanvas) && ChildCanvas->IsActiveAndEnabled())
		{
			ChildCanvas->UpdateBatches(bUpdateRectTransform, StartPriority);
		}
	}
	
	CanvasData.UpdateDepth();
	UpdateUIMeshBatches();

	if (IsRootCanvas() || bOverrideSorting)
	{
		UpdateMeshComponentsPriority(StartPriority);
	}

	return MinMeshPriority + MeshPriorityCount;
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateUIMeshBatches"), STAT_UnrealGUI_UpdateUIMeshBatches, STATGROUP_UnrealGUI);
void UCanvasSubComponent::UpdateUIMeshBatches()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateUIMeshBatches);
	
	CanvasData.UpdateBatchDirty();
	
	if ((CanvasData.DirtyFlag & CDF_BatchDirty) != 0)
	{
		MeshPriorityCount = 0;
		
		const auto OverrideCanvas = GetOrderOverrideCanvas();
		if (!IsValid(OverrideCanvas))
			return;

		const auto RootCanvas = OverrideCanvas->GetRootCanvas();
		if (!IsValid(RootCanvas))
			return;

		const auto OverrideCanvasSceneComp = Cast<USceneComponent>(OverrideCanvas->GetOuter());
		if (!IsValid(OverrideCanvasSceneComp))
			return;

		const auto World = GetWorld();
		if (!IsValid(World))
			return;
		
		const FTransform WorldToCanvasTransform = AttachTransform->GetComponentTransform().Inverse();
		CanvasData.MergeCanvasRenderer(this, OverrideCanvas, RootCanvas, OverrideCanvasSceneComp, AttachTransform, WorldToCanvasTransform);
			
		OverrideCanvas->CanvasData.DirtyFlag |= CDF_MeshPriorityDirty;

		CanvasData.DirtyFlag &= ~CDF_BatchDirty;
	}

	CanvasData.UpdateDirtyRenderers();
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateMeshComponentsPriority"), STAT_UnrealGUI_UpdateMeshComponentsPriority, STATGROUP_UnrealGUI);
void UCanvasSubComponent::UpdateMeshComponentsPriority(int32 StartPriority)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateMeshComponentsPriority);
	
	if (StartPriority > MinMeshPriority || (MinMeshPriority - StartPriority) > 200)
	{
		MinMeshPriority = StartPriority;
		CanvasData.DirtyFlag |= CDF_MeshPriorityDirty;
	}
	
	if ((CanvasData.DirtyFlag & CDF_MeshPriorityDirty) != 0)
	{
		if (StartPriority > MinMeshPriority || (MinMeshPriority - StartPriority) > 200)
		{
			MinMeshPriority = StartPriority;
		}
		
		MeshPriorityCount = 0;
		StartPriority = MinMeshPriority;

		UpdateCanvasMeshComponentsPriority(StartPriority, MeshPriorityCount);
		CanvasData.DirtyFlag &= ~CDF_MeshPriorityDirty;
	}
}

void UCanvasSubComponent::UpdateCanvasMeshComponentsPriority(int32& StartPriority, int32& PriorityCount)
{
	int32 CanvasIndex = UIMeshStorage.UpdateMeshComponentsPriority(this, StartPriority, PriorityCount);

	for (const int32 CanvasCount = CanvasData.NestedCanvases.Num(); CanvasIndex < CanvasCount; ++CanvasIndex)
	{
		const auto& NestedCanvas = CanvasData.NestedCanvases[CanvasIndex];
		if (IsValid(NestedCanvas) && NestedCanvas->IsActiveAndEnabled())
		{
			NestedCanvas->UpdateCanvasMeshComponentsPriority(StartPriority, PriorityCount);
		}
	}
}

void UCanvasSubComponent::CacheAllUIMesh(bool bIncludeChildren)
{
	MeshPriorityCount = 0;

	UIMeshStorage.CacheAllUIMesh();

	if (bIncludeChildren)
	{
		for (const auto& ChildCanvas : CanvasData.NestedCanvases)
		{
			if (IsValid(ChildCanvas) && ChildCanvas->IsActiveAndEnabled())
			{
				ChildCanvas->CacheAllUIMesh(bIncludeChildren);
			}
		}
	}
}

void UCanvasSubComponent::ClearAllMeshes()
{
	UIMeshStorage.CacheAllUIMesh();
}

/////////////////////////////////////////////////////
