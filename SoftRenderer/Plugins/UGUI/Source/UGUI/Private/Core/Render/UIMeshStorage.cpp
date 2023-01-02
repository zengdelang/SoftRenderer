#include "Core/Render/UIMeshStorage.h"
#include "UGUIWorldSubsystem.h"
#include "Core/Renderer/UIMeshProxyComponent.h"
#include "Core/Render/CanvasSubComponent.h"

bool FUIMeshStorage::CanClearMesh(const UCanvasSubComponent* Canvas)
{
	if (UIMeshList.Num() > 0)
	{
		if (const auto OverrideCanvas = Canvas)
		{
			if (const USceneComponent* SceneComponent = Cast<USceneComponent>(UIMeshList[0]))
			{
				const auto OverrideCanvasSceneComp = Cast<USceneComponent>(OverrideCanvas->GetOuter());
				if (OverrideCanvasSceneComp == SceneComponent->GetAttachParent())
				{
					return false;
				}
			}
		}
	}
	
	return false;
}

void FUIMeshStorage::CacheAllUIMesh()
{
	for (int32 Index = UIMeshList.Num() - 1; Index >= 0; --Index)
	{
		if (const auto UIMesh = Cast<USceneComponent>(UIMeshList[Index]))
		{
			if (IUIRenderProxyInterface* RenderProxy = Cast<IUIRenderProxyInterface>(UIMesh))
			{
				RenderProxy->ClearUIMesh();
			}
			else
			{
				UIMesh->DestroyComponent();
			}
		}
	}
	
	UIMeshList.Empty();
}

int32 FUIMeshStorage::UpdateMeshComponentsPriority(UCanvasSubComponent* Canvas, int32& StartPriority, int32& PriorityCount)
{
	if (!IsValid(Canvas))
		return 0;

	int32 Index = 0;
	int32 CanvasIndex = 0;
	for (const int32 Count = UIMeshList.Num(); Index < Count; ++Index)
	{
		IUIRenderProxyInterface* RenderProxy = Cast<IUIRenderProxyInterface>(UIMeshList[Index]);
		if (RenderProxy)
		{
			const int32 MinInstructionIndex = RenderProxy->GetMinInstructionIndex();
			const int32 MaxInstructionIndex = RenderProxy->GetMaxInstructionIndex();

			const auto& NestedCanvases = Canvas->CanvasData.GetNestedCanvases();
			for (const int32 CanvasCount = NestedCanvases.Num(); CanvasIndex < CanvasCount;)
			{
				const auto& NestedCanvas = NestedCanvases[CanvasIndex];
				if (IsValid(NestedCanvas))
				{
					if (MaxInstructionIndex < NestedCanvas->GetCanvasInstructionIndex())
					{
						break;
					}

					if (MinInstructionIndex < NestedCanvas->GetCanvasInstructionIndex())
					{
						break;
					}

					NestedCanvas->UpdateCanvasMeshComponentsPriority(StartPriority, PriorityCount);
					++CanvasIndex;
				}
			}

			RenderProxy->UpdateTranslucentSortPriority(StartPriority);

			++StartPriority;
			++PriorityCount;
		}
	}
	
	return CanvasIndex;
}
