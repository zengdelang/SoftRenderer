#pragma once

#include "CoreMinimal.h"
#include "Core/Renderer/UIRenderProxyInterface.h"
#include "Core/SpecializedCollections/ObjectIndexedSet.h"
#include "UIMeshStorage.generated.h"

USTRUCT()
struct FUIMeshStorage
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	FObjectIndexedSet UIMeshList;

	TWeakObjectPtr<class UUGUIWorldSubsystem> UIWorldSubsystem;

public:
	bool CanClearMesh(const UCanvasSubComponent* Canvas);

	void CacheAllUIMesh();

	int32 UpdateMeshComponentsPriority(UCanvasSubComponent* Canvas, int32& StartPriority, int32& PriorityCount);
	
};
