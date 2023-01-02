#pragma once

#include "CoreMinimal.h"
#include "SelectableComponent.h"
#include "EventSystem/Interfaces/PointerClickHandlerInterface.h"
#include "EventSystem/Interfaces/SubmitHandlerInterface.h"
#include "ButtonComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonComponentClickedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonComponentDoubleClickedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonComponentPressedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonComponentReleasedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnButtonComponentHoverEvent);

/**
 * Button that's meant to work with mouse or touch-based devices.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UButtonComponent : public USelectableComponent,
	public IPointerClickHandlerInterface, public ISubmitHandlerInterface
{
	GENERATED_UCLASS_BODY()

public:
	/** Called when the button is clicked */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentClickedEvent OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentDoubleClickedEvent OnDoubleClicked;

	/** Called when the button is pressed */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentPressedEvent OnPressed;

	/** Called when the button is released */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentReleasedEvent OnReleased;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentHoverEvent OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnButtonComponentHoverEvent OnUnhovered;
	
protected:
	TSharedPtr<FTweenRunner> TweenRunner;
	
public:
	virtual void OnDisable() override;
	
	/** 
     * Call all registered IPointerClickHandlers.
     * Register button presses using the IPointerClickHandler. You can also use it to tell what type of click happened (left, right etc.).
     * Make sure your Scene has an EventSystem.
	 *
	 * @param  EventData  Pointer Data associated with the event. Typically by the event system.
	 */
	virtual void OnPointerClick(UPointerEventData* EventData) override;

private:
	void Press() const;
	
public:
	virtual void OnSubmit(UBaseEventData* EventData) override;

private:
	void OnFinishSubmit();

public:
	virtual void OnPointerDown(UPointerEventData* EventData) override;

	virtual void OnPointerUp(UPointerEventData* EventData) override;

	virtual void OnPointerEnter(UPointerEventData* EventData) override;

	virtual void OnPointerExit(UPointerEventData* EventData) override;
	
};
