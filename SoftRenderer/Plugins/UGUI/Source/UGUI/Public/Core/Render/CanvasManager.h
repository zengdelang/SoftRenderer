#pragma once

#include "CoreMinimal.h"
#include "Core/Render/LateUpdateInterface.h"
#include "Core/Render/CanvasSubComponent.h"

class UGUI_API FCanvasManager
{
protected:
    FCanvasManager();
	
public:
	static void Initialize();
	static void Shutdown();

public:
	static void WillRenderCanvases();

	static void RefreshLateUpdateObjects();

public:
    static void AddCanvas(UCanvasSubComponent* CanvasComp);
    static void RemoveCanvas(UCanvasSubComponent* CanvasComp);

	static void SortCanvasList();
	
private:
	void InternalAddCanvas(UCanvasSubComponent* CanvasComp);
	void InternalRemoveCanvas(UCanvasSubComponent* CanvasComp);

	void InternalSortCanvasList();

public:
	static void AddLateUpdateObject(ILateUpdateInterface* LateUpdateObj);
	static void RemoveLateUpdateObject(ILateUpdateInterface* LateUpdateObj);

private:
	void InternalAddLateUpdateObject(ILateUpdateInterface* LateUpdateObj);
	void InternalRemoveLateUpdateObject(ILateUpdateInterface* LateUpdateObj);

public:
	/* Return the render order of canvas node. If canvas is not found, returns the number of canvases in the canvas manager. */
    static int32 GetRenderOrder(const UCanvasSubComponent* CanvasComp);
	
private:
	static TSharedPtr<class FCanvasManager> Instance;

	static TArray<TWeakObjectPtr<UObject>> InvalidObjects;
	
	TArray<TWeakObjectPtr<UCanvasSubComponent>> CanvasList;
	
	TSet<TWeakObjectPtr<UObject>> LateUpdateObjects;
	TSet<TWeakObjectPtr<UObject>> PendingAddLateUpdateObjects;
	TSet<TWeakObjectPtr<UObject>> PendingRemoveLateUpdateObjects;

	uint8 bUpdatingLatUpdateObjects : 1;
	
};
