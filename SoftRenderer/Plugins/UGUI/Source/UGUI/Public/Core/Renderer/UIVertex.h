#pragma once

#include "CoreMinimal.h"
#include "RenderResource.h"

FORCEINLINE void FastLinearColorToFColor(const FLinearColor& LinearColor, void* Ptr)
{
#if PLATFORM_LITTLE_ENDIAN
	const VectorRegister FinalLinearColor = VectorMultiply(MakeVectorRegister( LinearColor.B, LinearColor.G, LinearColor.R, LinearColor.A ), MakeVectorRegister( 255.0f, 255.0f, 255.0f, 255.0f));
	VectorStoreByte4(FinalLinearColor, Ptr);
#else // PLATFORM_LITTLE_ENDIAN
	const VectorRegister FinalLinearColor = VectorMultiply(MakeVectorRegister( LinearColor.A, LinearColor.R, LinearColor.G, LinearColor.B ), MakeVectorRegister( 255.0f, 255.0f, 255.0f, 255.0f));
	VectorStoreByte4(FinalLinearColor, Ptr);
#endif
}

FORCEINLINE void FastBlendLinearColorToFColor(VectorRegister ColorRegister, void* Ptr)
{
	const VectorRegister FinalLinearColor = VectorMultiply(ColorRegister, VectorLoadByte4(Ptr));
	VectorStoreByte4(FinalLinearColor, Ptr);
}

FORCEINLINE void FastBlendLinearColorToFColor(VectorRegister ColorRegister, void* SrcPtr, void* TargetPtr)
{
	const VectorRegister FinalLinearColor = VectorMultiply(ColorRegister, VectorLoadByte4(SrcPtr));
	VectorStoreByte4(FinalLinearColor, TargetPtr);
}

MS_ALIGN(SIMD_ALIGNMENT) struct FUIVertex
{
	FVector Position;
	FColor Color;
	
	FVector2D UV0;
	FVector2D UV1;
	FVector2D UV2;
	FVector2D UV3;
	FVector2D UV4;
	FVector2D UV5;
	FVector2D UV6;
	FVector2D UV7;

#if WITH_UI_VERTEX_DATA_NORMAL_TANGENT
	FVector Normal;
	FVector Tangent;
#endif

private:
	static FVector DefaultTangent;

public:
	static FUIVertex SimpleVertex;

public:
	FUIVertex()
	{
	    
	}

	FUIVertex(FVector InPosition, FLinearColor InColor, FVector2D InUV0, FVector2D InUV1, FVector2D InUV2, FVector2D InUV4,
		FVector2D InUV5, FVector2D InUV6, FVector2D InUV7, FVector InNormal, FVector InTangent)
	{
		Position = InPosition;

		FastLinearColorToFColor(InColor, &Color);
		
		UV0 = InUV0;
		UV1 = InUV1;
		UV2 = InUV2;
		UV4 = InUV4;
		UV5 = InUV5;
		UV6 = InUV6;
		UV7 = InUV7;

#if WITH_UI_VERTEX_DATA_NORMAL_TANGENT
		Normal = InNormal;
		Tangent = InTangent;
#endif
	}

	FUIVertex(FVector InPosition, FColor InColor, FVector2D InUV0, FVector2D InUV1, FVector2D InUV2, FVector2D InUV4,
		FVector2D InUV5, FVector2D InUV6, FVector2D InUV7, FVector InNormal, FVector InTangent)
	{
		Position = InPosition;

		Color = InColor;
		
		UV0 = InUV0;
		UV1 = InUV1;
		UV2 = InUV2;
		UV4 = InUV4;
		UV5 = InUV5;
		UV6 = InUV6;
		UV7 = InUV7;

#if WITH_UI_VERTEX_DATA_NORMAL_TANGENT
		Normal = InNormal;
		Tangent = InTangent;
#endif
	}
} GCC_ALIGN(SIMD_ALIGNMENT);

template <> struct TIsPODType<FUIVertex> { enum { Value = true }; };

class FUIScreenVertexDeclaration : public FRenderResource
{
public:
	virtual ~FUIScreenVertexDeclaration() override {}

public:
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	
};

FVertexDeclarationRHIRef& GetUIScreenVertexDeclaration();

class FPooledDynamicUIMeshVertexBuffer : public FVertexBuffer
{
public:
	TArray<FUIVertex>* Vertices = nullptr;

private:
	bool bUseBufferPool = false;
	
public:
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	virtual FString GetFriendlyName() const override { return TEXT("FUIVertexBuffer"); }
	
};

/** Index Buffer */
class FPooledDynamicUIMeshBuffer32 : public FIndexBuffer
{
public:
	TArray<uint32>* Indices = nullptr;

private:
	bool bUseBufferPool = false;
	
public:
	virtual void InitRHI() override;
	void UpdateRHI();
	
	virtual void ReleaseRHI() override;
	
};

class FPooledDynamicUIMeshBuffer16 : public FIndexBuffer
{
public:
	TArray<uint16> Indices;

private:
	bool bUseBufferPool = false;

public:
	virtual void InitRHI() override;
	void UpdateRHI();
	
	virtual void ReleaseRHI() override;
	
};
