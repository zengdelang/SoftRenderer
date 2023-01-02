#pragma once

#include "CoreMinimal.h"
#include "AbstractEventData.h"
#include "BaseEventData.generated.h"

class UEventSystemComponent;
class UBaseInputModuleSubComponent;

/**
 * A class that contains the base event data that is common to all event types in the new EventSystem.
 */
UCLASS(BlueprintType)
class UGUI_API UBaseEventData : public UAbstractEventData
{
    GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	UEventSystemComponent* EventSystem = nullptr;

public:
	void SetEventSystem(UEventSystemComponent* InEventSystem)
	{
		EventSystem = InEventSystem;
	}

public:
	/**
	 * A reference to the BaseInputModule that sent this event.
	 */
	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	UBaseInputModuleSubComponent* GetCurrentInputModule() const;

	/**
	 * The object currently considered selected by the EventSystem.
	 */
	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	USceneComponent* GetSelectedObject() const;

	UFUNCTION(BlueprintCallable, Category = MaskableGraphic)
	void SetSelectedObject(USceneComponent* InSelectedObject);

};
