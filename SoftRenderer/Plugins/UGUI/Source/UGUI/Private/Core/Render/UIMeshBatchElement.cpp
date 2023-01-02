#include "Core/Render/UIMeshBatchElement.h"
#include "Core/Render/CanvasRendererSubComponent.h"

/////////////////////////////////////////////////////
// FUIMeshBatchElement

void FUIMeshBatchElement::MergeVertexBuffer(TArray<FDynamicMeshVertex>& Vertices, FBox& LocalBox,
	const int32 BatchVerticesCount, const FTransform& WorldToCanvasTransform)
{
	Vertices.SetNumZeroed(BatchVerticesCount);
	
	int32 VerticesDelta = 0;

	for (int32 Index = 0, Count = UIMeshBatchSections.Num(); Index < Count; ++Index)
	{
		const auto& UIMeshBatchSection = UIMeshBatchSections[Index];
		auto& UIMeshVertices = UIMeshBatchSections[Index].Mesh->Vertices;

		FPlatformMisc::Prefetch(UIMeshVertices.GetData());

		const int32 SectionVerticesCount = UIMeshVertices.Num();
		if (SectionVerticesCount > 0)
		{
			FTransform RendererToCanvasTransform;
			FTransform::Multiply(&RendererToCanvasTransform, &UIMeshBatchSections[Index].RendererToWorldTransform, &WorldToCanvasTransform);
			MergeSectionVertexBuffer(Vertices, UIMeshVertices, VerticesDelta, RendererToCanvasTransform, UIMeshBatchSection.Color, UIMeshBatchSection.InheritedAlpha, LocalBox);
		}

		VerticesDelta += SectionVerticesCount;
	}
}

void FUIMeshBatchElement::MergeRHIBuffer(TArray<FUIVertex>& Vertices, bool bIsScreenSpaceOverlay, FBox& LocalBox,
	const int32 BatchVerticesCount, const FTransform& WorldToCanvasTransform)
{
	Vertices.SetNumZeroed(BatchVerticesCount);
	
	int32 VerticesDelta = 0;
	
	for (int32 Index = 0, Count = UIMeshBatchSections.Num(); Index < Count; ++Index)
	{
		const auto& UIMeshBatchSection = UIMeshBatchSections[Index];
		const auto& UIMeshVertices = UIMeshBatchSections[Index].Mesh->Vertices;

		FPlatformMisc::Prefetch(UIMeshVertices.GetData());

		const int32 SectionVerticesCount = UIMeshVertices.Num();
		if (SectionVerticesCount > 0)
		{
			FTransform RendererToCanvasTransform;
			FTransform::Multiply(&RendererToCanvasTransform, &UIMeshBatchSections[Index].RendererToWorldTransform, &WorldToCanvasTransform);
			MergeSectionRHIVertexBuffer(Vertices, bIsScreenSpaceOverlay, UIMeshVertices, VerticesDelta, RendererToCanvasTransform,
				UIMeshBatchSection.Color, UIMeshBatchSection.InheritedAlpha, LocalBox);
		}

		VerticesDelta += SectionVerticesCount;
	}
}

void FUIMeshBatchElement::MergeSectionVertexBuffer(TArray<FDynamicMeshVertex>& Vertices, TArray<FUIVertex>& UIMeshVertices, int32 VerticesDelta, const FTransform& RendererToCanvasTransform,
                                                   const FLinearColor& SectionColor, float InheritedAlpha, FBox& LocalBox) const
{
#if PLATFORM_LITTLE_ENDIAN
	const VectorRegister ColorRegister = MakeVectorRegister(SectionColor.B, SectionColor.G, SectionColor.R, SectionColor.A * InheritedAlpha);
#else // PLATFORM_LITTLE_ENDIAN
	const VectorRegister ColorRegister = MakeVectorRegister(SectionColor.A * InheritedAlpha, SectionColor.R, SectionColor.G, SectionColor.B);
#endif

	RendererToCanvasTransform.DiagnosticCheckNaN_All();
	
	const auto TransformTranslation = RendererToCanvasTransform.GetTranslation();
	const VectorRegister Translation = VectorLoadFloat3_W0(&TransformTranslation);
	
	const auto TransformRotation = RendererToCanvasTransform.GetRotation();
	const VectorRegister Rotation = VectorLoadAligned(&TransformRotation);

	const auto TransformScale3D = RendererToCanvasTransform.GetScale3D();
	const VectorRegister Scale3D = VectorLoadFloat3_W0(&TransformScale3D);

	const int32 VerticesCount = UIMeshVertices.Num();
	
	LocalBox.IsValid = LocalBox.IsValid || VerticesCount > 0;
	
	for (int32 Index = 0; Index < VerticesCount; ++Index)
	{
		FUIVertex& UIVertex = UIMeshVertices[Index];
			
		FDynamicMeshVertex& DynamicMeshVertex = Vertices[VerticesDelta];
		++VerticesDelta;
			
		DynamicMeshVertex.TextureCoordinate[0] = UIVertex.UV0;
		DynamicMeshVertex.TextureCoordinate[1] = UIVertex.UV1;
		DynamicMeshVertex.TextureCoordinate[2] = UIVertex.UV2;
		DynamicMeshVertex.TextureCoordinate[4] = UIVertex.UV4;
		DynamicMeshVertex.TextureCoordinate[5] = UIVertex.UV5;
		DynamicMeshVertex.TextureCoordinate[6] = UIVertex.UV6;
		DynamicMeshVertex.TextureCoordinate[7] = UIVertex.UV7;

		FastBlendLinearColorToFColor(ColorRegister, &UIVertex.Color, &DynamicMeshVertex.Color);
		
		// TransformPosition
		{
			const VectorRegister InputVectorW0 = VectorLoadFloat3_W0(&UIVertex.Position);

			const VectorRegister ScaledVec = VectorMultiply(Scale3D, InputVectorW0);
			const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);

			const VectorRegister TranslatedVec = VectorAdd(RotatedVec, Translation);
				
			VectorStoreFloat3(TranslatedVec, &DynamicMeshVertex.Position);

			DynamicMeshVertex.TextureCoordinate[3].X = DynamicMeshVertex.Position.X;
			DynamicMeshVertex.TextureCoordinate[3].Y = DynamicMeshVertex.Position.Y;

			const auto& BoxMinVectorRegister = VectorMin(TranslatedVec, VectorLoadFloat3(&LocalBox.Min));
			VectorStoreFloat3(BoxMinVectorRegister, &LocalBox.Min);

			const auto& BoxMaxVectorRegister = VectorMax(TranslatedVec, VectorLoadFloat3(&LocalBox.Max));
			VectorStoreFloat3(BoxMaxVectorRegister, &LocalBox.Max);
		}
	}
}

void FUIMeshBatchElement::MergeSectionRHIVertexBuffer(TArray<FUIVertex>& Vertices, bool bIsScreenSpaceOverlay, const TArray<FUIVertex>& UIMeshVertices, int32 VerticesDelta, const FTransform& RendererToCanvasTransform,
	const FLinearColor& SectionColor, float InheritedAlpha, FBox& LocalBox) const
{
	const int32 VerticesCount = UIMeshVertices.Num();
	FMemory::Memcpy(Vertices.GetData() + VerticesDelta, UIMeshVertices.GetData(), VerticesCount * sizeof(FUIVertex));

#if PLATFORM_LITTLE_ENDIAN
	const VectorRegister ColorRegister = MakeVectorRegister(SectionColor.B, SectionColor.G, SectionColor.R, SectionColor.A * InheritedAlpha);
	const bool bNeedBlendColor = VectorMaskBits(VectorCompareEQ(ColorRegister, GlobalVectorConstants::FloatOne)) != 0x0F;
#else // PLATFORM_LITTLE_ENDIAN
	const VectorRegister ColorRegister = MakeVectorRegister(SectionColor.A * InheritedAlpha, SectionColor.R, SectionColor.G, SectionColor.B);
	const bool bNeedBlendColor = VectorMaskBits(VectorCompareEQ(ColorRegister, GlobalVectorConstants::FloatOne)) != 0x0F;
#endif

	RendererToCanvasTransform.DiagnosticCheckNaN_All();
	
	const auto TransformTranslation = RendererToCanvasTransform.GetTranslation();
	const VectorRegister Translation = VectorLoadFloat3_W0(&TransformTranslation);
	
	const auto TransformRotation = RendererToCanvasTransform.GetRotation();
	const VectorRegister Rotation = VectorLoadAligned(&TransformRotation);

	const auto TransformScale3D = RendererToCanvasTransform.GetScale3D();
	const VectorRegister Scale3D = VectorLoadFloat3_W0(&TransformScale3D);
	
	if (bNeedBlendColor)
	{
		if (bIsScreenSpaceOverlay)
		{
			for (int32 Index = 0; Index < VerticesCount; ++Index)
			{
				auto& RHIVertex = Vertices[VerticesDelta];
				++VerticesDelta;
		
				FastBlendLinearColorToFColor(ColorRegister, &RHIVertex.Color);
		
				// TransformPosition
				{
					const VectorRegister InputVectorW0 = VectorLoadFloat3_W0(&UIMeshVertices[Index].Position);

					const VectorRegister ScaledVec = VectorMultiply(Scale3D, InputVectorW0);
					const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);

					const VectorRegister TranslatedVec = VectorAdd(RotatedVec, Translation);
				
					VectorStoreFloat3(TranslatedVec, &RHIVertex.Position);

					RHIVertex.UV3.X = RHIVertex.Position.X;
					RHIVertex.UV3.Y = RHIVertex.Position.Y;
				}
			}
		}
		else
		{
			LocalBox.IsValid = LocalBox.IsValid || VerticesCount > 0;
			
			for (int32 Index = 0; Index < VerticesCount; ++Index)
			{
				auto& RHIVertex = Vertices[VerticesDelta];
				++VerticesDelta;
		
				FastBlendLinearColorToFColor(ColorRegister, &RHIVertex.Color);
		
				// TransformPosition
				{
					const VectorRegister InputVectorW0 = VectorLoadFloat3_W0(&UIMeshVertices[Index].Position);

					const VectorRegister ScaledVec = VectorMultiply(Scale3D, InputVectorW0);
					const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);

					const VectorRegister TranslatedVec = VectorAdd(RotatedVec, Translation);
				
					VectorStoreFloat3(TranslatedVec, &RHIVertex.Position);

					RHIVertex.UV3.X = RHIVertex.Position.X;
					RHIVertex.UV3.Y = RHIVertex.Position.Y;

					const auto& BoxMinVectorRegister = VectorMin(TranslatedVec, VectorLoadFloat3(&LocalBox.Min));
					VectorStoreFloat3(BoxMinVectorRegister, &LocalBox.Min);
					
					const auto& BoxMaxVectorRegister = VectorMax(TranslatedVec, VectorLoadFloat3(&LocalBox.Max));
					VectorStoreFloat3(BoxMaxVectorRegister, &LocalBox.Max);
				}
			}
		}
	}
	else
	{
		if (bIsScreenSpaceOverlay)
		{
			for (int32 Index = 0; Index < VerticesCount; ++Index)
			{
				auto& RHIVertex = Vertices[VerticesDelta];
				++VerticesDelta;

				// TransformPosition
				{
					const VectorRegister InputVectorW0 = VectorLoadFloat3_W0(&UIMeshVertices[Index].Position);

					const VectorRegister ScaledVec = VectorMultiply(Scale3D, InputVectorW0);
					const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);

					const VectorRegister TranslatedVec = VectorAdd(RotatedVec, Translation);
				
					VectorStoreFloat3(TranslatedVec, &RHIVertex.Position);

					RHIVertex.UV3.X = RHIVertex.Position.X;
					RHIVertex.UV3.Y = RHIVertex.Position.Y;
				}
			}
		}
		else
		{
			LocalBox.IsValid = LocalBox.IsValid || VerticesCount > 0;
			
			for (int32 Index = 0; Index < VerticesCount; ++Index)
			{
				auto& RHIVertex = Vertices[VerticesDelta];
				++VerticesDelta;

				// TransformPosition
				{
					const VectorRegister InputVectorW0 = VectorLoadFloat3_W0(&UIMeshVertices[Index].Position);

					const VectorRegister ScaledVec = VectorMultiply(Scale3D, InputVectorW0);
					const VectorRegister RotatedVec = VectorQuaternionRotateVector(Rotation, ScaledVec);

					const VectorRegister TranslatedVec = VectorAdd(RotatedVec, Translation);
				
					VectorStoreFloat3(TranslatedVec, &RHIVertex.Position);

					RHIVertex.UV3.X = RHIVertex.Position.X;
					RHIVertex.UV3.Y = RHIVertex.Position.Y;

					const auto& BoxMinVectorRegister = VectorMin(TranslatedVec, VectorLoadFloat3(&LocalBox.Min));
					VectorStoreFloat3(BoxMinVectorRegister, &LocalBox.Min);

					const auto& BoxMaxVectorRegister = VectorMax(TranslatedVec, VectorLoadFloat3(&LocalBox.Max));
					VectorStoreFloat3(BoxMaxVectorRegister, &LocalBox.Max);
				}
			}
		}
	}
}

void FUIMeshBatchElement::MergeIndexBuffer(TArray<uint32>& Indices, const int32 BatchIndexCount)
{
	Indices.SetNumZeroed(BatchIndexCount);
	
	int32 IndexDelta = 0;
	int32 VerticesDelta = 0;
	
	for (int32 Index = 0, Count = UIMeshBatchSections.Num(); Index < Count; ++Index)
	{
		const auto& UIMeshBatchSection = UIMeshBatchSections[Index];
		const auto& UIMeshIndices = UIMeshBatchSections[Index].Mesh->Indices;
		
		FPlatformMisc::Prefetch(UIMeshIndices.GetData());

		const int32 SectionIndexCount = UIMeshIndices.Num();
		const int32 SectionVerticesCount = UIMeshBatchSection.Mesh->Vertices.Num();
		if (SectionIndexCount > 0)
		{
			MergeSectionIndexBuffer(Indices, UIMeshIndices, IndexDelta, VerticesDelta);
		}

		IndexDelta += SectionIndexCount;
		VerticesDelta += SectionVerticesCount;
	}
}

void FUIMeshBatchElement::MergeSectionIndexBuffer(TArray<uint32>& Indices, const TArray<uint32>& UIMeshIndices, int32 IndexDelta, int32 VerticesDelta)
{
	uint32* IndexBufferData = Indices.GetData() + IndexDelta; 
	
	const uint32* IndicesData = UIMeshIndices.GetData();
	const int32 Count = UIMeshIndices.Num();
	const int32 Times = Count / 4;
	
	for (int32 Index = 0; Index < Times; ++Index)
	{
#if PLATFORM_MAC || PLATFORM_MAC_X86 || PLATFORM_MAC_ARM64
		VectorIntStore(VectorIntAdd(VectorIntLoadAligned(IndicesData), MakeVectorRegisterInt(VerticesDelta, VerticesDelta, VerticesDelta, VerticesDelta)), IndexBufferData);
#else
		VectorIntStoreAligned(VectorIntAdd(VectorIntLoadAligned(IndicesData), MakeVectorRegisterInt(VerticesDelta, VerticesDelta, VerticesDelta, VerticesDelta)), IndexBufferData);
#endif
		
		IndicesData += 4;
		IndexBufferData += 4;
	}

	const int32 RemainTimes = Count % 4;
	for (int32 Index = 0; Index < RemainTimes; ++Index)
	{
		IndexBufferData[0] = (IndicesData[0] + VerticesDelta);
		++IndicesData;
		++IndexBufferData;
	}
}

/////////////////////////////////////////////////////
