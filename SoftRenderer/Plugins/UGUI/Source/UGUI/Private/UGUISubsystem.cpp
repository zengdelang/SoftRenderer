#include "UGUISubsystem.h"
#include "SpriteMergeSubsystem.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Culling/ClipperRegistry.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Render/CanvasManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// FWorldViewportInfo

void FWorldViewportInfo::UpdateViewportSize(const UWorld* InWorld, FVector2D InViewportSize)
{
	if (!ViewportSize.Equals(InViewportSize))
	{
		ViewportSize = InViewportSize;
		UUGUISubsystem::OnEditorViewportInfoChanged.Broadcast(InWorld);
	}
}

void FWorldViewportInfo::SetRenderMode(const UWorld* InWorld, ECanvasRenderMode InRenderMode)
{
	if (CanvasRenderMode != InRenderMode)
	{
		CanvasRenderMode = InRenderMode;
		UUGUISubsystem::OnEditorViewportInfoChanged.Broadcast(InWorld);
	}
}

void FWorldViewportInfo::SetEventViewportClient(UObject* InEventViewportClient)
{
	EventViewportClient = InEventViewportClient;
}

/////////////////////////////////////////////////////
// UUGUISubsystem

TMap<TWeakObjectPtr<UWorld>, FWorldViewportInfo> UUGUISubsystem::WorldViewportInfoList;
FOnEditorViewportInfoChanged UUGUISubsystem::OnEditorViewportInfoChanged;

UUGUISubsystem::UUGUISubsystem()
{
 
}

bool UUGUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return FApp::CanEverRender();
}

void UUGUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	FSlateThrottleManager::Get();
	UKismetSystemLibrary::ExecuteConsoleCommand(nullptr, TEXT("Slate.bAllowThrottling 0"));
#endif
}

void UUGUISubsystem::Deinitialize()
{
	FCanvasUpdateRegistry::Shutdown();
	FCanvasManager::Shutdown();
	FLayoutRebuilder::Shutdown();
	FClipperRegistry::Shutdown();
	
	Super::Deinitialize();
}

DECLARE_CYCLE_STAT(TEXT("UIBase --- SubsystemTick"), STAT_UnrealGUI_SubsystemTick, STATGROUP_UnrealGUI);
void UUGUISubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_SubsystemTick);

	USpriteMergeSubsystem::OnSpriteMergeTick.Broadcast();
	
	OnPreWillRenderCanvases.Broadcast();
	
	FCanvasManager::RefreshLateUpdateObjects();
	UCanvasSubComponent::OnWillRenderCanvases.Broadcast();
	FCanvasManager::WillRenderCanvases();
	
	OnAfterSubsystemTick.Broadcast();
}

FWorldViewportInfo* UUGUISubsystem::GetWorldViewportInfo(const UObject* WorldContextObject, bool bAddDefaultValue)
{
	if (!IsValid(WorldContextObject))
		return nullptr;

	const auto World = WorldContextObject->GetWorld();
	if (!IsValid(World) || World->IsGameWorld())
		return nullptr;

	FWorldViewportInfo* WorldViewportInfoPtr = WorldViewportInfoList.Find(World);
	if (!WorldViewportInfoPtr && bAddDefaultValue)
	{
		WorldViewportInfoPtr = &WorldViewportInfoList.Emplace(World, FWorldViewportInfo());
	}
	return WorldViewportInfoPtr;
}

FVector2D UUGUISubsystem::GetViewportSize(UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
		return FVector2D(1, 1);

	const auto World = WorldContextObject->GetWorld();
	if (!IsValid(World))
		return FVector2D(1, 1);

	if (World->IsGameWorld())
	{
		return UWidgetLayoutLibrary::GetViewportSize(WorldContextObject);
	}

	const FWorldViewportInfo* WorldViewportInfoPtr = WorldViewportInfoList.Find(World);
	if (WorldViewportInfoPtr)
	{
		return WorldViewportInfoPtr->GetViewportSize();
	}
	
	return FVector2D(1, 1);
}

UObject* UUGUISubsystem::GetEventViewportClient(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
		return nullptr;

	const auto World = WorldContextObject->GetWorld();
	if (!IsValid(World))
		return nullptr;

	if (World->IsGameWorld())
	{
		return World->GetGameViewport();
	}

	const FWorldViewportInfo* WorldViewportInfoPtr = WorldViewportInfoList.Find(World);
	if (WorldViewportInfoPtr)
	{
		return WorldViewportInfoPtr->GetEventViewportClient();
	}

	return nullptr;
}

void UUGUISubsystem::SetEventViewportClient(const UObject* WorldContextObject, UObject* InEventViewportClient)
{
	FWorldViewportInfo* ViewportInfo = GetWorldViewportInfo(WorldContextObject, true);
	if (ViewportInfo)
	{
		ViewportInfo->SetEventViewportClient(InEventViewportClient);
	}
}

/////////////////////////////////////////////////////
