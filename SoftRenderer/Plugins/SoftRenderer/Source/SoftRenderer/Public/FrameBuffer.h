#pragma once

#include "CoreMinimal.h"
#include "FrameBuffer.generated.h"

/**
 * FrameBuffer --- 帧图像数据
 * 
 * 等价于Bitmap的位图数据结构
 *     左上角第一个点代表原点，X,Y坐标(0, 0)
 *     Width代表水平方向的像素个数
 *     Height代码垂直方向的像素个数
 *     Pixels存储一维像素数组，存储顺序如下
 *
 * Pixels存储顺序, 每一个格子存储一个uint32的数据，每8个bit代表一个颜色通道(b,g,r,a)
 *    - - - - - X
 *    - 0 1 2 3 
 *    - 4 5 6 7
 *    Y
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class UFrameBuffer : public UObject
{
	GENERATED_UCLASS_BODY()

protected:
	/** 帧图像的像素宽度 */
	int32 Width;

	/** 帧图像的像素高度 */
	int32 Height;

	/** 帧图像的一维像素数组数据 */
	TArray<uint32> Pixels;

	/** 导出帧图像数据到Texture */
	UPROPERTY(Transient)
	UTexture2D* Texture;
	
public:
	/**
	 * 指定帧图像的新像素宽高
	 */
	UFUNCTION(BlueprintCallable)
	void Resize(int32 InWidth, int32 InHeight);

	/**
	 * 清理帧图像数据，用指定的颜色填充整个帧图像数据
	 */
	UFUNCTION(BlueprintCallable)
	void Clear(FLinearColor ClearColor = FLinearColor::Black);
	
	/**
	 * 在X,Y对应位置的像素上填充Color指定的颜色
	 */
	UFUNCTION(BlueprintCallable)
	void Point(int32 X, int32 Y, FLinearColor Color = FLinearColor::Black);
	
	/**
	 * 帧图像数据更新到Texture2D纹理中
	 */
	UFUNCTION(BlueprintCallable)
	UTexture2D* UpdateTexture2D();

public:
	void DrawLine(int32 StartX, int32 StartY, int32 EndX, int32 EndY);
	
};
