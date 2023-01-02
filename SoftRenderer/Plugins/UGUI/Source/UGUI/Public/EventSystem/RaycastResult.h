#pragma once

#include "CoreMinimal.h"
#include "RaycastResult.generated.h"

class UBaseRaycasterSubComponent;

USTRUCT(BlueprintType)
struct FRaycastResult
{
	GENERATED_USTRUCT_BODY()

public:	
	/**
	 * The GameObject that was hit by the raycast.
	 */
    UPROPERTY(Transient)
    USceneComponent* GameObject;
	
    /**
     * The screen position from which the raycast was generated.
     */
    FVector ScreenPosition;

public:
    FRaycastResult()
    {
        GameObject = nullptr;
        ScreenPosition = FVector::ZeroVector;
    }

	virtual ~FRaycastResult() = default;
	
    /**
     * Is there an associated module and a hit GameObject.
     */
    bool IsValidResult() const;

    /**
     * Reset the result.
     */
    void Clear()
    {
        GameObject = nullptr;
        ScreenPosition = FVector::ZeroVector;
    }

    virtual FString ToString() const;
	virtual void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const;
	
};
