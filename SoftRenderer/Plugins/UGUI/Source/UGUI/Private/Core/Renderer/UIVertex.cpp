#include "Core/Renderer/UIVertex.h"

/************************************************************************/
/* FUIScreenVertexDeclaration                                           */
/************************************************************************/

TAutoConsoleVariable<int32> CVarUsePooledBuffer(
	TEXT("UGUI.UsePooledBuffer"),
	1,
	TEXT("0 - Use immediate buffer.")
	TEXT("1 - Use pooled buffer."));

FVector FUIVertex::DefaultTangent = FVector(1.f, 0.f, 0.f);

FUIVertex FUIVertex::SimpleVertex = FUIVertex(FVector::ZeroVector, FColor::White, FVector2D::ZeroVector, FVector2D::ZeroVector,
	FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector, FVector::DownVector, FUIVertex::DefaultTangent);

void FUIScreenVertexDeclaration::InitRHI()
{
	FVertexDeclarationElementList Elements;
	
	constexpr uint32 Stride = sizeof(FUIVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, Position), VET_Float3, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, Color), VET_Color, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV0), VET_Float2, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV1), VET_Float2, 3, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV2), VET_Float2, 4, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV3), VET_Float2, 5, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV4), VET_Float2, 6, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV5), VET_Float2, 7, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV6), VET_Float2, 8, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FUIVertex, UV7), VET_Float2, 9, Stride));
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}

void FUIScreenVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}

TGlobalResource<FUIScreenVertexDeclaration> GUIScreenVertexDeclaration;

FVertexDeclarationRHIRef& GetUIScreenVertexDeclaration()
{
	return GUIScreenVertexDeclaration.VertexDeclarationRHI;
}

/************************************************************************/
/* FPooledDynamicUIMeshVertexBuffer                                     */
/************************************************************************/

class FGlobalDynamicUIMeshPoolPolicy
{
public:
	/** Buffers are created with a simple byte size */
	typedef uint32 CreationArguments;
	
	enum
	{
		NumSafeFrames = 1,       /** Number of frames to leaves buffers before reclaiming/reusing */
		NumPoolBucketSizes = 16, /** Number of pool buckets */
		NumToDrainPerFrame = 5,  /** Max. number of resources to cull in a single frame */
		CullAfterFramesNum = 30 * 60 * 10 /** Resources are culled if unused for more frames than this */
	};
	
	/** Get the pool bucket index from the size
	 * @param Size the number of bytes for the resource
	 * @returns The bucket index.
	 */
	uint32 GetPoolBucketIndex(uint32 Size)
	{
		unsigned long Lower = 0;
		unsigned long Upper = NumPoolBucketSizes;

		do
		{
			const unsigned long Middle = (Upper + Lower) >> 1;
			if(Size <= BucketSizes[Middle-1])
			{
				Upper = Middle;
			}
			else
			{
				Lower = Middle;
			}
		}
		while(Upper - Lower > 1);
		
		check(Size <= BucketSizes[Lower]);
		check((Lower == 0) || (Size > BucketSizes[Lower-1]));
		
		return Lower;
	}
	
	/** Get the pool bucket size from the index
	 * @param Bucket the bucket index
	 * @returns The bucket size.
	 */
	uint32 GetPoolBucketSize(uint32 Bucket)
	{
		check(Bucket < NumPoolBucketSizes);
		return BucketSizes[Bucket];
	}
	
private:
	/** The bucket sizes */
	static uint32 BucketSizes[NumPoolBucketSizes];
};

uint32 FGlobalDynamicUIMeshPoolPolicy::BucketSizes[NumPoolBucketSizes] = {
	64, 128, 256, 512, 1024, 2048, 4096,
	8*1024, 16*1024, 32*1024, 64*1024, 128*1024, 256*1024,
	512*1024, 1*1024*1024, 2*1024*1024
};

class FGlobalDynamicUIMeshIndexPolicy : public FGlobalDynamicUIMeshPoolPolicy
{
public:
	enum
	{
		NumSafeFrames = FGlobalDynamicUIMeshPoolPolicy::NumSafeFrames,
		NumPoolBuckets = FGlobalDynamicUIMeshPoolPolicy::NumPoolBucketSizes,
		NumToDrainPerFrame = FGlobalDynamicUIMeshPoolPolicy::NumToDrainPerFrame,
		CullAfterFramesNum = FGlobalDynamicUIMeshPoolPolicy::CullAfterFramesNum
	};
	
	/** Creates the resource
	 * @param Args The buffer size in bytes.
	 * @returns A suitably sized buffer or NULL on failure.
	 */
	FIndexBufferRHIRef CreateResource(FGlobalDynamicUIMeshPoolPolicy::CreationArguments Args)
	{
		FGlobalDynamicUIMeshPoolPolicy::CreationArguments BufferSize = GetPoolBucketSize(GetPoolBucketIndex(Args));
		// The use of BUF_Static is deliberate - on OS X the buffer backing-store orphaning & reallocation will dominate execution time
		// so to avoid this we don't reuse a buffer for several frames, thereby avoiding the pipeline stall and the reallocation cost.
		FRHIResourceCreateInfo CreateInfo;
		FIndexBufferRHIRef VertexBuffer = RHICreateIndexBuffer(sizeof(uint32), BufferSize, BUF_Static, CreateInfo);
		return VertexBuffer;
	}
	
	/** Gets the arguments used to create resource
	 * @param Resource The buffer to get data for.
	 * @returns The arguments used to create the buffer.
	 */
	FGlobalDynamicUIMeshPoolPolicy::CreationArguments GetCreationArguments(FIndexBufferRHIRef Resource)
	{
		return (Resource->GetSize());
	}
	
	/** Frees the resource
	 * @param Resource The buffer to prepare for release from the pool permanently.
	 */
	void FreeResource(FIndexBufferRHIRef Resource)
	{
	}
};

class FGlobalDynamicUIMeshIndexPool : public TRenderResourcePool<FIndexBufferRHIRef, FGlobalDynamicUIMeshIndexPolicy, FGlobalDynamicUIMeshPoolPolicy::CreationArguments>
{
public:
	/** Destructor */
	virtual ~FGlobalDynamicUIMeshIndexPool() override
	{
	}
	
public:
	// From FTickableObjectRenderThread
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGlobalDynamicUIMeshIndexPool, STATGROUP_Tickables);
	}
};

TGlobalResource<FGlobalDynamicUIMeshIndexPool> GDynamicUIMeshIndexPool;

class FGlobalDynamicUIMeshIndexPolicy16 : public FGlobalDynamicUIMeshPoolPolicy
{
public:
	enum
	{
		NumSafeFrames = FGlobalDynamicUIMeshPoolPolicy::NumSafeFrames,
		NumPoolBuckets = FGlobalDynamicUIMeshPoolPolicy::NumPoolBucketSizes,
		NumToDrainPerFrame = FGlobalDynamicUIMeshPoolPolicy::NumToDrainPerFrame,
		CullAfterFramesNum = FGlobalDynamicUIMeshPoolPolicy::CullAfterFramesNum
	};
	
	/** Creates the resource
	 * @param Args The buffer size in bytes.
	 * @returns A suitably sized buffer or NULL on failure.
	 */
	FIndexBufferRHIRef CreateResource(FGlobalDynamicUIMeshPoolPolicy::CreationArguments Args)
	{
		FGlobalDynamicUIMeshPoolPolicy::CreationArguments BufferSize = GetPoolBucketSize(GetPoolBucketIndex(Args));
		// The use of BUF_Static is deliberate - on OS X the buffer backing-store orphaning & reallocation will dominate execution time
		// so to avoid this we don't reuse a buffer for several frames, thereby avoiding the pipeline stall and the reallocation cost.
		FRHIResourceCreateInfo CreateInfo;
		FIndexBufferRHIRef VertexBuffer = RHICreateIndexBuffer(sizeof(uint16), BufferSize, BUF_Static, CreateInfo);
		return VertexBuffer;
	}
	
	/** Gets the arguments used to create resource
	 * @param Resource The buffer to get data for.
	 * @returns The arguments used to create the buffer.
	 */
	FGlobalDynamicUIMeshPoolPolicy::CreationArguments GetCreationArguments(FIndexBufferRHIRef Resource)
	{
		return (Resource->GetSize());
	}
	
	/** Frees the resource
	 * @param Resource The buffer to prepare for release from the pool permanently.
	 */
	void FreeResource(FIndexBufferRHIRef Resource)
	{
	}
};

class FGlobalDynamicUIMeshIndexPool16 : public TRenderResourcePool<FIndexBufferRHIRef, FGlobalDynamicUIMeshIndexPolicy16, FGlobalDynamicUIMeshPoolPolicy::CreationArguments>
{
public:
	/** Destructor */
	virtual ~FGlobalDynamicUIMeshIndexPool16() override
	{
	}
	
public:
	// From FTickableObjectRenderThread
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGlobalDynamicUIMeshIndexPool16, STATGROUP_Tickables);
	}
};

TGlobalResource<FGlobalDynamicUIMeshIndexPool16> GDynamicUIMeshIndexPool16;

class FGlobalDynamicUIMeshVertexPolicy : public FGlobalDynamicUIMeshPoolPolicy
{
public:
	enum
	{
		NumSafeFrames = FGlobalDynamicUIMeshPoolPolicy::NumSafeFrames,
		NumPoolBuckets = FGlobalDynamicUIMeshPoolPolicy::NumPoolBucketSizes,
		NumToDrainPerFrame = FGlobalDynamicUIMeshPoolPolicy::NumToDrainPerFrame,
		CullAfterFramesNum = FGlobalDynamicUIMeshPoolPolicy::CullAfterFramesNum
	};
	
	/** Creates the resource
	 * @param Args The buffer size in bytes.
	 * @returns A suitably sized buffer or NULL on failure.
	 */
	FVertexBufferRHIRef CreateResource(FGlobalDynamicUIMeshPoolPolicy::CreationArguments Args)
	{
		FGlobalDynamicUIMeshPoolPolicy::CreationArguments BufferSize = GetPoolBucketSize(GetPoolBucketIndex(Args));
		FRHIResourceCreateInfo CreateInfo;
		FVertexBufferRHIRef VertexBuffer = RHICreateVertexBuffer(BufferSize, BUF_Dynamic | BUF_ShaderResource, CreateInfo);
		return VertexBuffer;
	}
	
	/** Gets the arguments used to create resource
	 * @param Resource The buffer to get data for.
	 * @returns The arguments used to create the buffer.
	 */
	FGlobalDynamicUIMeshPoolPolicy::CreationArguments GetCreationArguments(FVertexBufferRHIRef Resource)
	{
		return (Resource->GetSize());
	}
	
	/** Frees the resource
	 * @param Resource The buffer to prepare for release from the pool permanently.
	 */
	void FreeResource(FVertexBufferRHIRef Resource)
	{

	}

};

class FGlobalDynamicUIMeshVertexPool : public TRenderResourcePool<FVertexBufferRHIRef, FGlobalDynamicUIMeshVertexPolicy, FGlobalDynamicUIMeshPoolPolicy::CreationArguments>
{
public:
	/** Destructor */
	virtual ~FGlobalDynamicUIMeshVertexPool() override
	{
	}
	
public:
	// From FTickableObjectRenderThread
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FGlobalDynamicUIMeshVertexPool, STATGROUP_Tickables);
	}
};

TGlobalResource<FGlobalDynamicUIMeshVertexPool> GDynamicUIMeshVertexPool;

/** This is our default implementation using GDynamicUIMeshIndexPool. */
class FPooledDynamicUIMeshBufferAllocator
{
public:
	FIndexBufferRHIRef AllocIndexBuffer(uint32 SizeInBytes)
	{
		if (SizeInBytes <= FGlobalDynamicUIMeshIndexPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
		{
			return GDynamicUIMeshIndexPool.CreatePooledResource(SizeInBytes);
		}

		FRHIResourceCreateInfo CreateInfo;
		return RHICreateIndexBuffer(sizeof(uint32), SizeInBytes, BUF_Static, CreateInfo);
	}

	void ReleaseIndexBuffer(FIndexBufferRHIRef& IndexBufferRHI)
	{
		if (IsValidRef(IndexBufferRHI))
		{
			if (IndexBufferRHI->GetSize() <= FGlobalDynamicUIMeshIndexPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
			{
				GDynamicUIMeshIndexPool.ReleasePooledResource(IndexBufferRHI);
			}

			IndexBufferRHI = nullptr;
		}
	}

	FIndexBufferRHIRef AllocIndexBuffer16(uint32 SizeInBytes)
	{
		if (SizeInBytes <= FGlobalDynamicUIMeshIndexPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
		{
			return GDynamicUIMeshIndexPool16.CreatePooledResource(SizeInBytes);
		}

		FRHIResourceCreateInfo CreateInfo;
		return RHICreateIndexBuffer(sizeof(uint16), SizeInBytes, BUF_Static, CreateInfo);
	}

	void ReleaseIndexBuffer16(FIndexBufferRHIRef& IndexBufferRHI)
	{
		if (IsValidRef(IndexBufferRHI))
		{
			if (IndexBufferRHI->GetSize() <= FGlobalDynamicUIMeshIndexPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
			{
				GDynamicUIMeshIndexPool16.ReleasePooledResource(IndexBufferRHI);
			}

			IndexBufferRHI = nullptr;
		}
	}

	FVertexBufferRHIRef AllocVertexBuffer(uint32 SizeInBytes)
	{
		if (SizeInBytes <= FGlobalDynamicUIMeshPoolPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
		{
			return GDynamicUIMeshVertexPool.CreatePooledResource(SizeInBytes);
		}

		FRHIResourceCreateInfo CreateInfo;
		return RHICreateVertexBuffer(SizeInBytes, BUF_Dynamic | BUF_ShaderResource, CreateInfo);
	}

	void ReleaseVertexBuffer(FVertexBufferRHIRef& VertexBufferRHI)
	{
		if (IsValidRef(VertexBufferRHI))
		{
			if (VertexBufferRHI->GetSize() <= FGlobalDynamicUIMeshPoolPolicy().GetPoolBucketSize(FGlobalDynamicUIMeshIndexPolicy::NumPoolBuckets - 1))
			{
				GDynamicUIMeshVertexPool.ReleasePooledResource(VertexBufferRHI);
			}

			VertexBufferRHI = nullptr;
		}
	}
};

static FPooledDynamicUIMeshBufferAllocator DefaultDynamicUIMeshBufferAllocator;

void FPooledDynamicUIMeshVertexBuffer::InitRHI()
{
	bUseBufferPool = CVarUsePooledBuffer.GetValueOnRenderThread() == 1;
	
	FVertexBuffer::InitRHI();

	const uint32 SizeInBytes = Vertices->Num() * sizeof(FUIVertex);
	if (!bUseBufferPool)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Static, CreateInfo);
	}
	else
	{
		VertexBufferRHI = DefaultDynamicUIMeshBufferAllocator.AllocVertexBuffer(SizeInBytes);

		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, SizeInBytes, RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices->GetData(), SizeInBytes);
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
}

void FPooledDynamicUIMeshVertexBuffer::ReleaseRHI()
{
	if (bUseBufferPool)
	{
		DefaultDynamicUIMeshBufferAllocator.ReleaseVertexBuffer(VertexBufferRHI);
	}
	
	FVertexBuffer::ReleaseRHI();
}

void FPooledDynamicUIMeshBuffer32::InitRHI()
{
	bUseBufferPool = CVarUsePooledBuffer.GetValueOnRenderThread() == 1;
	
	FIndexBuffer::InitRHI();

	const uint32 SizeInBytes = Indices->Num() * sizeof(uint32);
	
	if (!bUseBufferPool)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), SizeInBytes, BUF_Static, CreateInfo);

		UpdateRHI();
	}
	else
	{
		IndexBufferRHI = DefaultDynamicUIMeshBufferAllocator.AllocIndexBuffer(SizeInBytes);

		// Write the indices to the index buffer.
		void* Buffer;
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RHILockIndexBuffer)
			Buffer = RHILockIndexBuffer(IndexBufferRHI,0, SizeInBytes,RLM_WriteOnly);
		}
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(Memcpy)
			FMemory::Memcpy(Buffer, Indices->GetData(), SizeInBytes);
		}
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RHIUnlockIndexBuffer)
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}
}

void FPooledDynamicUIMeshBuffer32::UpdateRHI()
{
	if (!bUseBufferPool)
	{
		// Copy the index data into the index buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices->Num() * sizeof(uint32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices->GetData(), Indices->Num() * sizeof(uint32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
}

void FPooledDynamicUIMeshBuffer32::ReleaseRHI()
{
	if (bUseBufferPool)
	{
		DefaultDynamicUIMeshBufferAllocator.ReleaseIndexBuffer(IndexBufferRHI);
	}
	
	FIndexBuffer::ReleaseRHI();
}

void FPooledDynamicUIMeshBuffer16::InitRHI()
{
	bUseBufferPool = CVarUsePooledBuffer.GetValueOnRenderThread() == 1;
	
	FIndexBuffer::InitRHI();

	const uint32 SizeInBytes = Indices.Num() * sizeof(uint16);
	
	if (!bUseBufferPool)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), SizeInBytes, BUF_Static, CreateInfo);

		UpdateRHI();
	}
	else
	{
		IndexBufferRHI = DefaultDynamicUIMeshBufferAllocator.AllocIndexBuffer16(SizeInBytes);

		// Write the indices to the index buffer.
		void* Buffer;
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RHILockIndexBuffer)
			Buffer = RHILockIndexBuffer(IndexBufferRHI,0, SizeInBytes,RLM_WriteOnly);
		}
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(Memcpy)
			FMemory::Memcpy(Buffer, Indices.GetData(), SizeInBytes);
		}
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RHIUnlockIndexBuffer)
			RHIUnlockIndexBuffer(IndexBufferRHI);
		}
	}
}

void FPooledDynamicUIMeshBuffer16::UpdateRHI()
{
	// Copy the index data into the index buffer.
	void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(uint16), RLM_WriteOnly);
	FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(uint16));
	RHIUnlockIndexBuffer(IndexBufferRHI);
}

void FPooledDynamicUIMeshBuffer16::ReleaseRHI()
{
	if (bUseBufferPool)
	{
		DefaultDynamicUIMeshBufferAllocator.ReleaseIndexBuffer16(IndexBufferRHI);
	}
	
	FIndexBuffer::ReleaseRHI();
}
