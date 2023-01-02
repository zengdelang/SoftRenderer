#pragma once

#include "CoreMinimal.h"
#include "RenderObject.h"
#include "RenderScene.generated.h"

/**
 * 简单的渲染场景表示
 *    OpaqueRenderObjects 不透明渲染对象列表
 */
UCLASS(Blueprintable, BlueprintType)
class URenderScene : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** 不透明渲染对象类列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<URenderObject>> OpaqueRenderObjectsClasses;
	
	/** 不透明渲染对象列表 */
	UPROPERTY(Transient)
	TArray<URenderObject*> OpaqueRenderObjects;
	
};
