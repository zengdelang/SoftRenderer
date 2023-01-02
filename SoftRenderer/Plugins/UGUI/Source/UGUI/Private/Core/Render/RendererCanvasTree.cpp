#include "Core/Render/RendererCanvasTree.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/Render/UIMeshBatchDescStorage.h"
#include "UGUIWorldSubsystem.h"
#include "UGUISettings.h"
#include "UGUISubsystem.h"
#include "Core/Renderer/UIMeshProxyComponent.h"

/////////////////////////////////////////////////////
// FRendererCanvasTree

// TODO 如果 FreeList超出一定时间，并且有效的太少就收缩下, 要小心Children没清空
void FRendererCanvasTree::AddRenderer(UCanvasRendererSubComponent* Renderer)
{
	if (!IsValid(Renderer))
		return;

	if (const int32* NodeIndexPtr = NodeMap.Find(Renderer->AttachTransform))
	{
		if (TreeNodes[*NodeIndexPtr].Renderer == Renderer)
		{
			return;
		}
	}
	
	PendingAddRenderers.Add(Renderer);
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- RemoveRenderer"), STAT_UnrealGUI_RemoveRenderer, STATGROUP_UnrealGUI);
void FRendererCanvasTree::RemoveRenderer(const UCanvasRendererSubComponent* Renderer)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RemoveRenderer);
	
	if (!IsValid(Renderer))
		return;
	
	PendingAddRenderers.Remove(Renderer);

	const int32* NodeIndexPtr = NodeMap.Find(Renderer->AttachTransform);
	if (NodeIndexPtr)
	{
		FRendererCanvasTreeNode& CurNode = TreeNodes[*NodeIndexPtr];
		if (CurNode.Renderer != Renderer)
			return;

		CurNode.Renderer = nullptr;
		
		if (const auto IndexPtr = RendererCanvasMap.Find(Renderer))
		{
			FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[*IndexPtr];

			if (CurChainNode.Prev != -1)
			{
				FRendererCanvasChainNode& PrevChainNode = RendererCanvasChain[CurChainNode.Prev];
				PrevChainNode.Next = CurChainNode.Next;
			}
			else
			{
				FirstChainIndex = CurChainNode.Next;
			}
	
			if (CurChainNode.Next != -1)
			{
				FRendererCanvasChainNode& NextChainNode = RendererCanvasChain[CurChainNode.Next];
				NextChainNode.Prev = CurChainNode.Prev;
			}

			CurChainNode.Next = FreeChainListIndex;
			FreeChainListIndex = *IndexPtr;

			if (Renderer->OwnerRenderProxyInfo.IsValid())
			{
				DirtyFlag |= CDF_BatchDirty;
			}
			
			RendererCanvasMap.Remove(Renderer);
		}
		
		RemoveNode(*NodeIndexPtr);
	}
}

void FRendererCanvasTree::AddCanvas(UCanvasSubComponent* Canvas)
{
	if (!IsValid(Canvas))
		return;

	if (const int32* NodeIndexPtr = NodeMap.Find(Canvas->AttachTransform))
	{
		if (TreeNodes[*NodeIndexPtr].Canvas == Canvas)
		{
			return;
		}
	}
	
	PendingAddCanvases.Add(Canvas);
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- RemoveCanvas"), STAT_UnrealGUI_RemoveCanvas, STATGROUP_UnrealGUI);
void FRendererCanvasTree::RemoveCanvas(UCanvasSubComponent* Canvas)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RemoveCanvas);
	
	if (!IsValid(Canvas))
		return;
	
	PendingAddCanvases.Remove(Canvas);

	const int32* NodeIndexPtr = NodeMap.Find(Canvas->AttachTransform);
	if (NodeIndexPtr)
	{
		FRendererCanvasTreeNode& CurNode = TreeNodes[*NodeIndexPtr];
		if (CurNode.Canvas != Canvas)
			return;

		CurNode.Canvas = nullptr;
		
		if (const auto IndexPtr = RendererCanvasMap.Find(Canvas))
		{
			Canvas->CacheAllUIMesh(true);
			CheckBatchDirty(Canvas);
			
			FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[*IndexPtr];

			if (CurChainNode.Prev != -1)
			{
				FRendererCanvasChainNode& PrevChainNode = RendererCanvasChain[CurChainNode.Prev];
				PrevChainNode.Next = CurChainNode.Next;
			}
			else
			{
				FirstChainIndex = CurChainNode.Next;
			}
	
			if (CurChainNode.Next != -1)
			{
				FRendererCanvasChainNode& NextChainNode = RendererCanvasChain[CurChainNode.Next];
				NextChainNode.Prev = CurChainNode.Prev;
			}

			CurChainNode.Next = FreeChainListIndex;
			FreeChainListIndex = *IndexPtr;
			
			RendererCanvasMap.Remove(Canvas);
		}
		
		RemoveNode(*NodeIndexPtr);
		NestedCanvases.Remove(Canvas);
	}
}

void FRendererCanvasTree::NotifyCanvasStateChanged(UCanvasSubComponent* Canvas, bool bShow)
{
	if (!bShow)
	{
		UpdateBatchDirtyCanvases.Add(Canvas);
	}
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateTree"), STAT_UnrealGUI_UpdateTree, STATGROUP_UnrealGUI);
void FRendererCanvasTree::UpdateTree()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateTree);
	
	if (PendingAddRenderers.Num() > 0)
	{
		for (const auto& Renderer : PendingAddRenderers)
		{
			AddRendererNode(Renderer);
		}

		PendingAddRenderers.Empty();
	}

	if (PendingAddCanvases.Num() > 0)
	{
		for (const auto& Canvas : PendingAddCanvases)
		{
			AddCanvasNode(Canvas);
		}

		PendingAddCanvases.Empty();
	}
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateBatchDirty"), STAT_UnrealGUI_UpdateBatchDirty, STATGROUP_UnrealGUI);
void FRendererCanvasTree::UpdateBatchDirty()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateBatchDirty);
	
	if ((DirtyFlag & CDF_BatchDirty) != 0)
	{
		UpdateBatchDirtyCanvases.Empty();
		return;
	}

	if (UpdateBatchDirtyCanvases.Num() > 0)
	{
		for (const auto& Canvas : UpdateBatchDirtyCanvases)
		{
			if (CheckBatchDirty(Canvas))
			{
				break;
			}
		}

		UpdateBatchDirtyCanvases.Empty();
	}
}

bool FRendererCanvasTree::CheckBatchDirty(UCanvasSubComponent* Canvas)
{
	TArray<UCanvasSubComponent*, TInlineAllocator<64>> CanvasList;
	CanvasList.Add(Canvas);

	const UCanvasRendererSubComponent* PreRenderer = nullptr;
	const UCanvasRendererSubComponent* NextRenderer = nullptr;
			
	if (const auto IndexPtr = RendererCanvasMap.Find(Canvas))
	{
		int32 PreChainIndex = RendererCanvasChain[*IndexPtr].Prev;
		int32 NextChainIndex = RendererCanvasChain[*IndexPtr].Next;
				
		while (PreChainIndex != -1)
		{
			const FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[PreChainIndex];
			const FRendererCanvasTreeNode& CurNode = TreeNodes[CurChainNode.NodeIndex];
			
			if (IsValid(CurNode.Renderer))
			{
				PreRenderer = CurNode.Renderer;
				break;
			}

			if (IsValid(CurNode.Canvas))
			{
				CanvasList.Add(CurNode.Canvas);
			}
		
			PreChainIndex = CurChainNode.Prev;
		}
				
		while (NextChainIndex != -1)
		{
			const FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[NextChainIndex];
			const FRendererCanvasTreeNode& CurNode = TreeNodes[CurChainNode.NodeIndex];
			
			if (IsValid(CurNode.Renderer))
			{
				NextRenderer = CurNode.Renderer;
				break;
			}

			if (IsValid(CurNode.Canvas))
			{
				CanvasList.Add(CurNode.Canvas);
			}
		
			NextChainIndex = CurChainNode.Next;
		}
	}

	if (IsValid(PreRenderer) && IsValid(NextRenderer))
	{
		if (PreRenderer->OwnerRenderProxyInfo == NextRenderer->OwnerRenderProxyInfo)
		{
			bool bMarkBatchDirty = false;
					
			for (const auto& MiddleCanvas : CanvasList)
			{
				if (HasUIBatches(MiddleCanvas))
				{
					bMarkBatchDirty = true;
					break;
				}
			}

			if (bMarkBatchDirty)
			{
				DirtyFlag |= CDF_BatchDirty;
				return true;
			}
		}
		else
		{
			bool bMarkBatchDirty = true;
					
			for (const auto& MiddleCanvas : CanvasList)
			{
				if (HasUIBatches(MiddleCanvas))
				{
					bMarkBatchDirty = false;
					break;
				}
			}

			if (bMarkBatchDirty)
			{
				DirtyFlag |= CDF_BatchDirty;
				return true;
			}
		}
	}

	return false;
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateDepth"), STAT_UnrealGUI_UpdateDepth, STATGROUP_UnrealGUI);
void FRendererCanvasTree::UpdateDepth()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_UpdateDepth);
	
	if ((DirtyFlag & CDF_DepthDirty) != 0)
	{
		NestedCanvases.Reset();
		
		int32 CurNestedCanvasNum = 0;
		int32 Depth = 0;
		int32 CurChainIndex = FirstChainIndex;
		
		while (CurChainIndex != -1)
		{
			const FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[CurChainIndex];
			const FRendererCanvasTreeNode& CurNode = TreeNodes[CurChainNode.NodeIndex];
			
			if (IsValid(CurNode.Renderer))
			{
				CurNode.Renderer->SetAbsoluteDepth(Depth);
				++Depth;
			}
			else if (IsValid(CurNode.Canvas))
			{
				NestedCanvases.Add(CurNode.Canvas);

				CurNode.Canvas->SetNestedRenderDepth(Depth);
				++Depth;
				
				Depth += CurNode.Canvas->CanvasData.MaxDepth;
				CurNestedCanvasNum += (1 + CurNode.Canvas->CanvasData.NestedCanvasNum);
			}
		
			CurChainIndex = CurChainNode.Next;
		}

		const int32 NewMaxDepth = FMath::Max(Depth, 20000 * (CurNestedCanvasNum + 1));
		
		if (NewMaxDepth > MaxDepth && !ThisCanvas->IsOverrideSorting())
		{
			if (const auto ParentCanvas = ThisCanvas->GetParentCanvas())
			{
				ParentCanvas->GetCanvasData().DirtyFlag |= CDF_DepthDirty; 
			}
		}

		MaxDepth = NewMaxDepth;
		NestedCanvasNum = CurNestedCanvasNum;
		
		DirtyFlag &= ~CDF_DepthDirty;
	}
}

void FRendererCanvasTree::AddRendererNode(UCanvasRendererSubComponent* Renderer)
{
	if (!IsValid(Renderer))
		return;

	if (RendererCanvasMap.Contains(Renderer))
		return;

	const int32 NodeIndex = AddNode(Renderer->AttachTransform);
	if (NodeIndex != -1)
	{
		auto& CurNode = TreeNodes[NodeIndex];
		CurNode.Renderer = Renderer;

		DirtyFlag |= CDF_DepthDirty;
		
		if (Renderer->IsActiveAndEnabled())
		{
			DirtyFlag |= CDF_BatchDirty;
		}

		UpdateChainNode(NodeIndex, CurNode.Prev, Renderer);
	}
}

void FRendererCanvasTree::AddCanvasNode(UCanvasSubComponent* Canvas)
{
	if (!IsValid(Canvas))
		return;

	if (RendererCanvasMap.Contains(Canvas))
		return;

	const int32 NodeIndex = AddNode(Canvas->AttachTransform);
	if (NodeIndex != -1)
	{
		auto& CurNode = TreeNodes[NodeIndex];
		CurNode.Canvas = Canvas;

		NestedCanvases.Add(Canvas);
		DirtyFlag |= CDF_DepthDirty;

		Canvas->CanvasData.DirtyFlag |= CDF_BatchDirty;

		if (Canvas->IsActiveAndEnabled())
		{
			UpdateBatchDirtyCanvases.Add(Canvas);
		}
		
		UpdateChainNode(NodeIndex, CurNode.Prev, Canvas);
	}
}

void FRendererCanvasTree::UpdateChainNode(int32 NodeIndex, int32 Prev, UObject* Obj)
{
	int32 PreChainNodeIndex = -1;
	while (Prev != -1)
	{
		const auto& PrevNode = TreeNodes[Prev];

		if (IsValid(PrevNode.Renderer))
		{
			PreChainNodeIndex = *RendererCanvasMap.Find(PrevNode.Renderer);
			break;
		}

		if (IsValid(PrevNode.Canvas))
		{
			PreChainNodeIndex = *RendererCanvasMap.Find(PrevNode.Canvas);
			break;
		}

		Prev = PrevNode.Prev;
	}

	const int32 ChainNodeIndex = CreateChainNode();
	FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[ChainNodeIndex];
	CurChainNode.NodeIndex = NodeIndex;
		
	RendererCanvasMap.Add(Obj, ChainNodeIndex);
		
	if (PreChainNodeIndex != -1)
	{
		FRendererCanvasChainNode& PreChainNode = RendererCanvasChain[PreChainNodeIndex];
		
		CurChainNode.Next = PreChainNode.Next;
		CurChainNode.Prev = PreChainNodeIndex;
		
		if (PreChainNode.Next != -1)
		{
			FRendererCanvasChainNode& NextChainNode = RendererCanvasChain[PreChainNode.Next];
			NextChainNode.Prev = ChainNodeIndex;
		}
			
		PreChainNode.Next = ChainNodeIndex;
	}
	else
	{
		CurChainNode.Prev = -1;
		CurChainNode.Next = FirstChainIndex;
			
		if (FirstChainIndex != - 1)
		{
			RendererCanvasChain[FirstChainIndex].Prev = ChainNodeIndex;
		}
		FirstChainIndex = ChainNodeIndex;
	}
}

struct FRendererChildZOrder
{
	int32 ChildIndex;
	int32 ZOrder;

	FRendererChildZOrder(int32 InChildIndex, int32 InZOrder) : ChildIndex(InChildIndex), ZOrder(InZOrder) { }
};

static TMap<USceneComponent*, FRendererChildZOrder> TempChildrenOrder;

int32 FRendererCanvasTree::AddNode(USceneComponent* SceneComp)
{
	if (!IsValid(SceneComp))
		return -1;

	const int32* NodeIndexPtr = NodeMap.Find(SceneComp);
	if (NodeIndexPtr)
		return *NodeIndexPtr;

	const int32 CurNodeIndex = CreateNode();

	const int32 ParentNodeIndex = AddNode(SceneComp->GetAttachParent());
	if (ParentNodeIndex == -1)
	{
		FRendererCanvasTreeNode& CurNode = TreeNodes[CurNodeIndex];
		CurNode.Next = FreeListIndex;
		FreeListIndex = CurNodeIndex;
		return -1;
	}

	FRendererCanvasTreeNode& CurNode = TreeNodes[CurNodeIndex];
	FRendererCanvasTreeNode& ParentNode = TreeNodes[ParentNodeIndex];

	CurNode.Parent = ParentNodeIndex;
	CurNode.TreeComponent = SceneComp;

	NodeMap.Add(SceneComp, CurNodeIndex);
	
	if (ParentNode.Children.Num() <= 0)
	{
		ParentNode.Children.Add(CurNodeIndex);

		CurNode.Prev = ParentNodeIndex;
		CurNode.Next = ParentNode.Next;

		if (ParentNode.Next != -1)
		{
			FRendererCanvasTreeNode& NextNode = TreeNodes[ParentNode.Next];
			NextNode.Prev = CurNodeIndex;
		}
		ParentNode.Next = CurNodeIndex;
	}
	else
	{
		TempChildrenOrder.Reset();
		
		int32 Index = 0;
		for (const auto& ChildComp : ParentNode.TreeComponent->GetAttachChildren())
		{
			int32 ZOrder = 0;
			if (const URectTransformComponent* RectTransformComp = Cast<URectTransformComponent>(ChildComp))
			{
				ZOrder = RectTransformComp->GetZOrder();
			}
			
			TempChildrenOrder.Add(ChildComp, FRendererChildZOrder(Index, ZOrder));
			++Index;
		}

		const FRendererChildZOrder& ChildOrder = *TempChildrenOrder.Find(SceneComp);

		int32 InsertPos = ParentNode.Children.Num();
		for (int32 ChildIndex = InsertPos - 1; ChildIndex >= 0; --ChildIndex)
		{
			const FRendererChildZOrder* CurChildOrderPtr = TempChildrenOrder.Find(TreeNodes[ParentNode.Children[ChildIndex]].TreeComponent);

			if (ChildOrder.ZOrder == CurChildOrderPtr->ZOrder)
			{
				if (ChildOrder.ChildIndex > CurChildOrderPtr->ChildIndex)
				{
					break;
				}
			}
			else
			{
				if (ChildOrder.ZOrder > CurChildOrderPtr->ZOrder)
				{
					break;
				}
			}
 
			--InsertPos;
		}

		ParentNode.Children.Insert(CurNodeIndex, InsertPos);

		if (InsertPos == ParentNode.Children.Num() - 1)
		{
			int32 PreNodeIndex = ParentNode.Children[InsertPos - 1];
			while (const auto ChildNum = TreeNodes[PreNodeIndex].Children.Num())
			{
				PreNodeIndex = TreeNodes[PreNodeIndex].Children[ChildNum - 1];
			}

			FRendererCanvasTreeNode& PrevNode = TreeNodes[PreNodeIndex];
			
			CurNode.Prev = PreNodeIndex;
			CurNode.Next = PrevNode.Next;
			
			if (PrevNode.Next != -1)
			{
				FRendererCanvasTreeNode& NextNode = TreeNodes[PrevNode.Next];
				NextNode.Prev = CurNodeIndex;
			}
			
			PrevNode.Next = CurNodeIndex;
		}
		else
		{
			const int32 NextNodeIndex = ParentNode.Children[InsertPos + 1];
			FRendererCanvasTreeNode& NextNode = TreeNodes[NextNodeIndex];
			FRendererCanvasTreeNode& PrevNode = TreeNodes[NextNode.Prev];
			CurNode.Prev = NextNode.Prev;
			CurNode.Next = NextNodeIndex;
			NextNode.Prev = CurNodeIndex;
			PrevNode.Next = CurNodeIndex;
		}
	}

	return CurNodeIndex;
}

int32 FRendererCanvasTree::CreateNode()
{
	if (FreeListIndex == -1)
	{
		return TreeNodes.Emplace(FRendererCanvasTreeNode());
	}

	const int32 NodeIndex = FreeListIndex;
	FreeListIndex = TreeNodes[FreeListIndex].Next;
	return NodeIndex;
}

void FRendererCanvasTree::RemoveNode(int32 NodeIndex)
{
	FRendererCanvasTreeNode& CurNode = TreeNodes[NodeIndex];
	if (NodeIndex == 0 || CurNode.Children.Num() > 0 || IsValid(CurNode.Renderer) || IsValid(CurNode.Canvas))
	{
		return;
	}

	if (CurNode.Prev != -1)
	{
		FRendererCanvasTreeNode& PrevNode = TreeNodes[CurNode.Prev];
		PrevNode.Next = CurNode.Next;
	}
	
	if (CurNode.Next != -1)
	{
		FRendererCanvasTreeNode& NextNode = TreeNodes[CurNode.Next];
		NextNode.Prev = CurNode.Prev;
	}
	
	CurNode.Next = FreeListIndex;
	FreeListIndex = NodeIndex;
	
	NodeMap.Remove(CurNode.TreeComponent);
	CurNode.TreeComponent = nullptr;
	
	FRendererCanvasTreeNode& ParentNode = TreeNodes[CurNode.Parent];
	ParentNode.Children.Remove(NodeIndex);

	if (CurNode.Parent != 0 && ParentNode.Children.Num() == 0 && !IsValid(ParentNode.Renderer) && !IsValid(ParentNode.Canvas))
	{
		RemoveNode(CurNode.Parent);
	}
}

int32 FRendererCanvasTree::CreateChainNode()
{
	if (FreeChainListIndex == -1)
	{
		return RendererCanvasChain.Emplace(FRendererCanvasChainNode());
	}

	const int32 ChainNodeIndex = FreeChainListIndex;
	FreeChainListIndex = RendererCanvasChain[FreeChainListIndex].Next;
	return ChainNodeIndex;
}

static FUIMeshBatchDescStorage UIMeshBatchDescStorage;

DECLARE_CYCLE_STAT(TEXT("UICanvas --- MergeCanvasRenderer"), STAT_UnrealGUI_MergeCanvasRenderer, STATGROUP_UnrealGUI);
void FRendererCanvasTree::MergeCanvasRenderer(UCanvasSubComponent* Canvas, UCanvasSubComponent* OverrideCanvas, const UCanvasSubComponent* RootCanvas,
	USceneComponent* OverrideCanvasSceneComp, const USceneComponent* RectTransform, const FTransform& WorldToCanvasTransform)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_MergeCanvasRenderer);
	
	FUIMeshStorage& UIMeshStorage = ThisCanvas->UIMeshStorage;
	const int32 OldBatchNum = UIMeshStorage.UIMeshList.Num();
	
	int32 Section = 0;
	int32 CurChainIndex = FirstChainIndex;
	int32 InstructionIndex = -1;
	
	while (CurChainIndex != -1)
	{
		const FRendererCanvasChainNode& CurChainNode = RendererCanvasChain[CurChainIndex];
		const FRendererCanvasTreeNode& CurNode = 	TreeNodes[CurChainNode.NodeIndex];

		CurChainIndex = CurChainNode.Next;
		
		if (IsValid(CurNode.Canvas))
		{
			++InstructionIndex;
			CurNode.Canvas->SetCanvasInstructionIndex(InstructionIndex);

			if (HasUIBatches(CurNode.Canvas))
			{
				++Section;
			}
			continue;
		}

		++InstructionIndex;
		
		const auto& Handle = CurNode.Renderer;
		if (IsValid(Handle) && Handle->CanRender())
		{
			UIMeshBatchDescStorage.DoMergeCanvasRenderer(Canvas, Handle, InstructionIndex, Section, WorldToCanvasTransform, UIMeshStorage);
		}
	}
	
	for (int32 Index = UIMeshStorage.UIMeshList.Num() - 1; Index >= 0; --Index)
	{
		if (const auto UIMesh = Cast<USceneComponent>(UIMeshStorage.UIMeshList[Index]))
		{
			if (IUIRenderProxyInterface* RenderProxy = Cast<IUIRenderProxyInterface>(UIMesh))
			{
				if (RenderProxy->IsExternalRenderProxy() || RenderProxy->BatchDescIndex == -1)
				{
					RenderProxy->ClearUIMesh();
				}
			}
		}
	}
	
	for (int32 Index = 0, Count = UIMeshBatchDescStorage.UIMeshBatchDescList.Num(); Index < Count; ++Index)
	{
		FUIMeshBatchDesc& UIMeshBatchDesc = UIMeshBatchDescStorage.UIMeshBatchDescList[Index];
		
		if (!UIMeshBatchDesc.UIRenderProxy && !UIMeshBatchDesc.bIsExternalRenderProxy)
		{
			if (UIMeshStorage.UIWorldSubsystem.IsValid())
			{
				UIMeshBatchDesc.UIRenderProxy = Cast<IUIRenderProxyInterface>(UIMeshStorage.UIWorldSubsystem->GetUnusedMesh());
			}
		}

		IUIRenderProxyInterface* RenderProxy = UIMeshBatchDesc.UIRenderProxy;
		if (!RenderProxy)
			continue;

		const auto SceneComponent = Cast<USceneComponent>(RenderProxy);
		if (!IsValid(SceneComponent))
			continue;
		
		UIMeshStorage.UIMeshList.AddUnique(SceneComponent);

#if WITH_EDITOR
		ECanvasRenderMode TargetRenderMode = RootCanvas->GetRenderMode();
		TSharedPtr<FUISceneViewExtension, ESPMode::ThreadSafe>& ViewExtension = OverrideCanvas->GetViewExtension();
		
		const UWorld* World = SceneComponent->GetWorld();
		const FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(World);
		if (WorldViewportInfo)
		{
			TargetRenderMode = WorldViewportInfo->GetRenderMode();
		}

		if (IsValid(World))
		{
			if (World->WorldType == EWorldType::Type::Editor)
			{
				TargetRenderMode = ECanvasRenderMode::CanvasRenderMode_WorldSpace;
			}
		}

		if (TargetRenderMode == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
		{
			ViewExtension.Reset();
		}
#else
		const ECanvasRenderMode TargetRenderMode = RootCanvas->GetRenderMode();
		const TSharedPtr<FUISceneViewExtension, ESPMode::ThreadSafe>& ViewExtension = OverrideCanvas->GetViewExtension();
#endif

		bool bRefreshRenderProxy = false;
		if (!UIMeshBatchDesc.bIsExternalRenderProxy)
		{
			if (IsValid(RootCanvas))
			{
				USceneComponent* RootCanvasSceneComp = Cast<URectTransformComponent>(RootCanvas->GetOuter());
				if (IsValid(RootCanvasSceneComp))
				{
					SceneComponent->AttachToComponent(RootCanvasSceneComp, FAttachmentTransformRules::KeepRelativeTransform);
				}
				else
				{
					SceneComponent->AttachToComponent(OverrideCanvasSceneComp, FAttachmentTransformRules::KeepRelativeTransform);
				}
			}
			else
			{
				SceneComponent->AttachToComponent(OverrideCanvasSceneComp, FAttachmentTransformRules::KeepRelativeTransform);
			}

			SceneComponent->SetRelativeTransform(FTransform::Identity);
			
			RenderProxy->SetupCanvas(Canvas, UIMeshBatchDesc.ProxyInfo, UIMeshBatchDesc.UIMeshBatchElement, UIMeshBatchDesc.BatchVerticesCount, UIMeshBatchDesc.BatchIndexCount, bRefreshRenderProxy);
		}
		else if (IsValid(Canvas))
		{
			RenderProxy->SetupVirtualWorldTransform(Canvas->bUseVirtualWorldTransform,
				Canvas->VirtualWorldTransform, Canvas->AttachTransform);
		}

		RenderProxy->SetRenderMode(TargetRenderMode, OverrideCanvas->GetViewExtension());
		RenderProxy->SetGraphicType(UIMeshBatchDesc.GraphicType);
		RenderProxy->SetGraphicData(UIMeshBatchDesc.GraphicData);
		RenderProxy->SetInstructionIndex(UIMeshBatchDesc.MinInstructionIndex, UIMeshBatchDesc.MaxInstructionIndex);
		RenderProxy->SetUIMaterial(RectTransform, UIMeshBatchDesc.BaseMeshMaterial, UIMeshBatchDesc.Texture, UIMeshBatchDesc.ClipRect,
			UIMeshBatchDesc.ClipSoftnessRect, UIMeshBatchDesc.bRectClipping, UIMeshBatchDesc.bTextElement, UIMeshBatchDesc.bRefreshRenderProxy || bRefreshRenderProxy);

#if USE_CAMERA_CULLING_MASK
		if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(UIRenderProxy))
		{
			PrimitiveComp->SetLayerMask(UIMeshBatchDesc.LayerMask, false);    
		}
#endif
	}

	UIMeshBatchDescStorage.Reset();
	
	const int32 NewBatchNum = UIMeshStorage.UIMeshList.Num();
	if (OldBatchNum == 0 && NewBatchNum > 0 || OldBatchNum > 0 && NewBatchNum == 0)
	{
		if (!ThisCanvas->IsOverrideSorting())
		{
			const auto ParentCanvas = ThisCanvas->GetParentCanvas();
			if (IsValid(ParentCanvas))
			{
				ParentCanvas->CanvasData.UpdateBatchDirtyCanvases.Add(ThisCanvas);
			}
		}
	}
}

void FRendererCanvasTree::UpdateDirtyRenderers()
{
	if (DirtyRenderProxies.Num() > 0)
	{
		for (auto& RenderProxy : DirtyRenderProxies)
		{
			if (RenderProxy.IsValid())
			{
				RenderProxy->UpdateDirtyRenderers();
			}
		}
		
		DirtyRenderProxies.Reset();
	}
}

bool FRendererCanvasTree::HasUIBatches(UCanvasSubComponent* Canvas)
{
	if (IsValid(Canvas) && Canvas->IsActiveAndEnabled() && Canvas->UIMeshStorage.UIMeshList.Num() > 0)
	{
		return true;
	}

	for (const auto& NestedCanvas : Canvas->GetCanvasData().NestedCanvases)
	{
		if (HasUIBatches(NestedCanvas))
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////
