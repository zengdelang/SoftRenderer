#pragma once

#include "CoreMinimal.h"
#include "VertexShader.generated.h"

/**
 * 渲染对象的顶点信息
 *    第一课中，目前顶点信息只用到Position的数据
 *    
 */
USTRUCT(BlueprintType)
struct FRenderObjectVertex
{
	GENERATED_BODY()

public:
	/**
	 * 顶点在本地空间中的位置
	 */
	UPROPERTY(EditAnywhere)
	FVector Position;

public:
	/**
	 * 存储顶点着色器计算后的坐标位置
	 */
	FVector4 VertexPos;
	
	/**
	 * 存储顶点着色器计算后的屏幕坐标位置
	 */
	FVector2D ScreenPos;

	/**
	 * 存储顶点着色器计算后的屏幕坐标位置,像素单位
	 */
	FIntPoint ScreenPosInPixels;
	
};

/**
 * 顶点着色器对象
 */
UCLASS(Blueprintable, BlueprintType)
class SOFTRENDERER_API UVertexShader : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual FVector4 RunVertexShader(const FRenderObjectVertex& Vertex,
		const FMatrix& LocalToWorldMatrix, const FMatrix& WorldToViewMatrix, const FMatrix& ProjectionMatrix);
	
};
