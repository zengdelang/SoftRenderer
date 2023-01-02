#pragma once

#include "CoreMinimal.h"
#include "UIMeshStorage.h"
#include "RendererCanvasTree.generated.h"

class UCanvasRendererSubComponent;
class UCanvasSubComponent;

USTRUCT()
struct FRendererCanvasTreeNode
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	USceneComponent* TreeComponent;

	UPROPERTY(Transient)
	UCanvasRendererSubComponent* Renderer;

	UPROPERTY(Transient)
	UCanvasSubComponent* Canvas;
	
	int32 Parent;
	
	TArray<int32> Children;

	int32 Next;
	int32 Prev;

public:
	FRendererCanvasTreeNode()
		: TreeComponent(nullptr)
		, Renderer(nullptr)
		, Canvas(nullptr)
		, Parent(-1)
		, Next(-1)
		, Prev(-1)
	{
	}
};

struct FRendererCanvasChainNode
{
public:
	int32 NodeIndex;

	int32 Prev;
	int32 Next;

public:
	FRendererCanvasChainNode(): NodeIndex(0), Prev(-1), Next(-1)
	{
	}

	FRendererCanvasChainNode(int32 InNodeIndex, int32 InPrev, int32 InNext) : NodeIndex(InNodeIndex), Prev(InPrev), Next(InNext)
	{
	}
};

USTRUCT()
struct FRendererCanvasTree
{
	GENERATED_BODY()

	friend class UCanvasSubComponent;
	friend class UCanvasRendererSubComponent;
	
protected:
	UPROPERTY(Transient)
	TMap<USceneComponent*, int32> NodeMap;

	UPROPERTY(Transient)
	TArray<FRendererCanvasTreeNode> TreeNodes;

	UPROPERTY(Transient)
	TSet<UCanvasRendererSubComponent*> PendingAddRenderers;

	UPROPERTY(Transient)
	TSet<UCanvasSubComponent*> PendingAddCanvases;

	UPROPERTY(Transient)
	TSet<UCanvasSubComponent*> UpdateBatchDirtyCanvases;

	UPROPERTY(Transient)
	UCanvasSubComponent* ThisCanvas;
	
	UPROPERTY(Transient)
	TMap<UObject*, int32> RendererCanvasMap;
	
	UPROPERTY(Transient)
	TArray<UCanvasSubComponent*> NestedCanvases;

	TSet<TWeakObjectPtr<UUIMeshProxyComponent>> DirtyRenderProxies;
	
	TArray<FRendererCanvasChainNode> RendererCanvasChain;
	
	int32 FreeListIndex;
	
	int32 FirstChainIndex;
	int32 FreeChainListIndex;

	int32 NestedCanvasNum;
	int32 MaxDepth;
	
public:
	uint32 DirtyFlag;
	
public:
	void AddRenderer(UCanvasRendererSubComponent* Renderer);
	void RemoveRenderer(const UCanvasRendererSubComponent* Renderer);

	void AddCanvas(UCanvasSubComponent* Canvas);
	void RemoveCanvas(UCanvasSubComponent* Canvas);

	void NotifyCanvasStateChanged(UCanvasSubComponent* Canvas, bool bShow);
	
	void UpdateTree();
	void UpdateBatchDirty();
	void UpdateDepth();

	FORCEINLINE void Empty()
	{
		FreeListIndex = -1;
		FirstChainIndex = -1;
		FreeChainListIndex = -1;
		NestedCanvasNum = 0;
		MaxDepth = 20000;
		DirtyFlag = 0;

		NodeMap.Empty();
		TreeNodes.Empty();
		PendingAddRenderers.Empty();
		PendingAddCanvases.Empty();
		UpdateBatchDirtyCanvases.Empty();
		RendererCanvasMap.Empty();
		NestedCanvases.Empty();
		RendererCanvasChain.Empty();
		DirtyRenderProxies.Empty();
	}

	FORCEINLINE const TArray<UCanvasSubComponent*>& GetNestedCanvases()
	{
		return NestedCanvases;
	}

protected:
	bool CheckBatchDirty(UCanvasSubComponent* Canvas);
	
	void UpdateRootComponent(UCanvasSubComponent* InThisCanvas, USceneComponent* RootComp)
	{
		ThisCanvas = InThisCanvas;
		FRendererCanvasTreeNode RootNode;
		RootNode.TreeComponent = RootComp;
		TreeNodes.Emplace(RootNode);
		NodeMap.Emplace(RootComp, 0);
	}
	
	void AddRendererNode(UCanvasRendererSubComponent* Renderer);
	void AddCanvasNode(UCanvasSubComponent* Canvas);
	void UpdateChainNode(int32 NodeIndex, int32 Prev, UObject* Obj);
	
	int32 AddNode(USceneComponent* SceneComp);
	int32 CreateNode();
	void RemoveNode(int32 NodeIndex);

	int32 CreateChainNode();

	void MergeCanvasRenderer(UCanvasSubComponent* Canvas, UCanvasSubComponent* OverrideCanvas, const UCanvasSubComponent* RootCanvas,
		USceneComponent* OverrideCanvasSceneComp, const USceneComponent* RectTransform, const FTransform& WorldToCanvasTransform);

	void UpdateDirtyRenderers();

	static bool HasUIBatches(UCanvasSubComponent* Canvas);
	
public:
	FRendererCanvasTree() : ThisCanvas(nullptr), FreeListIndex(-1), FirstChainIndex(-1), FreeChainListIndex(-1), NestedCanvasNum(0), MaxDepth(20000),
	                        DirtyFlag(0)
	{
	}
};
