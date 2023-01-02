#include "VertexShader.h"

/////////////////////////////////////////////////////
// UVertexShader

UVertexShader::UVertexShader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FVector4 UVertexShader::RunVertexShader(const FRenderObjectVertex& Vertex,
	const FMatrix& LocalToWorldMatrix, const FMatrix& WorldToViewMatrix, const FMatrix& ProjectionMatrix)
{
	const auto WorldSpacePos = LocalToWorldMatrix.TransformPosition(Vertex.Position);
	const auto ViewSpacePos = WorldToViewMatrix.TransformPosition(WorldSpacePos);
	const auto ClipSpacePos = ProjectionMatrix.TransformPosition(ViewSpacePos);

	// 顶点着色器输出齐次裁剪空间的位置
	return ClipSpacePos;
}

/////////////////////////////////////////////////////
