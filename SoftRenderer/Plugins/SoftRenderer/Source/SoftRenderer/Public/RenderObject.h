#pragma once

#include "CoreMinimal.h"
#include "VertexShader.h"
#include "RenderObject.generated.h"

/**
 * 渲染对象的材质信息
 */
USTRUCT(BlueprintType)
struct FRenderObjectMaterial
{
	GENERATED_BODY()

public:
	/**
	 * 顶点着色器类
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UVertexShader> VertexShaderClass;

	/**
	 * 顶点着色器
	 */
	UPROPERTY(Transient)
	UVertexShader* VertexShader;
};

/**
 * 渲染对象
 *    模拟一个简单的静态模型渲染对象
 *
 * 构造一个简单三角形模型的方法:
 *    Vertices[0] = FRenderObjectVertex(Position = [50, 0, 0])
 *    Vertices[1] = FRenderObjectVertex(Position = [-50, 0, 0])
 *    Vertices[2] = FRenderObjectVertex(Position = [0, 50, 0])
 *     
 *    Indices = {0, 1, 2} (表示用Vertices数组中的第0，1，2这三个顶点来构造一个三角形)
 */
UCLASS(Blueprintable, BlueprintType)
class SOFTRENDERER_API URenderObject : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * 模型本地空间的顶点信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRenderObjectVertex> Vertices;

	/**
	 * 模型本地空间的索引信息，一个三角形3个顶点构成，必须是3的倍数
	 * 这里使用uint32来索引，实际渲染管线还有uint16格式的索引信息可选，uint16格式存储用于节省内存
	 * 由于蓝图这里uint32编译不过，用int32代替
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Indices;

	/**
	 * 渲染对象在世界空间中的位置
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation;

	/**
	 * 渲染对象在世界空间中的旋转
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator WorldRotation;

	/**
	 * 渲染对象在世界空间中的缩放
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldScale;

public:
	/**
	 * 渲染对象的材质信息
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRenderObjectMaterial Material;

public:
	/**
	 * 获取渲染对象从本地空间转换到世界空间的变换矩阵
	 */
	FMatrix GetLocalToWorld() const;
	
};
