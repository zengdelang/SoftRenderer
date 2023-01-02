#include "Core/Renderer/UICanvasPrimitiveBuffer.h"

/////////////////////////////////////////////////////
// FUICanvasPrimitiveBuffer

FUICanvasPrimitiveBuffer::FUICanvasPrimitiveBuffer()
	: CanvasLocalToWorld(FMatrix::Identity)
{
}

FUICanvasPrimitiveBuffer::~FUICanvasPrimitiveBuffer()
{

}

void FUICanvasPrimitiveBuffer::Update(const FMatrix& NewCanvasLocalToWorld)
{
	check(IsInRenderingThread());
	
	if (!CanvasPrimitiveUniformBuffer.IsValid() || !CanvasLocalToWorld.Equals(NewCanvasLocalToWorld))
	{
		CanvasLocalToWorld = NewCanvasLocalToWorld;

		if (!IsInitialized())
		{
			InitResource();
		}
		
		if (CanvasPrimitiveUniformBuffer.IsValid())
		{
			CanvasPrimitiveUniformBuffer->ReleaseResource();
			CanvasPrimitiveUniformBuffer.Reset();
		}
		
		CanvasPrimitiveUniformBuffer = MakeShareable(new TUniformBuffer<FPrimitiveUniformShaderParameters>());
		CanvasPrimitiveUniformBuffer->SetContents(
			GetPrimitiveUniformShaderParameters(CanvasLocalToWorld, CanvasLocalToWorld, FVector::ZeroVector, FBoxSphereBounds(), FBoxSphereBounds(), FBoxSphereBounds(),
				false, false, false, false, false, false,
				GetDefaultLightingChannelMask(), 1.0f, INDEX_NONE, INDEX_NONE, false, nullptr));
		CanvasPrimitiveUniformBuffer->InitResource();
	}
}

void FUICanvasPrimitiveBuffer::CleanUp()
{
	BeginReleaseResource(this);
	BeginCleanup(this);
}

void FUICanvasPrimitiveBuffer::InitDynamicRHI()
{
	
}

void FUICanvasPrimitiveBuffer::ReleaseDynamicRHI()
{
	CanvasLocalToWorld = FMatrix::Identity;

	if (CanvasPrimitiveUniformBuffer.IsValid())
	{
		CanvasPrimitiveUniformBuffer->ReleaseResource();
		CanvasPrimitiveUniformBuffer.Reset();
	}

	for (const auto& PrimitiveUniformBufferElem : PrimitiveUniformBufferMap)
	{
		if (PrimitiveUniformBufferElem.Value.PrimitiveUniformBuffer != nullptr)
		{
			PrimitiveUniformBufferElem.Value.PrimitiveUniformBuffer->ReleaseResource();
		}
	}
	PrimitiveUniformBufferMap.Empty();
}

void FUICanvasPrimitiveBuffer::AddPrimitiveUniformBufferCount(uint32 TransformId, FMatrix VirtualWorldTransform)
{
	FPrimitiveBufferInfo* PrimitiveBufferInfoPtr = PrimitiveUniformBufferMap.Find(TransformId);
	if (PrimitiveBufferInfoPtr)
	{
		++PrimitiveBufferInfoPtr->Count;
		
		if (!PrimitiveBufferInfoPtr->VirtualWorldTransform.Equals(VirtualWorldTransform))
		{
			if (!IsInitialized())
			{
				InitResource();
			}
			
			PrimitiveBufferInfoPtr->VirtualWorldTransform = VirtualWorldTransform;
		
			if (PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.IsValid())
			{
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->ReleaseResource();
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.Reset();
			}

			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer = MakeShareable(new TUniformBuffer<FPrimitiveUniformShaderParameters>());
			
			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->SetContents(
				GetPrimitiveUniformShaderParameters(VirtualWorldTransform, VirtualWorldTransform, FVector::ZeroVector, FBoxSphereBounds(), FBoxSphereBounds(), FBoxSphereBounds(),
			false, false, false, false, false, false,
					GetDefaultLightingChannelMask(), 1.0f, INDEX_NONE, INDEX_NONE, false, nullptr));
			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->InitResource();
		}
	}
	else
	{
		if (!IsInitialized())
		{
			InitResource();
		}
		
		FPrimitiveBufferInfo PrimitiveBufferInfo;
		++PrimitiveBufferInfo.Count;
		
		PrimitiveBufferInfo.VirtualWorldTransform = VirtualWorldTransform;

		PrimitiveBufferInfo.PrimitiveUniformBuffer = MakeShareable(new TUniformBuffer<FPrimitiveUniformShaderParameters>());
		
		PrimitiveBufferInfo.PrimitiveUniformBuffer->SetContents(
			GetPrimitiveUniformShaderParameters(VirtualWorldTransform, VirtualWorldTransform, FVector::ZeroVector, FBoxSphereBounds(), FBoxSphereBounds(), FBoxSphereBounds(),
				false, false, false, false, false, false,
				GetDefaultLightingChannelMask(), 1.0f, INDEX_NONE, INDEX_NONE, false, nullptr));
		PrimitiveBufferInfo.PrimitiveUniformBuffer->InitResource();
 
		PrimitiveUniformBufferMap.Emplace(TransformId, PrimitiveBufferInfo);
	}
}

void FUICanvasPrimitiveBuffer::RemovePrimitiveUniformBuffer(uint32 TransformId)
{
	FPrimitiveBufferInfo* PrimitiveBufferInfoPtr = PrimitiveUniformBufferMap.Find(TransformId);
	if (PrimitiveBufferInfoPtr)
	{
		--PrimitiveBufferInfoPtr->Count;

		if (PrimitiveBufferInfoPtr->Count <= 0)
		{
			if (PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.IsValid())
			{
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->ReleaseResource();
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.Reset();
			}
			
			PrimitiveUniformBufferMap.Remove(TransformId);
		}
	}
}

void FUICanvasPrimitiveBuffer::UpdatePrimitiveUniformBuffer(uint32 TransformId, FMatrix VirtualWorldTransform)
{
	FPrimitiveBufferInfo* PrimitiveBufferInfoPtr = PrimitiveUniformBufferMap.Find(TransformId);
	if (PrimitiveBufferInfoPtr)
	{
		if (!PrimitiveBufferInfoPtr->VirtualWorldTransform.Equals(VirtualWorldTransform))
		{
			if (!IsInitialized())
			{
				InitResource();
			}
			
			PrimitiveBufferInfoPtr->VirtualWorldTransform = VirtualWorldTransform;

			if (PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.IsValid())
			{
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->ReleaseResource();
				PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.Reset();
			}

			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer = MakeShareable(new TUniformBuffer<FPrimitiveUniformShaderParameters>());

			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->SetContents(
				GetPrimitiveUniformShaderParameters(VirtualWorldTransform, VirtualWorldTransform, FVector::ZeroVector, FBoxSphereBounds(), FBoxSphereBounds(), FBoxSphereBounds(),
			false, false, false, false, false, false,
					GetDefaultLightingChannelMask(), 1.0f, INDEX_NONE, INDEX_NONE, false, nullptr));
			PrimitiveBufferInfoPtr->PrimitiveUniformBuffer->InitResource();
		}
	}
	else
	{
		if (!IsInitialized())
		{
			InitResource();
		}
		
		FPrimitiveBufferInfo PrimitiveBufferInfo;
		++PrimitiveBufferInfo.Count;
		
		PrimitiveBufferInfo.VirtualWorldTransform = VirtualWorldTransform;

		PrimitiveBufferInfo.PrimitiveUniformBuffer = MakeShareable(new TUniformBuffer<FPrimitiveUniformShaderParameters>());
		
		PrimitiveBufferInfo.PrimitiveUniformBuffer->SetContents(
			GetPrimitiveUniformShaderParameters(VirtualWorldTransform, VirtualWorldTransform, FVector::ZeroVector, FBoxSphereBounds(), FBoxSphereBounds(), FBoxSphereBounds(),
				false, false, false, false, false, false,
				GetDefaultLightingChannelMask(), 1.0f, INDEX_NONE, INDEX_NONE, false, nullptr));
		PrimitiveBufferInfo.PrimitiveUniformBuffer->InitResource();
 
		PrimitiveUniformBufferMap.Emplace(TransformId, PrimitiveBufferInfo);
	}
}

TUniformBuffer<FPrimitiveUniformShaderParameters>* FUICanvasPrimitiveBuffer::GetPrimitiveUniformBuffer(uint32 TransformId)
{
	const FPrimitiveBufferInfo* PrimitiveBufferInfoPtr = PrimitiveUniformBufferMap.Find(TransformId);
	if (PrimitiveBufferInfoPtr)
	{
		return PrimitiveBufferInfoPtr->PrimitiveUniformBuffer.Get();
	}
	return nullptr;
}

const FMatrix& FUICanvasPrimitiveBuffer::GetVirtualWorldTransform(uint32 TransformId)
{
	const FPrimitiveBufferInfo* PrimitiveBufferInfoPtr = PrimitiveUniformBufferMap.Find(TransformId);
	if (PrimitiveBufferInfoPtr)
	{
		return PrimitiveBufferInfoPtr->VirtualWorldTransform;
	}
	return CanvasLocalToWorld;
}

/////////////////////////////////////////////////////
