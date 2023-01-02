#include "UGUIWorldSubsystem.h"
#include "Core/SpecializedCollections/WeakObjectIndexedSet.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Renderer/UIMeshProxyComponent.h"
#include "Core/Widgets/GraphicElementInterface.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "GameFramework/HUD.h"
#include "UGUISubsystem.h"
#include "Engine/Canvas.h"
#include "UGUI.h"
#include "UISequence/Public/UISequenceModule.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "AudioDevice.h"

/////////////////////////////////////////////////////
// UUGUIWorldSubsystem

UUGUIWorldSubsystem::UUGUIWorldSubsystem()
{
	CacheViewportSize = FVector2D::ZeroVector;

	UIMeshCreationTimes = 0;
	MaterialDynamicInstanceCreationTimes = 0;
	
	UIMeshActor = nullptr;

	bIsDirty = false;
}

void UUGUIWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UUGUISubsystem* Subsystem = GEngine->GetEngineSubsystem<UUGUISubsystem>();
	if (IsValid(Subsystem))
	{
#if WITH_EDITOR
		if (IsValid(GetWorld()) && (GetWorld()->WorldType == EWorldType::Game || GetWorld()->WorldType == EWorldType::PIE))
		{
			Subsystem->OnPreWillRenderCanvases.AddUObject(this, &UUGUIWorldSubsystem::UpdateViewportSize);
		}
#else
		Subsystem->OnPreWillRenderCanvases.AddUObject(this, &UUGUIWorldSubsystem::UpdateViewportSize);
#endif
	}

#if WITH_EDITOR
	if (IsValid(GetWorld()) && (GetWorld()->WorldType == EWorldType::Game || GetWorld()->WorldType == EWorldType::PIE))
	{
		AHUD::OnShowDebugInfo.AddUObject(this, &UUGUIWorldSubsystem::ShowDebugInfo);
	}
#else
	AHUD::OnShowDebugInfo.AddUObject(this, &UUGUIWorldSubsystem::ShowDebugInfo);
#endif
}

void UUGUIWorldSubsystem::Deinitialize()
{
	AHUD::OnShowDebugInfo.RemoveAll(this);
	
	UUGUISubsystem* Subsystem = GEngine->GetEngineSubsystem<UUGUISubsystem>();
	if (IsValid(Subsystem))
	{
		Subsystem->OnPreWillRenderCanvases.RemoveAll(this);
	}

	DynamicMaterialInstances.Empty();
	
	Super::Deinitialize();
}

bool UUGUIWorldSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return FApp::CanEverRender();
}

DECLARE_CYCLE_STAT(TEXT("UIBase --- UpdateViewportSize"), STAT_UnrealGUI_UpdateViewportSize, STATGROUP_UnrealGUI);
void UUGUIWorldSubsystem::UpdateViewportSize()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateViewportSize);
	
	const FVector2D NewViewportSize = UUGUISubsystem::GetViewportSize(this);
	if (!FMath::IsNearlyEqual(NewViewportSize.X, CacheViewportSize.X) || !FMath::IsNearlyEqual(NewViewportSize.Y, CacheViewportSize.Y))
	{
		CacheViewportSize = NewViewportSize;
		ViewportResizedEvent.Broadcast();
	}
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- AddRaycastTarget"), STAT_UnrealGUI_AddRaycastTarget, STATGROUP_UnrealGUI);
void UUGUIWorldSubsystem::AddRaycastTarget(UCanvasSubComponent* InCanvas, UObject* Graphics)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_AddRaycastTarget);
	
	if (!IsValid(InCanvas) || !IsValid(Graphics))
	{
		return;
	}
	
	if (const auto TransformSetPtr = PendingRemoveGraphics.Find(InCanvas))
	{
		TransformSetPtr->Remove(Graphics);
	}

	auto& TransformSet = PendingAddGraphics.FindOrAdd(InCanvas);
	TransformSet.Add(Graphics);

	bIsDirty = true;
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- RemoveRaycastTarget"), STAT_UnrealGUI_RemoveRaycastTarget, STATGROUP_UnrealGUI);
void UUGUIWorldSubsystem::RemoveRaycastTarget(UCanvasSubComponent* InCanvas, UObject* Graphics)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RemoveRaycastTarget);
	
	if (!IsValid(InCanvas) || !IsValid(Graphics))
	{
		return;
	}
	
	if (const auto TransformSetPtr = PendingAddGraphics.Find(InCanvas))
	{
		TransformSetPtr->Remove(Graphics);
	}

	auto& TransformSet = PendingRemoveGraphics.FindOrAdd(InCanvas);
	TransformSet.Add(Graphics);

	bIsDirty = true;
}

struct FWorldRayInfo
{
	FVector RayOrigin;
	FVector RayDirection;

	FWorldRayInfo()
	{
		
	}

	FWorldRayInfo(const FVector& InRayOrigin, const FVector& InRayDirection) : RayOrigin(InRayOrigin), RayDirection(InRayDirection)
	{
	}
};

static TMap<UCanvasSubComponent*, FWorldRayInfo> CacheWorldRays;

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- RaycastAll"), STAT_UnrealGUI_RaycastAll, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- OctreeFindElements"), STAT_UnrealGUI_OctreeFindElements, STATGROUP_UnrealGUI);
DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- TestRectTransform"), STAT_UnrealGUI_TestRectTransform, STATGROUP_UnrealGUI);
void UUGUIWorldSubsystem::RaycastAll(const UPointerEventData* EventData, FRaycastResult& RaycastResult)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RaycastAll);
	
	if (bIsDirty)
	{
		bIsDirty = false;
		UpdateRectTransformOctree();
	}
	
	int32 RenderOrder = MAX_int32;
	TArray<TWeakObjectPtr<UCanvasSubComponent>, TInlineAllocator<8>> InvalidCanvas;
	for (auto& CanvasOctree : CanvasOctrees)
	{
		auto& CanvasWeakObject = CanvasOctree.Key;
		if (!CanvasWeakObject.IsValid())
		{
			InvalidCanvas.Add(CanvasWeakObject);
			continue;
		}

		UCanvasSubComponent* TargetCanvas = CanvasWeakObject.Get();
		if (!IsValid(TargetCanvas) || !IsValid(TargetCanvas->GetRectTransform()))
		{
			InvalidCanvas.Add(CanvasWeakObject);
			continue;
		}

		const int32 CurRenderOrder = TargetCanvas->GetRenderOrder();
		if (CurRenderOrder > RenderOrder)
		{
			continue;
		}

		const auto OverrideCanvas = TargetCanvas->GetOrderOverrideCanvas();
		UCanvasSubComponent* RootCanvas = nullptr;
		if (OverrideCanvas->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
		{
			RootCanvas = OverrideCanvas->GetRootCanvas();
		}

		FVector RayOrigin;
		FVector RayDirection;
		
		if (const FWorldRayInfo* RayInfo = CacheWorldRays.Find(RootCanvas))
		{
			RayOrigin = RayInfo->RayOrigin;
			RayDirection = RayInfo->RayDirection;
		}
		else
		{
			if (!FRectTransformUtility::GetWorldRayFromCanvas(TargetCanvas, FVector2D(EventData->Position), RayOrigin, RayDirection))
				continue;

			CacheWorldRays.Add(RootCanvas, FWorldRayInfo(RayOrigin, RayDirection));
		}

		int32 Depth = -1;
		
		FVector InvRayDirection = FVector(1 / RayDirection.X, 1 / RayDirection.Y, 1 / RayDirection.Z);
		const VectorRegister VectorRayOrigin = VectorLoadFloat3(&RayOrigin);
		const VectorRegister VectorInvRayDirection = VectorLoadFloat3(&InvRayDirection);
		CanvasOctree.Value.FindElements([&VectorRayOrigin, &VectorInvRayDirection](FVector Center, FVector Extent)
		{
			SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_OctreeFindElements);
			
			const VectorRegister VectorCenter = VectorLoadFloat3(&Center);
			const VectorRegister VectorExtent = VectorLoadFloat3(&Extent);

			const VectorRegister Min = VectorSubtract(VectorCenter, VectorExtent);
			const VectorRegister Max = VectorAdd(VectorCenter, VectorExtent);

			const VectorRegister T1 = VectorMultiply(VectorSubtract(Min, VectorRayOrigin), VectorInvRayDirection);
			const VectorRegister T2 = VectorMultiply(VectorSubtract(Max, VectorRayOrigin), VectorInvRayDirection);

			const VectorRegister VectorTMax = VectorMax(T1, T2);
			const VectorRegister VectorTMin = VectorMin(T1, T2);
			VectorStoreFloat3(VectorTMax, &Center);
			VectorStoreFloat3(VectorTMin, &Extent);

			const float TMax = FMath::Min(Center[0], FMath::Min(Center[1], Center[2]));
			const float TMin = FMath::Max(Extent[0], FMath::Max(Extent[1], Extent[2]));
			
			return TMax >= TMin && TMax >= 0;
		},
		[&Depth, &RayOrigin, &RayDirection, &EventData, &RaycastResult](UObject* Rect, const FHashOctreeObjectItem& ObjectItem)
		{
			SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_TestRectTransform);
			
			const auto Graphic = Cast<IGraphicElementInterface>(Rect);

			if (!Graphic)
				return;

			const int32 CurDepth = Graphic->GetDepth();
			// -1 means it hasn't been processed by the canvas, which means it isn't actually drawn
			if (CurDepth == -1 || !Graphic->IsRaycastTarget() || CurDepth < Depth)
				return;

			const auto CanvasRenderer = Graphic->GetCanvasRenderer();
			if (IsValid(CanvasRenderer) && CanvasRenderer->IsShouldCull())
				return;

			const auto RectTransform = Graphic->GetTransformComponent();
			if (!IsValid(RectTransform))
				return;
			
			if (!FRectTransformUtility::RectangleIntersectRayInWorldSpace(RectTransform, RayOrigin, RayDirection, RectTransform->IsIgnoreReversedGraphicsInHierarchy(),
				ObjectItem.WorldCorner0, ObjectItem.WorldCorner1, ObjectItem.WorldCorner2, ObjectItem.WorldCorner3))
				return;

			if (Graphic->Raycast(RayOrigin, RayDirection))
			{
				Depth = CurDepth;
				RaycastResult.GameObject = RectTransform;
				RaycastResult.ScreenPosition = EventData->Position;
			}
		});
		
		if (Depth > -1)
		{
			RenderOrder = CurRenderOrder;
		}
	}

	for (const auto& InvalidCanvasOctree : InvalidCanvas)
	{
		CanvasOctrees.Remove(InvalidCanvasOctree);
	}

	CacheWorldRays.Reset();
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- UpdateRectTransformOctree"), STAT_UnrealGUI_UpdateRectTransformOctree, STATGROUP_UnrealGUI);
void UUGUIWorldSubsystem::UpdateRectTransformOctree()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateRectTransformOctree);
	
	for (const auto& RectTransformPair : PendingAddGraphics)
	{
		const auto& CanvasWeakObject = RectTransformPair.Key;
		if (!CanvasWeakObject.IsValid())
			continue;

		const auto TargetCanvas = CanvasWeakObject.Get();
		auto& CanvasOctree = CanvasOctrees.FindOrAdd(TargetCanvas);
		for (const auto& RectTransform : RectTransformPair.Value)
		{
			CanvasOctree.AddElement(RectTransform.Get());
		}
	}

	for (const auto& RectTransformPair : PendingRemoveGraphics)
	{
		const auto& CanvasWeakObject = RectTransformPair.Key;
		if (!CanvasWeakObject.IsValid())
			continue;

		const auto TargetCanvas = CanvasWeakObject.Get();
		if (const auto CanvasOctreePtr = CanvasOctrees.Find(TargetCanvas))
		{
			for (const auto& RectTransform : RectTransformPair.Value)
			{
				CanvasOctreePtr->RemoveElement(RectTransform.Get());
			}

			if (CanvasOctreePtr->GetElementNum() <= 0)
			{
				CanvasOctrees.Remove(TargetCanvas);
			}
		}
	}
	
	PendingAddGraphics.Reset();
	PendingRemoveGraphics.Reset();
}

void UUGUIWorldSubsystem::AddUnusedMesh(USceneComponent* UIMeshComp)
{
	if (!IsValid(UIMeshComp))
		return;

	UIMeshes.Add(UIMeshComp);

	// TODO 两分钟执行一次检查
	// TODO 数量多了以后，起一个定时器，如果长事件没有使用到，就销毁
	// DynamicMaterialInstances 清空和数量超标检查
}

USceneComponent* UUGUIWorldSubsystem::GetUnusedMesh()
{
	if (UIMeshes.Num() > 0)
	{
		return UIMeshes.Pop();
	}
	
	const auto World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}
	
	if (!IsValid(UIMeshActor))
	{
		FActorSpawnParameters SpawnInfo;
#if WITH_EDITOR
		SpawnInfo.bHideFromSceneOutliner = true;
#endif
		SpawnInfo.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;
		UIMeshActor = World->SpawnActor<AActor>(SpawnInfo);
	}
	
	const auto SceneComponent = NewObject<USceneComponent>(UIMeshActor, UUIMeshProxyComponent::StaticClass(), NAME_None, RF_Transient);

#if WITH_EDITOR
	SceneComponent->CreationMethod = EComponentCreationMethod::Instance;
	SceneComponent->bIsEditorOnly = true;
#endif

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	++UIMeshCreationTimes;
#endif
	
	if(!SceneComponent->IsRegistered())	
	{
		SceneComponent->RegisterComponentWithWorld(GetWorld());
	}
	
	return SceneComponent;
}

void UUGUIWorldSubsystem::AddUnusedMaterial(UMaterialInterface* ParentMaterial,
	UMaterialInstanceDynamic* MaterialInstanceDynamic)
{
	if (!IsValid(ParentMaterial) || !IsValid(MaterialInstanceDynamic))
		return;

	MaterialInstanceDynamic->ClearParameterValues();
	FMaterialInstanceDynamicPool& InstancePool = DynamicMaterialInstances.FindOrAdd(ParentMaterial);
	InstancePool.Instances.Push(MaterialInstanceDynamic);
}

UMaterialInstanceDynamic* UUGUIWorldSubsystem::GetUnusedMaterial(UMaterialInterface* ParentMaterial)
{
	if (!IsValid(ParentMaterial))
		return nullptr;
	
	UMaterialInstanceDynamic* NewInstance = nullptr;
	if (FMaterialInstanceDynamicPool* InstancePoolPtr = DynamicMaterialInstances.Find(ParentMaterial))
	{
		if (InstancePoolPtr->Instances.Num() > 0)
		{
			NewInstance = InstancePoolPtr->Instances.Pop();
		}
	}

	if (!IsValid(NewInstance))
	{
		NewInstance = UMaterialInstanceDynamic::Create(ParentMaterial, this);
	}

	return NewInstance;
}

UAudioComponent* UUGUIWorldSubsystem::GetAudioComponent(UObject* PrincipalObject, bool bMasterTrack)
{
	if (!IsValid(PrincipalObject))
		return nullptr;

	USceneComponent* SceneComponent = nullptr;
	FString ObjectName;

	if (!bMasterTrack)
	{
		if (PrincipalObject->IsA<AActor>())
		{
			AActor* Actor = Cast<AActor>(PrincipalObject);
			SceneComponent = Actor->GetRootComponent();
			ObjectName =
#if WITH_EDITOR
			Actor->GetActorLabel();
#else
			Actor->GetName();
#endif
		}
		else if (USceneComponent* AttachSceneComp = Cast<USceneComponent>(PrincipalObject))
		{
			SceneComponent = AttachSceneComp;
			ObjectName = AttachSceneComp->GetName();
		}

		if (!IsValid(SceneComponent))
		{
			UE_LOG(LogUGUI, Warning, TEXT("UUGUIWorldSubsystem::GetAudioComponent --- SceneComponent is nullptr"));
			return nullptr;
		}
	}
	
	if (UIAudioComponents.Num() > 0)
	{
		UAudioComponent* ExistingComponent = UIAudioComponents.Pop();

		if (IsValid(ExistingComponent))
		{
			if (!bMasterTrack && (ExistingComponent->GetAttachParent() != SceneComponent))
			{
				ExistingComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);
				ExistingComponent->SetRelativeTransform(FTransform::Identity);
			}
			
			return ExistingComponent;
		}
	}
	
	USoundCue* TempPlaybackAudioCue = NewObject<USoundCue>();
	const FAudioDevice::FCreateComponentParams Params(GetWorld());
	UAudioComponent* NewAudioComponent = FAudioDevice::CreateComponent(TempPlaybackAudioCue, Params);

	if (!IsValid(NewAudioComponent))
	{
		UE_LOG(LogUGUI, Warning, TEXT("UUGUIWorldSubsystem::GetAudioComponent --- Failed to create audio component for UISequenceAudioComponent '%s'"), *ObjectName);
		return nullptr;
	}
	
	NewAudioComponent->SetFlags(RF_Transient);

	if (!bMasterTrack && (NewAudioComponent->GetAttachParent() != SceneComponent))
	{
		NewAudioComponent->AttachToComponent(SceneComponent, FAttachmentTransformRules::KeepRelativeTransform);
		NewAudioComponent->SetRelativeTransform(FTransform::Identity);
	}

	return NewAudioComponent;
}

void UUGUIWorldSubsystem::CacheAudioComponent(UAudioComponent* CacheAudioComp)
{
	if (IsValid(CacheAudioComp))
	{
		CacheAudioComp->Stop();
		CacheAudioComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
		UIAudioComponents.Push(CacheAudioComp);
	}
}

void UUGUIWorldSubsystem::ShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	{
		const UFont* RenderFont = GEngine->GetMediumFont();
		Canvas->SetDrawColor(FColor::Blue);
		YPos += 16;

		YPos += Canvas->DrawText(RenderFont, TEXT("UI World Subsystem :"), 4, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("UI Mesh Creation Times: %d\n"), UIMeshCreationTimes), 24, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("MaterialDynamicInstance Creation Times: %d\n"), MaterialDynamicInstanceCreationTimes), 24, YPos);
	}
#endif
}

/////////////////////////////////////////////////////
