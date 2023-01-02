#include "Core/Render/CanvasManager.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Render/LateUpdateInterface.h"
#include "UGUI.h"

struct FCompareCanvas
{
	FORCEINLINE bool operator()(const TWeakObjectPtr<UCanvasSubComponent>& WeakA, const TWeakObjectPtr<UCanvasSubComponent>& WeakB) const
	{
		if (!WeakA.IsValid() || !WeakB.IsValid())
		{
			return false;
		}
		
		const UCanvasSubComponent* A = WeakA.Get();
		const UCanvasSubComponent* B = WeakB.Get();

		if (A->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
			return true;

		if (B->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace)
			return false;
		
		const int32 ASortOrder = A->GetSortingOrder();
		const int32 BSortOrder = B->GetSortingOrder();

		if (ASortOrder != BSortOrder)
			return ASortOrder < BSortOrder;

		// If everything is the same sort on fallback sortID to ensure consistency every sort.
		return A->GetUniqueID() < B->GetUniqueID();
	}
};

TSharedPtr<FCanvasManager> FCanvasManager::Instance = nullptr;
TArray<TWeakObjectPtr<UObject>> FCanvasManager::InvalidObjects;

FCanvasManager::FCanvasManager()
{
	bUpdatingLatUpdateObjects = false;
}

void FCanvasManager::Initialize()
{
	if (Instance.IsValid())
	{
		return;
	}

	Instance = MakeShareable(new FCanvasManager());
}

void FCanvasManager::Shutdown()
{
	if (Instance.IsValid())
	{
		ensure(Instance.IsUnique());
		Instance.Reset();
	}
}

void FCanvasManager::AddCanvas(UCanvasSubComponent* CanvasComp)
{
	Initialize();
	Instance->InternalAddCanvas(CanvasComp);
}

void FCanvasManager::RemoveCanvas(UCanvasSubComponent* CanvasComp)
{
	Initialize();
	Instance->InternalRemoveCanvas(CanvasComp);
}

void FCanvasManager::SortCanvasList()
{
	Initialize();
	Instance->InternalSortCanvasList();
}

void FCanvasManager::InternalAddCanvas(UCanvasSubComponent* CanvasComp)
{
	if (!IsValid(CanvasComp))
		return;
	
	int32 InsertAt = CanvasList.Num();
	
	// This loop does two things:
	// 1) check if the canvas is already in m_List
	// 2) find where the canvas should be inserted in the list
	for (int32 Index = 0, Count = CanvasList.Num(); Index < Count; ++Index)
	{
		const UCanvasSubComponent* Canvas = Cast<UCanvasSubComponent>(CanvasList[Index].Get());

		if (!IsValid(Canvas))
		{
			continue;
		}
		
		if (Canvas == CanvasComp)
		{
			// Canvas already in list. Do not duplicate.
			return;
		}

		if (InsertAt == Count && (CanvasComp->GetSortingOrder() < Canvas->GetSortingOrder()))
		{
			InsertAt = Index;
		}
	}
	
	CanvasList.Insert(CanvasComp, InsertAt);

	CanvasComp->OnAddToCanvasManager();
}

void FCanvasManager::InternalRemoveCanvas(UCanvasSubComponent* CanvasComp)
{
	for (int32 Index = CanvasList.Num() - 1; Index >= 0; --Index)
	{
		if (CanvasComp == CanvasList[Index])
		{
			CanvasList.RemoveAt(Index, 1, false);

			if (IsValid(CanvasComp))
			{
				CanvasComp->OnRemoveFromCanvasManager();
			}
			
			break;
		}
	}
}

void FCanvasManager::InternalSortCanvasList()
{
	for (int32 Index = CanvasList.Num() - 1; Index >= 0; --Index)
	{
		if (!CanvasList[Index].IsValid())
		{
			CanvasList.RemoveAt(Index, 1, false);
			break;
		}
	}

	CanvasList.Sort(FCompareCanvas());

	for (int32 Index = Instance->CanvasList.Num() - 1; Index >= 0; --Index)
	{
		const auto CanvasComp = Cast<UCanvasSubComponent>(Instance->CanvasList[Index].Get());
		if (!IsValid(CanvasComp))
		{
			Instance->CanvasList.RemoveAt(Index, 1, false);
		}
		else
		{
			CanvasComp->OnSortInCanvasManager(Index);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- UpdateCanvases"), STAT_UnrealGUI_WillRenderCanvases, STATGROUP_UnrealGUI);
void FCanvasManager::WillRenderCanvases()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_WillRenderCanvases);
	
	Initialize();

	bool bRemoveInvalidCanvas = false;
	int32 StartPriority = 1;
	
	for (int32 Index = 0, Count = Instance->CanvasList.Num(); Index < Count; ++Index)
	{
		const auto CanvasComp = Cast<UCanvasSubComponent>(Instance->CanvasList[Index].Get());
		if (IsValid(CanvasComp))
		{
			StartPriority = CanvasComp->UpdateBatches(false, StartPriority);
			StartPriority = FMath::Max(StartPriority, (Index + 1) * 150);
		}
		else
		{
			bRemoveInvalidCanvas = true;
		}
	}

	if (bRemoveInvalidCanvas)
	{
		for (int32 Index = Instance->CanvasList.Num() - 1; Index >= 0; --Index)
		{
			const auto CanvasComp = Cast<UCanvasSubComponent>(Instance->CanvasList[Index].Get());
			if (!IsValid(CanvasComp))
			{
				Instance->CanvasList.RemoveAt(Index, 1, false);
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIBase --- RefreshLateUpdateObjects"), STAT_UnrealGUI_RefreshLateUpdateObjects, STATGROUP_UnrealGUI);
void FCanvasManager::RefreshLateUpdateObjects()
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_RefreshLateUpdateObjects);
	
	Initialize();

	Instance->bUpdatingLatUpdateObjects = true;

	for (auto& Elem : Instance->LateUpdateObjects)
	{
		if (Elem.IsValid())
		{
			const auto LateUpdateObj = Cast<ILateUpdateInterface>(Elem.Get());
			if (LateUpdateObj)
			{
				LateUpdateObj->LateUpdate();
			}
		}
		else
		{
			InvalidObjects.Add(Elem);
		}
	}

	if (InvalidObjects.Num() > 0)
	{
		for (const auto& Elem : InvalidObjects)
		{
			Instance->LateUpdateObjects.Remove(Elem);
		}
		
		InvalidObjects.Reset();
	}

	Instance->bUpdatingLatUpdateObjects = false;

	if (Instance->PendingAddLateUpdateObjects.Num() > 0)
	{
		for (auto& Elem : Instance->PendingAddLateUpdateObjects)
		{
			if (Elem.IsValid())
			{
				Instance->LateUpdateObjects.Add(Elem);
				Instance->PendingRemoveLateUpdateObjects.Remove(Elem);
			}
		}
		
		Instance->PendingAddLateUpdateObjects.Empty();
	}

	if (Instance->PendingRemoveLateUpdateObjects.Num() > 0)
	{
		for (auto& Elem : Instance->PendingRemoveLateUpdateObjects)
		{
			if (Elem.IsValid())
			{
				Instance->LateUpdateObjects.Remove(Elem);
			}
		}
		
		Instance->PendingRemoveLateUpdateObjects.Empty();
	}
}

void FCanvasManager::AddLateUpdateObject(ILateUpdateInterface* LateUpdateObj)
{
	Initialize();
	Instance->InternalAddLateUpdateObject(LateUpdateObj);
}

void FCanvasManager::RemoveLateUpdateObject(ILateUpdateInterface* LateUpdateObj)
{
	Initialize();
	Instance->InternalRemoveLateUpdateObject(LateUpdateObj);
}

void FCanvasManager::InternalAddLateUpdateObject(ILateUpdateInterface* LateUpdateObj)
{
	if (bUpdatingLatUpdateObjects)
	{
		const auto Obj = Cast<UObject>(LateUpdateObj);
		if (IsValid(Obj))
		{
			PendingAddLateUpdateObjects.Add(Obj);
		}
	}
	else
	{
		const auto Obj = Cast<UObject>(LateUpdateObj);
		if (IsValid(Obj))
		{
			LateUpdateObjects.Add(Obj);
		}
	}
}

void FCanvasManager::InternalRemoveLateUpdateObject(ILateUpdateInterface* LateUpdateObj)
{
	if (bUpdatingLatUpdateObjects)
	{
		const auto Obj = Cast<UObject>(LateUpdateObj);
		if (IsValid(Obj))
		{
			PendingAddLateUpdateObjects.Remove(Obj);
			PendingRemoveLateUpdateObjects.Add(Obj);
		}
	}
	else
	{
		LateUpdateObjects.Remove(Cast<UObject>(LateUpdateObj));
	}
}

int32 FCanvasManager::GetRenderOrder(const UCanvasSubComponent* CanvasComp)
{
	Initialize();

	const int32 Index = Instance->CanvasList.IndexOfByKey(CanvasComp);
	if (Index == INDEX_NONE)
	{
		return -Instance->CanvasList.Num();
	}
	
	return -Index;
}
