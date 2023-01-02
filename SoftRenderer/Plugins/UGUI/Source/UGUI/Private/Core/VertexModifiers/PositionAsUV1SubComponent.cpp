#include "Core/VertexModifiers/PositionAsUV1SubComponent.h"

/////////////////////////////////////////////////////
// UPositionAsUV1SubComponent

UPositionAsUV1SubComponent::UPositionAsUV1SubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UPositionAsUV1SubComponent::ModifyMesh(FVertexHelper& VertexHelper)
{
    FUIVertex Vertex;
    for (int32 Index = 0, Count = VertexHelper.GetCurrentVerticesCount(); Index < Count; ++Index)
    {
        VertexHelper.PopulateUIVertex(Vertex, Index);
        Vertex.UV1 = FVector2D(Vertex.Position.X, Vertex.Position.Y);
        VertexHelper.SetUIVertex(Vertex, Index);
    }
}

/////////////////////////////////////////////////////
