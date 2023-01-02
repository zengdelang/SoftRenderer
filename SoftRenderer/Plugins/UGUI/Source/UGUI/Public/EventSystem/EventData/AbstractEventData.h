#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbstractEventData.generated.h"

/**
 * A class that can be used for sending simple events via the event system.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UAbstractEventData : public UObject
{
	GENERATED_UCLASS_BODY()

protected:
	uint8 bUsed : 1;

public:
	/**
	 * Reset the event.
	 */
	UFUNCTION(BlueprintCallable, Category = AbstractEventData)
	virtual void Reset()
	{
		bUsed = false;
	}

	/**
	 * Use the event.
	 *
	 * Internally sets a flag that can be checked via used to see if further processing should happen.
	 */
	UFUNCTION(BlueprintCallable, Category = AbstractEventData)
	virtual void Use()
	{
		bUsed = true;
	}

	/**
	 * Is the event used?
	 */
	UFUNCTION(BlueprintCallable, Category = AbstractEventData)
	virtual bool IsUsed()
	{
		return bUsed;
	}
	
};
