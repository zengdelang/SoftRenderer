#pragma once

#include "CoreMinimal.h"
#include "FrameBuffer.h"
#include "RenderScene.h"
#include "SoftRenderer.generated.h"

/**
 * 相机的投影模式
 */
UENUM()
enum class ESoftRendererCameraProjectionMode : uint8
{
	Perspective,    // 透视投影
	Orthographic    // 正交投影
};

/**
 * 渲染相机信息
 */
USTRUCT(BlueprintType)
struct FSoftRendererCamera
{
	GENERATED_BODY()

public:
	/** 相机的投影模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESoftRendererCameraProjectionMode ProjectionMode = ESoftRendererCameraProjectionMode::Orthographic;

	/** 相机的视场角 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FOV = 90;

	/** 正交宽度，场景单位, 只用于正交投影模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OrthoWidth = 1280;
	
	/** 相机在世界空间中的位置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ViewOrigin = FVector(-500, 0, 0);

	/** 相机在世界空间中的旋转 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};

/**
 * 渲染模式
 */
UENUM()
enum class ESoftRendererRenderMode : uint8
{
	Wireframe,  // 线框模式   第一课只支持线框模式
};

/**
 * 软光栅渲染器
 *    创建渲染器对象后，需要调用InitRenderer来初始化渲染器
 *    每一帧更新渲染器，需要调用Render
 */
UCLASS(Blueprintable, BlueprintType)
class USoftRenderer : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** 渲染模式 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESoftRendererRenderMode RenderMode;

	/** 渲染相机信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftRendererCamera RenderCamera;
	
public:
	/** 渲染的视口大小 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint ViewportSize;

	/** 渲染开始时，初始清空颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor ClearColor;

	/** 渲染的帧图像数据 */
	UPROPERTY(BlueprintReadOnly, Transient)
	UFrameBuffer* FrameBuffer;

	/** 渲染场景对象,待渲染的物体保存在渲染场景中 */
	UPROPERTY(BlueprintReadWrite, Transient)
	URenderScene* RenderScene;
	
public:
	/**
	 * 初始化渲染器
	 */
	UFUNCTION(BlueprintCallable)
	void InitRenderer();
	
	/**
	 * 每一帧调用进行渲染
	 */
	UFUNCTION(BlueprintCallable)
	void Render();

protected:
	/**
	 * 绘制渲染对象
	 */
	void DrawPrimitive(URenderObject* RenderObject, const FMatrix& WorldToViewMatrix, const FMatrix& ProjectionMatrix) const;

	/**
	 * 计算投影变换矩阵
	 */
	FMatrix CalculateProjectionMatrix() const;
	
};
