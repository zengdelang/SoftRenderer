#pragma once

#include "CoreMinimal.h"
#include "CanvasRendererSubComponent.h"

class FUIMeshBatchDesc
{
public:
	TSharedPtr<FUIMeshBatchElement> UIMeshBatchElement;

	FTransform WorldToCanvasTransform;
	
	int32 BatchVerticesCount;
	int32 BatchIndexCount;
	
	TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> GraphicData;
	TSharedPtr<FUIRenderProxyInfo> ProxyInfo;
	
	UMaterialInterface* Material;
	UMaterialInterface* BaseMeshMaterial;
	UTexture* Texture;
	
	IUIRenderProxyInterface* UIRenderProxy;
	TArray<FBox2D> SingleBoundBoxes;
	
	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;

	int32 MinInstructionIndex;
	int32 MaxInstructionIndex;
	
	int32 Section;

#if USE_CAMERA_CULLING_MASK
	int32 LayerMask;
#endif
	
	EUIGraphicType GraphicType;

	uint8 bRectClipping : 1;
	uint8 bUseAntiAliasing : 1;
	uint8 bTextElement : 1;
	uint8 bIsExternalRenderProxy : 1;
	
	uint8 bRefreshRenderProxy : 1;

public:
	FUIMeshBatchDesc()
		: BatchVerticesCount(0)
		, BatchIndexCount(0)
		, Material(nullptr)
		, BaseMeshMaterial(nullptr)
		, Texture(nullptr)
		, UIRenderProxy(nullptr)
	    , MinInstructionIndex(INT32_MAX)
		, MaxInstructionIndex(-1)
		, Section(0)
#if USE_CAMERA_CULLING_MASK
		, LayerMask(0)
#endif
		, GraphicType(EUIGraphicType::UIMesh)
		, bRectClipping(false)
		, bUseAntiAliasing(false)
		, bTextElement(false)
		, bIsExternalRenderProxy(false)
		, bRefreshRenderProxy(false)
	{
		SingleBoundBoxes.Reserve(32);
	}

	FORCEINLINE bool CanMergeCanvasRenderer(const UCanvasRendererSubComponent* Handle, const int32& InSection) const
	{
		if (InSection > Section)
		{
			return false;
		}

		if (Handle->GraphicType != GraphicType || Handle->GraphicType != EUIGraphicType::UIMesh)
		{
			return false;
		}

		if (bTextElement != Handle->bTextElement)
		{
			return false;
		}

		if (Material != Handle->Material)
		{
			return false;
		}

		if (Texture != Handle->Texture)
		{
			return false;
		}

		if (bRectClipping != Handle->bRectClipping)
		{
			return false;
		}

		if (Handle->bRectClipping && (!ClipRect.Equals(Handle->ClipRect) || !ClipSoftnessRect.Equals(
			Handle->ClipSoftnessRect)))
		{
			return false;
		}

		if (bUseAntiAliasing != Handle->bAntiAliasing)
		{
			return false;
		}

		if (Handle->bUseCustomRenderProxy)
		{
			return false;
		}

		if (bIsExternalRenderProxy)
		{
			return false;
		}

#if USE_CAMERA_CULLING_MASK
		if (IsValid(Handle->AttachTransform))
		{
			if (const auto HandleActor = Handle->AttachTransform->GetOwner())
			{
				if (LayerMask != HandleActor->LayerMask)
				{
					return false;
				}
			}
		}
#endif

		return true;
	}
	
	bool Intersect(const FBox2D& RendererBoundBox) const;

	FORCEINLINE void UpdateInstructionIndex(int32 InstructionIndex)
	{
		if (InstructionIndex < MinInstructionIndex)
		{
			MinInstructionIndex = InstructionIndex;
		}

		if (InstructionIndex > MaxInstructionIndex)
		{
			MaxInstructionIndex = InstructionIndex;
		}
	}
};

class FUIMeshBatchDescStorage
{
public:
	TArray<FUIMeshBatchDesc> UIMeshBatchDescList;

public:
	FUIMeshBatchDescStorage()
	{
		UIMeshBatchDescList.Reserve(32);
	}
	
public:
	void Reset()
	{
		UIMeshBatchDescList.Reset();
	}
	
	void CalculateRendererBoundBox(const UCanvasRendererSubComponent* Handle, const FTransform& WorldToCanvasTransform, FBox2D& RendererBoundBox) const;
	
	void DoMergeCanvasRenderer(UCanvasSubComponent* Canvas, UCanvasRendererSubComponent* Handle, int32 InstructionIndex, const int32& Section, const FTransform& WorldToCanvasTransform, FUIMeshStorage& UIMeshStorage);
	
};
