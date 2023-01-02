#pragma once

#include "CoreMinimal.h"
#include "Core/Renderer/UIVertex.h"

struct UGUI_API FVertexHelper
{
    friend class FUIMeshSceneProxy;
    friend class FUIMeshBatchElement;
    friend class UCanvasRendererSubComponent;
    
public:
    FORCEINLINE void Empty()
    {
        Vertices.Empty();
        Indices.Empty();
    }

    FORCEINLINE void Reset()
    {
        Vertices.Reset();
        Indices.Reset();
    }

    FORCEINLINE int32 GetCurrentVerticesCount() const
    {
        return Vertices.Num();
    }
	
    FORCEINLINE int32 GetCurrentIndexCount() const
    {
        return Indices.Num();
    }

    FORCEINLINE void Reserve(int32 VertexNum, int32 IndexNum)
    {
        Vertices.Reserve(VertexNum);
        Indices.Reserve(IndexNum);
    }

    FORCEINLINE void ReserveVertexNum(int32 VertexNum)
    {
        Vertices.Reserve(VertexNum);
    }

    FORCEINLINE void ReserveIndexNum(int32 IndexNum)
    {
        Indices.Reserve(IndexNum);
    }

    FORCEINLINE const FUIVertex* GetVertex(int32 Index)
    {
        if (Vertices.IsValidIndex(Index))
        {
            return &Vertices[Index];
        }
        
        return nullptr;
    }
	
public:
    /**
     * Fill a UIVertex with data from index of the stream.
     *
     * @param  Vertex  The vertex to fill
     * @param  Index  the position in the current list to fill.
     */
    FORCEINLINE void PopulateUIVertex(FUIVertex& Vertex, int32 Index)
    {
	    if (Vertices.IsValidIndex(Index))
	    {
            const FUIVertex& UIVertex = Vertices[Index];
            Vertex.Position = UIVertex.Position;
	        Vertex.Color = UIVertex.Color;
            Vertex.UV0 = UIVertex.UV0;
            Vertex.UV1 = UIVertex.UV1;
            Vertex.UV2 = UIVertex.UV2;
	        Vertex.UV4 = UIVertex.UV4;
	        Vertex.UV5 = UIVertex.UV5;
	        Vertex.UV6 = UIVertex.UV6;
	        Vertex.UV7 = UIVertex.UV7;

#if WITH_UI_VERTEX_DATA_NORMAL_TANGENT
	        Vertex.Normal = UIVertex.Normal;
	        Vertex.Tangent = UIVertex.Tangent;
#endif
	    }
    }

    /**
     * Set a UIVertex at the given index.
     * 
	 * @param  Vertex  The vertex to fill
	 * @param  Index  the position in the current list to fill.
     */
    FORCEINLINE void SetUIVertex(const FUIVertex& Vertex, int32 Index)
    {
        if (Vertices.IsValidIndex(Index))
        {
            FUIVertex& UIVertex = Vertices[Index];
            UIVertex.Position = Vertex.Position;
            UIVertex.Color = Vertex.Color;
            UIVertex.UV0 = Vertex.UV0;
            UIVertex.UV1 = Vertex.UV1;
            UIVertex.UV2 = Vertex.UV2;
            UIVertex.UV4 = Vertex.UV4;
            UIVertex.UV5 = Vertex.UV5;
            UIVertex.UV6 = Vertex.UV6;
            UIVertex.UV7 = Vertex.UV7;
            
#if WITH_UI_VERTEX_DATA_NORMAL_TANGENT
            UIVertex.Normal = Vertex.Normal;
            UIVertex.Tangent = Vertex.Tangent;
#endif
        }
    }

    FORCEINLINE void UpdateAllUV1(const FVector2D& InUV1)
    {
        for (int32 Index = 0, Count = Vertices.Num(); Index < Count; ++Index)
        {
            Vertices[Index].UV1 = InUV1;
        }
    }

public:
    /**
     * Add a single vertex to the stream.
     *
     * @param  Position  Position of the vertex
     * @param  Color  Color of the vertex
     * @param  UV0  UV of the vertex
     * @param  UV1  UV1 of the vertex
     * @param  UV2  UV2 of the vertex
     * @param  UV4  UV4 of the vertex
     * @param  UV5  UV5 of the vertex
     * @param  UV6  UV6 of the vertex
     * @param  UV7  UV7 of the vertex
     * @param  Normal  Normal of the vertex.
     * @param  Tangent  Tangent of the vertex
     */
    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2, FVector2D UV4, FVector2D UV5, FVector2D UV6, FVector2D UV7, FVector Normal, FVector Tangent)
    {
        FUIVertex UIVertex(Position, Color, UV0, UV1, UV2, UV4, UV5, UV6, UV7, Normal, Tangent);
        Vertices.Emplace(UIVertex);
    }

	/**
	 * Add a single vertex to the stream.
	 *
	 * @param  Position  Position of the vertex
     * @param  Color  Color of the vertex
     * @param  UV0  UV of the vertex
     * @param  UV1  UV1 of the vertex
     * @param  Normal  Normal of the vertex.
     * @param  Tangent  Tangent of the vertex
	 */
    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector Normal, FVector Tangent)
    {
        AddVert(Position, Color, UV0, UV1, FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector, Normal, Tangent);
    }
	
    /**
     * Add a single vertex to the stream.
     *
     * @param  Position  Position of the vertex
     * @param  Color  Color of the vertex
     * @param  UV0  UV of the vertex
     */
    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0)
    {
        AddVert(Position, Color, UV0, FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1)
    {
        AddVert(Position, Color, UV0, UV1, FVector2D::ZeroVector, FVector2D::ZeroVector, FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2)
    {
        AddVert(Position, Color, UV0, UV1, UV2, FVector2D::ZeroVector, FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2, FVector2D UV4)
    {
        AddVert(Position, Color, UV0, UV1, UV2, UV4, FVector2D::ZeroVector,FVector2D::ZeroVector,FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2, FVector2D UV4, FVector2D UV5)
    {
        AddVert(Position, Color, UV0, UV1, UV2, UV4, UV5, FVector2D::ZeroVector,FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2, FVector2D UV4, FVector2D UV5, FVector2D UV6)
    {
        AddVert(Position, Color, UV0, UV1, UV2, UV4, UV5, UV6, FVector2D::ZeroVector, DefaultNormal, DefaultTangent);
    }

    FORCEINLINE void AddVert(FVector Position, FLinearColor Color, FVector2D UV0, FVector2D UV1, FVector2D UV2, FVector2D UV4, FVector2D UV5, FVector2D UV6, FVector2D UV7)
    {
        AddVert(Position, Color, UV0, UV1, UV2, UV4, UV5, UV6, UV7, DefaultNormal, DefaultTangent);
    }

    /**
     * Add a single vertex to the stream.
     */
    FORCEINLINE void AddVert(const FUIVertex& Vertex)
    {
        Vertices.Add(Vertex);
    }

    /**
     * Add a triangle to the buffer.
     *
     * @param  Index0  Index 0
     * @param  Index1  Index 1
     * @param  Index2  Index 2
     */
    FORCEINLINE void AddTriangle(uint32 Index0, uint32 Index1, uint32 Index2)
    {
        Indices.Emplace(Index0);
        Indices.Emplace(Index1);
        Indices.Emplace(Index2);
    }

    FORCEINLINE void AddIndex(uint32 Index)
    {
        Indices.Emplace(Index);
    }
	
	/**
	 * Add a quad to the stream.
	 *
	 * @param  InVertices  4 Vertices representing the quad.
	 */
    FORCEINLINE void AddUIVertexQuad(const TArray<FUIVertex>& InVertices)
    {
        const int32 StartIndex = Vertices.Num();

        for (int32 Index = 0; Index < 4; ++Index)
        {
            Vertices.Add(InVertices[Index]);
        }

        AddTriangle(StartIndex, StartIndex + 2, StartIndex + 1);
        AddTriangle(StartIndex + 2, StartIndex, StartIndex + 3);
    }

public:
	/**
	 * Create a stream of UI vertex (in triangles) from the stream.
	 */
    void GetUIVertexStream(TArray<FUIVertex>& OutVertices);

	/**
	 * Add a list of triangles to the stream.
	 *
	 * @param  InVertices  InVertices to add. Length should be divisible by 3.
	 */
    void AddUIVertexTriangleStream(const TArray<FUIVertex>& InVertices);

public:
    FORCEINLINE void MoveVertexHelper(FVertexHelper& VertexHelper)
    {
        Vertices = MoveTemp(VertexHelper.Vertices);
        Indices = MoveTemp(VertexHelper.Indices);
    }
	
public:
    static TArray<FUIVertex> VertexStream;
	
private:
    static FVector DefaultTangent;
	static FVector DefaultNormal;

    MS_ALIGN(SIMD_ALIGNMENT) TArray<FUIVertex> Vertices GCC_ALIGN(SIMD_ALIGNMENT);
    MS_ALIGN(SIMD_ALIGNMENT) TArray<uint32> Indices GCC_ALIGN(SIMD_ALIGNMENT);  // use int32 for SIMD

};
