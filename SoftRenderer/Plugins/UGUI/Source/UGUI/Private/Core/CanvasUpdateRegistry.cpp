#include "Core/CanvasUpdateRegistry.h"
#include "SpriteMergeSubsystem.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Core/Culling/ClipperRegistry.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UCanvasUpdateRegistry

UCanvasUpdateRegistry::UCanvasUpdateRegistry(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SwapChainIndex = 0;
	
    bPerformingLayoutUpdate = false;
    bPerformingGraphicUpdate = false;
}

void UCanvasUpdateRegistry::Initialize()
{
    LayoutRebuildQueueList.SetNum(2);
    GraphicRebuildQueueList.SetNum(2);
}

void UCanvasUpdateRegistry::CleanInvalidItems(uint8 CurChainIndex)
{
    auto& LayoutRebuildQueue = LayoutRebuildQueueList[CurChainIndex];
    for (int32 Index = LayoutRebuildQueue.Num() - 1; Index >= 0; --Index)
    {
        const auto& Item = LayoutRebuildQueue[Index];

        if (!IsValid(Item))
        {
            LayoutRebuildQueue.RemoveAt(Index);
            continue;
        }

        const auto CanvasElement = Cast<ICanvasElementInterface>(Item);
        if (CanvasElement && CanvasElement->IsDestroyed())
        {
            LayoutRebuildQueue.RemoveAt(Index);
            CanvasElement->LayoutComplete();
        }
    }

    auto& GraphicRebuildQueue = GraphicRebuildQueueList[CurChainIndex];
    for (int32 Index = GraphicRebuildQueue.Num() - 1; Index >= 0; --Index)
    {
        const auto& Item = GraphicRebuildQueue[Index];

        if (!IsValid(Item))
        {
            GraphicRebuildQueue.RemoveAt(Index);
            continue;
        }

        const auto CanvasElement = Cast<ICanvasElementInterface>(Item);
        if (CanvasElement && CanvasElement->IsDestroyed())
        {
            GraphicRebuildQueue.RemoveAt(Index);
            CanvasElement->GraphicUpdateComplete();
        }
    }
}

bool UCanvasUpdateRegistry::InternalPerformUpdate(uint8 CurChainIndex)
{
    auto& LayoutRebuildQueue = LayoutRebuildQueueList[CurChainIndex];
    auto& GraphicRebuildQueue = GraphicRebuildQueueList[CurChainIndex];

    if (LayoutRebuildQueue.Num() <= 0 && GraphicRebuildQueue.Num() == 0 && !FClipperRegistry::GetInstance()->IsDirty())
        return false;
	
    CleanInvalidItems(CurChainIndex);
	
    PerformLayoutUpdate(LayoutRebuildQueue);

    // now layout is complete do culling...
    FClipperRegistry::GetInstance()->Cull();

    USpriteMergeSubsystem::OnSpriteMergeTryAgainTick.Broadcast();

    PerformGraphicUpdate(GraphicRebuildQueue);
	
    return true;
}

DECLARE_CYCLE_STAT(TEXT("UICanvasUpdate --- PerformLayoutUpdate"), STAT_UnrealGUI_PerformLayoutUpdate, STATGROUP_UnrealGUI);
void UCanvasUpdateRegistry::PerformLayoutUpdate(FCanvasElementIndexedSet& LayoutRebuildQueue)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_PerformLayoutUpdate);
    
    bPerformingLayoutUpdate = true;

    LayoutRebuildQueue.Sort();

    for (int32 Phase = 0; Phase <= static_cast<int32>(ECanvasUpdate::CanvasUpdate_PostLayout); ++Phase)
    {
        for (int32 Index = 0, Count = LayoutRebuildQueue.Num(); Index < Count; ++Index)
        {
            if (!IsValid(LayoutRebuildQueue[Index]))
                continue;

            const auto CanvasElement = Cast<ICanvasElementInterface>(LayoutRebuildQueue[Index]);
            if (CanvasElement)
            {
                CanvasElement->Rebuild(static_cast<ECanvasUpdate>(Phase));
            }
        }
    }

    for (int32 Index = 0, Count = LayoutRebuildQueue.Num(); Index < Count; ++Index)
    {
        if (!IsValid(LayoutRebuildQueue[Index]))
            continue;

        const auto CanvasElement = Cast<ICanvasElementInterface>(LayoutRebuildQueue[Index]);
        if (CanvasElement)
        {
            CanvasElement->LayoutComplete();
        }
    }

    LayoutRebuildQueue.Reset();

    bPerformingLayoutUpdate = false;
}

DECLARE_CYCLE_STAT(TEXT("UICanvasUpdate --- PerformGraphicUpdate"), STAT_UnrealGUI_PerformGraphicUpdate, STATGROUP_UnrealGUI);
void UCanvasUpdateRegistry::PerformGraphicUpdate(FCanvasElementIndexedSet& GraphicRebuildQueue)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_PerformGraphicUpdate);
    
    bPerformingGraphicUpdate = true;

    for (int32 Phase = static_cast<int32>(ECanvasUpdate::CanvasUpdate_PreRender); Phase < static_cast<int32>(ECanvasUpdate::CanvasUpdate_MaxUpdateValue); ++Phase)
    {
        for (int32 Index = 0, Count = GraphicRebuildQueue.Num(); Index < Count; ++Index)
        {
            if (!IsValid(GraphicRebuildQueue[Index]))
                continue;

            const auto CanvasElement = Cast<ICanvasElementInterface>(GraphicRebuildQueue[Index]);
            if (CanvasElement)
            {
                CanvasElement->Rebuild(static_cast<ECanvasUpdate>(Phase));
            }
        }
    }

    for (int32 Index = 0, Count = GraphicRebuildQueue.Num(); Index < Count; ++Index)
    {
        if (!IsValid(GraphicRebuildQueue[Index]))
            continue;

        const auto CanvasElement = Cast<ICanvasElementInterface>(GraphicRebuildQueue[Index]);
        if (CanvasElement)
        {
            CanvasElement->GraphicUpdateComplete();
        }
    }

    GraphicRebuildQueue.Reset();

    bPerformingGraphicUpdate = false;
}

void UCanvasUpdateRegistry::PerformUpdate()
{
    int32 SwapCount = 0;
    while (SwapCount < 10)
    {
        const uint8 CurChainIndex = SwapChainIndex;
        SwapChainIndex = 1 - SwapChainIndex;
    	if (!InternalPerformUpdate(CurChainIndex))
    	{
            return;
    	}

        ++SwapCount;
    }

    UE_LOG(LogUGUI, Warning, TEXT("UCanvasUpdateRegistry --- SwapCount > 10, There may be infinite cycles"));
}

bool UCanvasUpdateRegistry::InternalRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element)
{
    if (!Element)
        return false;

    const uint8 CurChainIndex = 1 - SwapChainIndex;
    if (LayoutRebuildQueueList[CurChainIndex].Contains(Element))
        return false;
    
    return LayoutRebuildQueueList[SwapChainIndex].AddUnique(Element);
}

bool UCanvasUpdateRegistry::InternalRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element)
{
    if (!Element)
        return false;

    // Rich text components may need to register multiple updates
    /*const uint8 CurChainIndex = 1 - SwapChainIndex;
    if (GraphicRebuildQueueList[CurChainIndex].Contains(Element))
        return false;*/
    
    return GraphicRebuildQueueList[SwapChainIndex].AddUnique(Element);
}

void UCanvasUpdateRegistry::InternalUnRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element)
{
    if (!Element)
        return;

    Element->LayoutComplete();
    LayoutRebuildQueueList[SwapChainIndex].Remove(Element);
}

void UCanvasUpdateRegistry::InternalUnRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element)
{
    if (!Element)
        return;

    Element->GraphicUpdateComplete();
    GraphicRebuildQueueList[SwapChainIndex].Remove(Element);
}

/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// FCanvasUpdateRegistry

UCanvasUpdateRegistry* FCanvasUpdateRegistry::Instance = nullptr;
FDelegateHandle FCanvasUpdateRegistry::PerformUpdateHandle;

void FCanvasUpdateRegistry::Initialize()
{
	if (Instance)
	{
		return;
	}

	Instance = NewObject<UCanvasUpdateRegistry>();
    Instance->AddToRoot();
    Instance->Initialize();
    PerformUpdateHandle = UCanvasSubComponent::OnWillRenderCanvases.AddUObject(Instance, &UCanvasUpdateRegistry::PerformUpdate);
}

void FCanvasUpdateRegistry::Shutdown()
{
	if (Instance)
	{
		if (PerformUpdateHandle.IsValid())
		{
            UCanvasSubComponent::OnWillRenderCanvases.Remove(PerformUpdateHandle);
            PerformUpdateHandle.Reset();
		}

        Instance->RemoveFromRoot();
        Instance = nullptr;
	}
}

void FCanvasUpdateRegistry::RegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element)
{
    Initialize();
    Instance->InternalRegisterCanvasElementForLayoutRebuild(Element);
}

bool FCanvasUpdateRegistry::TryRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element)
{
    Initialize();
    return Instance->InternalRegisterCanvasElementForLayoutRebuild(Element);
}

void FCanvasUpdateRegistry::RegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element)
{
    Initialize();
    Instance->InternalRegisterCanvasElementForGraphicRebuild(Element);
}

bool FCanvasUpdateRegistry::TryRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element)
{
    Initialize();
    return Instance->InternalRegisterCanvasElementForGraphicRebuild(Element);
}

void FCanvasUpdateRegistry::UnRegisterCanvasElementForRebuild(ICanvasElementInterface* Element)
{
    Initialize();
    Instance->InternalUnRegisterCanvasElementForLayoutRebuild(Element);
    Instance->InternalUnRegisterCanvasElementForGraphicRebuild(Element);
}

/////////////////////////////////////////////////////
