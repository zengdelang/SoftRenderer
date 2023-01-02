#include "SoftRenderer.h"
#include "VertexShader.h"

/////////////////////////////////////////////////////
// USoftRenderer

USoftRenderer::USoftRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RenderMode = ESoftRendererRenderMode::Wireframe;
	
	ViewportSize = FIntPoint(1280, 720);
	FrameBuffer = nullptr;
	RenderScene = nullptr;
}

void USoftRenderer::InitRenderer()
{
	if (!IsValid(FrameBuffer))
	{
		FrameBuffer = NewObject<UFrameBuffer>();
		FrameBuffer->Resize(FMath::Max(2, ViewportSize.X), FMath::Max(2, ViewportSize.Y));
	}

	if (IsValid(RenderScene))
	{
		// 创建渲染对象
		for (int32 Index = 0, Count = RenderScene->OpaqueRenderObjectsClasses.Num(); Index < Count; ++Index)
		{
			RenderScene->OpaqueRenderObjects.Emplace(NewObject<URenderObject>(RenderScene,RenderScene->OpaqueRenderObjectsClasses[Index]));
		}
	}
}

void USoftRenderer::Render()
{
	if (!IsValid(FrameBuffer))
		return;

	if (!IsValid(RenderScene))
		return;

	// 1 视口变化时Resize下FrameBuffer
	FrameBuffer->Resize(ViewportSize.X, ViewportSize.Y);

	// 2 将上一帧渲染的颜色数据用指定颜色清空
	FrameBuffer->Clear(ClearColor);

	// 3 计算视口变换矩阵
	//    这里要乘以一个额外的矩阵原因
	//       1 采用UE的坐标系，Z向上  X屏幕向内  Y向右
	//       2 需要转换为DirectX的左手坐标系，需要再旋转下
	const FMatrix ViewRotationMatrix = FInverseRotationMatrix(RenderCamera.Rotation) * FMatrix(
		FPlane(0,	0,	1,	0),
		FPlane(1,	0,	0,	0),
		FPlane(0,	1,	0,	0),
		FPlane(0,	0,	0,	1));
	
	const FMatrix WorldToViewMatrix = FTranslationMatrix(-RenderCamera.ViewOrigin) * ViewRotationMatrix;

	// 4 计算投影矩阵
	const FMatrix ProjectMatrix = CalculateProjectionMatrix();
	
	// 5 逐个渲染不透明物体
	for (const auto& RenderObject : RenderScene->OpaqueRenderObjects)
	{
		if (!IsValid(RenderObject))
			continue;
		
		DrawPrimitive(RenderObject, WorldToViewMatrix, ProjectMatrix);
	}
}

void USoftRenderer::DrawPrimitive(URenderObject* RenderObject, const FMatrix& WorldToViewMatrix, const FMatrix& ProjectionMatrix) const
{
	// 1 获取顶点着色器
	UVertexShader* VertexShader = RenderObject->Material.VertexShader;
	if (!IsValid(VertexShader) && RenderObject->Material.VertexShaderClass)
	{
		RenderObject->Material.VertexShader = NewObject<UVertexShader>(RenderObject, RenderObject->Material.VertexShaderClass);
		VertexShader = RenderObject->Material.VertexShader;
	}

	if (!IsValid(VertexShader))
		return;

	// 2 计算渲染对象的本地空间到时间空间的变换矩阵
	const FMatrix LocalToWorld = RenderObject->GetLocalToWorld();
	
	// 3 对每一个顶点执行顶点着色器程序
	for (auto& Vertex : RenderObject->Vertices)
	{
		Vertex.VertexPos = VertexShader->RunVertexShader(Vertex, LocalToWorld, WorldToViewMatrix, ProjectionMatrix);

		// 透视除法, 齐次坐标空间 /w 归一化到NDC坐标系中 
		Vertex.VertexPos *= 1 / Vertex.VertexPos.W;

		// 计算屏幕坐标
		Vertex.ScreenPos.X = (Vertex.VertexPos.X + 1.0f) * ViewportSize.X * 0.5f;
		Vertex.ScreenPos.Y = (1.0f - Vertex.VertexPos.Y) * ViewportSize.Y * 0.5f;

		// 整数屏幕坐标：加 0.5 的偏移取屏幕像素方格中心对齐，其实就是四舍五入
		Vertex.ScreenPosInPixels = FIntPoint( static_cast<int32>(Vertex.ScreenPos.X + 0.5f), static_cast<int32>(Vertex.ScreenPos.Y + 0.5f));
	}
	
	if (RenderMode == ESoftRendererRenderMode::Wireframe)
	{
		for (int32 Index = 0, Count = RenderObject->Indices.Num() / 3; Index < Count; ++Index)
		{
			const auto& Vertex1ScreenPos = RenderObject->Vertices[RenderObject->Indices[Index]].ScreenPosInPixels;
			const auto& Vertex2ScreenPos = RenderObject->Vertices[RenderObject->Indices[Index + 1]].ScreenPosInPixels;
			const auto& Vertex3ScreenPos = RenderObject->Vertices[RenderObject->Indices[Index + 2]].ScreenPosInPixels;
			
			FrameBuffer->DrawLine(Vertex1ScreenPos.X, Vertex1ScreenPos.Y, Vertex2ScreenPos.X, Vertex2ScreenPos.Y);
			FrameBuffer->DrawLine(Vertex2ScreenPos.X, Vertex2ScreenPos.Y, Vertex3ScreenPos.X, Vertex3ScreenPos.Y);
			FrameBuffer->DrawLine(Vertex1ScreenPos.X, Vertex1ScreenPos.Y, Vertex3ScreenPos.X, Vertex3ScreenPos.Y);
		}
	}
}

FMatrix USoftRenderer::CalculateProjectionMatrix() const
{
	float XAxisMultiplier;
	float YAxisMultiplier;
	
	const int32 SizeX = ViewportSize.X;
	const int32 SizeY = ViewportSize.Y;
	
	const bool bMaintainXFOV = RenderCamera.ProjectionMode == ESoftRendererCameraProjectionMode::Orthographic;
	if (bMaintainXFOV)
	{
		// 如果视口宽度大于高度
		XAxisMultiplier = 1.0f;
		YAxisMultiplier = SizeX / static_cast<float>(SizeY);
	}
	else
	{
		// 如果视口高度大于宽度
		XAxisMultiplier = SizeY / static_cast<float>(SizeX);
		YAxisMultiplier = 1.0f;
	}

	FMatrix ProjectionMatrix;
	
	if (RenderCamera.ProjectionMode == ESoftRendererCameraProjectionMode::Orthographic)
	{
		const float OrthoWidth = RenderCamera.OrthoWidth / 2.0f * XAxisMultiplier;
		const float OrthoHeight = RenderCamera.OrthoWidth / 2.0f / YAxisMultiplier;

		constexpr float NearPlane = 0.0f;
		constexpr float FarPlane = WORLD_MAX;

		constexpr float ZScale = 1.0f / (FarPlane - NearPlane);
		constexpr float ZOffset = -NearPlane;

		ProjectionMatrix = FOrthoMatrix(
				OrthoWidth, 
				OrthoHeight,
				ZScale,
				ZOffset
				);	
	}

	return ProjectionMatrix;
}

/////////////////////////////////////////////////////
