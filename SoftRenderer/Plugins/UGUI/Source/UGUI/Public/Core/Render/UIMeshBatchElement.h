#pragma once

#include "CoreMinimal.h"
#include "VertexHelper.h"

struct FUIMeshBatchSection
{
	TSharedPtr<FVertexHelper> Mesh;
	FTransform RendererToWorldTransform;

	FLinearColor Color;
	float InheritedAlpha;

	int32 VertexStartIndex;
	
public:
	FUIMeshBatchSection()
		: InheritedAlpha(1)
		, VertexStartIndex(0)
	{
	}

	FUIMeshBatchSection(const FLinearColor& InColor, const float& InInheritedAlpha,
		const FTransform& InRendererToWorldTransform, const TSharedPtr<FVertexHelper>& InMesh, int32 InVertexStartIndex)
	{
		RendererToWorldTransform = InRendererToWorldTransform;
		Color = InColor;
		InheritedAlpha = InInheritedAlpha;
		VertexStartIndex = InVertexStartIndex;

		Mesh = InMesh;
	}
};

class UGUI_API FUIMeshBatchElement
{
public:
	TArray<FUIMeshBatchSection> UIMeshBatchSections;

public:
	FUIMeshBatchElement()
	{
		UIMeshBatchSections.Reserve(32);
	}
	
	void MergeVertexBuffer(TArray<FDynamicMeshVertex>& Vertices, FBox& LocalBox, const int32 BatchVerticesCount, const FTransform& WorldToCanvasTransform);
	void MergeRHIBuffer(TArray<FUIVertex>& Vertices, bool bIsScreenSpaceOverlay, FBox& LocalBox, const int32 BatchVerticesCount, const FTransform& WorldToCanvasTransform);

protected:
	void MergeSectionVertexBuffer(TArray<FDynamicMeshVertex>& Vertices, TArray<FUIVertex>& UIMeshVertices, int32 VerticesDelta,
		const FTransform& RendererToCanvasTransform, const FLinearColor& SectionColor, float InheritedAlpha, FBox& LocalBox) const;
	void MergeSectionRHIVertexBuffer(TArray<FUIVertex>& Vertices, bool bIsScreenSpaceOverlay, const TArray<FUIVertex>& UIMeshVertices,
		int32 VerticesDelta, const FTransform& RendererToCanvasTransform, const FLinearColor& SectionColor, float InheritedAlpha, FBox& LocalBox) const;

public:
	void MergeIndexBuffer(TArray<uint32>& Indices, const int32 BatchIndexCount);

protected:
	static void MergeSectionIndexBuffer(TArray<uint32>& Indices, const TArray<uint32>& UIMeshIndices, int32 IndexDelta, int32 VerticesDelta);
	
};
