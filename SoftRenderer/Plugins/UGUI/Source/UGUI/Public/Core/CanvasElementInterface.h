#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CanvasElementInterface.generated.h"

class USceneComponent;

enum class ECanvasUpdate : uint8
{
    /**
     * Called before layout.
     */
    CanvasUpdate_Prelayout = 0,

    /**
     * Called for layout.
     */
    CanvasUpdate_Layout = 1,

    /**
     * Called after layout.
     */
    CanvasUpdate_PostLayout = 2,

    /**
     * Called before rendering.
     */
    CanvasUpdate_PreRender = 3,

    /**
     * Called late, before render.
     */
    CanvasUpdate_LatePreRender = 4,

    /**
     * Max enum value. Always last.
     */
    CanvasUpdate_MaxUpdateValue = 5

};

UINTERFACE(BlueprintType)
class UGUI_API UCanvasElementInterface : public UInterface
{
    GENERATED_BODY()
};

class UGUI_API ICanvasElementInterface
{
    GENERATED_BODY()

public:
    /**
	 * Rebuild the element for the given stage.
	 */
    virtual const USceneComponent* GetTransform() const { return nullptr; }
	
    /**
     * Rebuild the element for the given stage.
     *
     * @param  Executing  The current CanvasUpdate stage being rebuild.
     */
    virtual void Rebuild(ECanvasUpdate Executing) {}

    /**
     * Callback sent when this ICanvasElement has completed layout.
     */
    virtual void LayoutComplete() {}

    /**
     * Callback sent when this ICanvasElement has completed Graphic rebuild.
     */
    virtual void GraphicUpdateComplete() {}

    /**
     * Used if the native representation has been destroyed.
     *
     * @return Return true if the element is considered destroyed.
     */
    virtual bool IsDestroyed() { return true; }

    virtual FString ToString() { return FString(); }

    virtual uint32 GetHashCode() { return GetTypeHash(Cast<UObject>(this)); };
	
};
