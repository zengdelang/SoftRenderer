#include "Core/Render/UIMeshBatchDescStorage.h"
#include "Core/Renderer/UIMeshProxyComponent.h"
#include "Core/Render/UIMeshBatchElement.h"
#include "UGUISettings.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// FUIMeshBatchDesc

DECLARE_CYCLE_STAT(TEXT("UICanvas --- Intersect"), STAT_UnrealGUI_Intersect, STATGROUP_UnrealGUI);
bool FUIMeshBatchDesc::Intersect(const FBox2D& RendererBoundBox) const
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Intersect);

	for (const auto& Box : SingleBoundBoxes)
	{
		if (Box.bIsValid && Box.Intersect(RendererBoundBox))
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////
// FUIMeshBatchDescStorage

DECLARE_CYCLE_STAT(TEXT("UICanvas --- CalculateRendererBoundBox"), STAT_UnrealGUI_CalculateRendererBoundBox, STATGROUP_UnrealGUI);
void FUIMeshBatchDescStorage::CalculateRendererBoundBox(const UCanvasRendererSubComponent* Handle, const FTransform& WorldToCanvasTransform,
	FBox2D& RendererBoundBox) const
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_CalculateRendererBoundBox);

	FVector WorldCorners[4];
	Handle->AttachTransform->GetWorldCorners(WorldCorners);

	FVector2D Min(1e8, 1e8);
	FVector2D Max(-1e8, -1e8);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector CanvasSpacePoint = WorldToCanvasTransform.TransformPosition(WorldCorners[Index]);

		Min.X = FMath::Min(Min.X, CanvasSpacePoint.X);
		Min.Y = FMath::Min(Min.Y, CanvasSpacePoint.Y);

		Max.X = FMath::Max(Max.X, CanvasSpacePoint.X);
		Max.Y = FMath::Max(Max.Y, CanvasSpacePoint.Y);
	}

	RendererBoundBox.Min = Min;
	RendererBoundBox.Max = Max;
	RendererBoundBox.bIsValid = true;
}

DECLARE_CYCLE_STAT(TEXT("UICanvas --- DoMergeCanvasRenderer"), STAT_UnrealGUI_DoMergeCanvasRenderer, STATGROUP_UnrealGUI);
void FUIMeshBatchDescStorage::DoMergeCanvasRenderer(UCanvasSubComponent* Canvas, UCanvasRendererSubComponent* Handle, int32 InstructionIndex, const int32& Section, const FTransform& WorldToCanvasTransform, FUIMeshStorage& UIMeshStorage)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_DoMergeCanvasRenderer);

	FUIMeshBatchDesc* CurUIMeshBatchDesc = nullptr;
	int32 CurBatchDescIndex = -1;
	
	bool bCanMerge = true;
	if (UIMeshBatchDescList.Num() > 0)
	{
		const auto& UIMeshBatchDesc = UIMeshBatchDescList[UIMeshBatchDescList.Num() - 1];
		if (Section > UIMeshBatchDesc.Section)
		{
			bCanMerge = false;
		}
	}

	FBox2D RendererBoundBox;
	CalculateRendererBoundBox(Handle, WorldToCanvasTransform, RendererBoundBox);

	if (bCanMerge)
	{
		for (int32 Index = UIMeshBatchDescList.Num() - 1; Index >= 0; --Index)
		{
			auto& UIMeshBatchDesc = UIMeshBatchDescList[Index];
			if (UIMeshBatchDesc.CanMergeCanvasRenderer(Handle, Section))
			{
				CurBatchDescIndex = Index;
				CurUIMeshBatchDesc = &UIMeshBatchDesc;
				break;
			}

			if (UIMeshBatchDesc.Intersect(RendererBoundBox))
			{
				break;
			}
		}
	}

	if (CurUIMeshBatchDesc)
	{
		const USceneComponent* RendererSceneComponent = Cast<USceneComponent>(Handle->GetOuter());
		if (!IsValid(RendererSceneComponent))
			return;
		
		if (FUIRenderProxyInfo* RenderProxyInfo = Handle->OwnerRenderProxyInfo.IsValid() ? Handle->OwnerRenderProxyInfo.Pin().Get() : nullptr)
		{
			RenderProxyInfo->CanvasRenderers.Remove(Handle);

			if (UUIMeshProxyComponent* UIMeshProxyComp = RenderProxyInfo->OwnerRenderProxy.IsValid() ? RenderProxyInfo->OwnerRenderProxy.Get() : nullptr)
			{
				if (UIMeshProxyComp != CurUIMeshBatchDesc->UIRenderProxy)
				{
					if (UIMeshProxyComp->BatchDescIndex > -1)
					{
						UIMeshBatchDescList[UIMeshProxyComp->BatchDescIndex].bRefreshRenderProxy = true;
					}
					
					if (CurUIMeshBatchDesc->UIRenderProxy)
					{
						CurUIMeshBatchDesc->bRefreshRenderProxy = true;
					}
					else if (UIMeshProxyComp->BatchDescIndex <= -1)
					{
						if (UIMeshProxyComp->MaterialInterface == CurUIMeshBatchDesc->BaseMeshMaterial)
						{
							CurUIMeshBatchDesc->UIRenderProxy = UIMeshProxyComp;
							UIMeshProxyComp->BatchDescIndex = CurBatchDescIndex;
						}
					}
				}
			}
			else
			{
				if (CurUIMeshBatchDesc->UIRenderProxy)
				{
					CurUIMeshBatchDesc->bRefreshRenderProxy = true;
				}
			}
		}
		else
		{
			if (CurUIMeshBatchDesc->UIRenderProxy)
			{
				CurUIMeshBatchDesc->bRefreshRenderProxy = true;
			}
		}

		CurUIMeshBatchDesc->bRefreshRenderProxy = Handle->IsRefreshRenderProxy() || CurUIMeshBatchDesc->bRefreshRenderProxy;
		
		CurUIMeshBatchDesc->SingleBoundBoxes.Emplace(RendererBoundBox);
		CurUIMeshBatchDesc->UpdateInstructionIndex(InstructionIndex);
		
		Handle->OwnerRenderProxyInfo = CurUIMeshBatchDesc->ProxyInfo;
		CurUIMeshBatchDesc->ProxyInfo->CanvasRenderers.Add(Handle, CurUIMeshBatchDesc->UIMeshBatchElement->UIMeshBatchSections.Num());
		
		const auto& RendererMesh = Handle->GetMesh();
		CurUIMeshBatchDesc->UIMeshBatchElement->UIMeshBatchSections.Emplace(FUIMeshBatchSection(Handle->Color, Handle->InheritedAlpha,
			RendererSceneComponent->GetComponentTransform(), RendererMesh, CurUIMeshBatchDesc->BatchVerticesCount));
		CurUIMeshBatchDesc->BatchVerticesCount += RendererMesh->GetCurrentVerticesCount();
		CurUIMeshBatchDesc->BatchIndexCount += RendererMesh->GetCurrentIndexCount();
	}
	else
	{
		FUIMeshBatchDesc UIMeshBatchDesc;
		
		UIMeshBatchDesc.Texture = Handle->Texture;
		UIMeshBatchDesc.Material = Handle->Material;
		UIMeshBatchDesc.ClipRect = Handle->ClipRect;
		UIMeshBatchDesc.ClipSoftnessRect = Handle->ClipSoftnessRect;
		UIMeshBatchDesc.GraphicType = Handle->GraphicType;
		UIMeshBatchDesc.bRectClipping = Handle->bRectClipping;
		UIMeshBatchDesc.bUseAntiAliasing = Handle->IsAntiAliasing();
		UIMeshBatchDesc.bTextElement = Handle->bTextElement;
		
		UIMeshBatchDesc.BaseMeshMaterial = UIMeshBatchDesc.Material != nullptr ? UIMeshBatchDesc.Material :
			Canvas->GetDefaultMaterialForUIMesh(UIMeshBatchDesc.bUseAntiAliasing, UIMeshBatchDesc.bTextElement);

		if (Handle->GraphicData.IsValid())
		{
			UIMeshBatchDesc.GraphicData = MakeShareable(Handle->GraphicData.Pin()->CopyGraphicData());
		}
		
		UIMeshBatchDesc.Section = Section;
		UIMeshBatchDesc.SingleBoundBoxes.Emplace(RendererBoundBox);
		UIMeshBatchDesc.UpdateInstructionIndex(InstructionIndex);

#if USE_CAMERA_CULLING_MASK
		uint32 HandleLayerMask = -1;
		if (IsValid(Handle->AttachTransform))
		{
			if (const auto HandleActor = Handle->AttachTransform->GetOwner())
			{
				HandleLayerMask = HandleActor->LayerMask;
			}
		}

		UIMeshBatchDesc.LayerMask = HandleLayerMask;
#endif
		
		UIMeshBatchDesc.bRefreshRenderProxy = Handle->IsRefreshRenderProxy() || UIMeshBatchDesc.bRefreshRenderProxy;

		UIMeshBatchDesc.bIsExternalRenderProxy = Handle->GetUseCustomRenderProxy();
		if (UIMeshBatchDesc.bIsExternalRenderProxy)
		{
			USceneComponent* CustomRenderProxyComp = Handle->CustomRenderProxyComponent.Get();
			UIMeshBatchDesc.UIRenderProxy = Cast<IUIRenderProxyInterface>(CustomRenderProxyComp);
			UIMeshStorage.UIMeshList.Remove(CustomRenderProxyComp);
			UIMeshBatchDesc.bRefreshRenderProxy = true;
		}
		else
		{
			const USceneComponent* RendererSceneComponent = Cast<USceneComponent>(Handle->GetOuter());
			if (!IsValid(RendererSceneComponent))
				return;
			
			UIMeshBatchDesc.UIMeshBatchElement = MakeShareable(new FUIMeshBatchElement);
			UIMeshBatchDesc.ProxyInfo = MakeShareable(new FUIRenderProxyInfo);
			
			if (FUIRenderProxyInfo* RenderProxyInfo = Handle->OwnerRenderProxyInfo.IsValid() ? Handle->OwnerRenderProxyInfo.Pin().Get() : nullptr)
			{
				RenderProxyInfo->CanvasRenderers.Remove(Handle);

				if (UUIMeshProxyComponent* UIMeshProxyComp = RenderProxyInfo->OwnerRenderProxy.IsValid() ? RenderProxyInfo->OwnerRenderProxy.Get() : nullptr)
				{
					if (UIMeshProxyComp->BatchDescIndex > -1)
					{
						UIMeshBatchDescList[UIMeshProxyComp->BatchDescIndex].bRefreshRenderProxy = true;
					}
					else 
					{
						if (UIMeshProxyComp->MaterialInterface == UIMeshBatchDesc.BaseMeshMaterial)
						{
							UIMeshBatchDesc.UIRenderProxy = UIMeshProxyComp;
						}
					}
				}
			}

			Handle->OwnerRenderProxyInfo = UIMeshBatchDesc.ProxyInfo;
			UIMeshBatchDesc.ProxyInfo->CanvasRenderers.Add(Handle, 0);
	
			const auto& RendererMesh = Handle->GetMesh();
			UIMeshBatchDesc.UIMeshBatchElement->UIMeshBatchSections.Emplace(FUIMeshBatchSection(Handle->Color, Handle->InheritedAlpha,
				RendererSceneComponent->GetComponentTransform(), RendererMesh, 0));
			UIMeshBatchDesc.BatchVerticesCount += RendererMesh->GetCurrentVerticesCount();
			UIMeshBatchDesc.BatchIndexCount += RendererMesh->GetCurrentIndexCount();
		}

		if (UIMeshBatchDesc.UIRenderProxy)
		{
			UIMeshBatchDesc.UIRenderProxy->BatchDescIndex = UIMeshBatchDescList.Num();
		}
		else
		{
			UIMeshBatchDesc.bRefreshRenderProxy = true;
		}
		
		UIMeshBatchDescList.Emplace(UIMeshBatchDesc);
	}
}

/////////////////////////////////////////////////////
