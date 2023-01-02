#pragma once

#include "CoreMinimal.h"
#include "Core/CanvasElementInterface.h"
#include "SpecializedCollections/CanvasElementIndexedSet.h"
#include "CanvasUpdateRegistry.generated.h"

UCLASS()
class UGUI_API UCanvasUpdateRegistry : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    UPROPERTY()
    TArray<FCanvasElementIndexedSet> LayoutRebuildQueueList;

    UPROPERTY()
    TArray<FCanvasElementIndexedSet> GraphicRebuildQueueList;

    uint8 SwapChainIndex;
	
    uint8 bPerformingLayoutUpdate : 1;
    uint8 bPerformingGraphicUpdate : 1;

public:
    void Initialize();

protected:
    void CleanInvalidItems(uint8 CurChainIndex);
	
    bool InternalPerformUpdate(uint8 CurChainIndex);
	void PerformLayoutUpdate(FCanvasElementIndexedSet& LayoutRebuildQueue);
	void PerformGraphicUpdate(FCanvasElementIndexedSet& GraphicRebuildQueue);
	
public:
    void PerformUpdate();

    bool InternalRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element);
    bool InternalRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element);

    void InternalUnRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element);
    void InternalUnRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element);

};

class UGUI_API FCanvasUpdateRegistry
{
public:
	static void Initialize();
	static void Shutdown(); 

public:
    /**
     * Try and add the given element to the layout rebuild list.
     * Will not return if successfully added.
     *
     * @param  Element  The element that is needing rebuilt.
     */
    static void RegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element);

    /**
     * Try and add the given element to the layout rebuild list.
     *
     * @param  Element  The element that is needing rebuilt.
     * @return True if the element was successfully added to the rebuilt list.
     *         False if either already inside a Graphic Update loop OR has already been added to the list.
     */
    static bool TryRegisterCanvasElementForLayoutRebuild(ICanvasElementInterface* Element);

    /**
     * Try and add the given element to the rebuild list.
     * Will not return if successfully added.
     *
     * @param  Element  The element that is needing rebuilt.
     */
    static void RegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element);

    /**
     * Try and add the given element to the layout rebuild list.
     *
     * @param  Element  The element that is needing rebuilt.
     * @return True if the element was successfully added to the rebuilt list.
     *         False if either already inside a Graphic Update loop OR has already been added to the list.
     */
    static bool TryRegisterCanvasElementForGraphicRebuild(ICanvasElementInterface* Element);

    /**
     * Remove the given element from both the graphic and the layout rebuild lists.
     */
    static void UnRegisterCanvasElementForRebuild(ICanvasElementInterface* Element);

    /**
     * Are graphics layouts currently being calculated..
     *
     * @return True if the rebuild loop is CanvasUpdate.Prelayout, CanvasUpdate.Layout or CanvasUpdate.Postlayout
     */
    static bool IsRebuildingLayout()
    {
        Initialize();
        return Instance->bPerformingLayoutUpdate;
    }

    /**
     * Are graphics layouts currently being calculated..
     *
     * @return True if the rebuild loop is CanvasUpdate.PreRender or CanvasUpdate.Render
     */
    static bool IsRebuildingGraphics()
    {
        Initialize();
        return Instance->bPerformingGraphicUpdate;
    }
	
private:
	static UCanvasUpdateRegistry* Instance;
    static FDelegateHandle PerformUpdateHandle;
	
};
