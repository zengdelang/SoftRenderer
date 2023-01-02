#include "Core/Render/VertexHelper.h"

TArray<FUIVertex> FVertexHelper::VertexStream;
FVector FVertexHelper::DefaultTangent = FVector(1.f, 0.f, 0.f);
FVector FVertexHelper::DefaultNormal = FVector(0, 0, -1);

void FVertexHelper::GetUIVertexStream(TArray<FUIVertex>& OutVertices)
{
	OutVertices.Reset();

	for (int32 I = 0, Count = Indices.Num(); I < Count; ++I)
	{
		const int32 Index = Indices[I];
		const FUIVertex& UIVertex = Vertices[Index];
		OutVertices.Add(UIVertex);
	}
}

void FVertexHelper::AddUIVertexTriangleStream(const TArray<FUIVertex>& InVertices)
{
	Reserve(InVertices.Num(), InVertices.Num());
	
	for (int32 I = 0, Count = InVertices.Num(); I < Count; I += 3)
	{
		const int32 StartIndex = Vertices.Num();

		for (int32 J = 0; J < 3; ++J)
		{
			const auto& Vertex = InVertices[I + J];
			Vertices.Add(Vertex);
		}
    	
		Indices.Add(StartIndex);
		Indices.Add(StartIndex + 1);
		Indices.Add(StartIndex + 2);
	}
}
