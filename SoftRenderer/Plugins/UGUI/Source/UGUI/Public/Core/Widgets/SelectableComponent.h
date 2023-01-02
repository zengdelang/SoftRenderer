#pragma once

#include "CoreMinimal.h"
#include "EventSystem/Interfaces/DeselectHandlerInterface.h"
#include "EventSystem/Interfaces/PointerDownHandlerInterface.h"
#include "EventSystem/Interfaces/PointerEnterHandlerInterface.h"
#include "EventSystem/Interfaces/PointerExitHandlerInterface.h"
#include "EventSystem/Interfaces/PointerUpHandlerInterface.h"
#include "EventSystem/Interfaces/SelectHandlerInterface.h"
#include "ColorSpriteBlock.h"
#include "GraphicElementInterface.h"
#include "EventSystem/EventSystemComponent.h"
#include "SelectableComponent.generated.h"

/**
 * An enumeration of selected states of objects
 */
UENUM(BlueprintType)
enum class ESelectableSelectionState : uint8
{
	/**
	 * The UI object can be selected.
	 */
	SelectionState_Normal UMETA(DisplayName = "Normal"),

	/**
	 * The UI object is highlighted.
	 */
	SelectionState_Highlighted UMETA(DisplayName = "Highlighted"),

	/**
	 * The UI object is pressed.
	 */
	SelectionState_Pressed UMETA(DisplayName = "Pressed"),

	/**
	 * The UI object is selected
	 */
	SelectionState_Selected UMETA(DisplayName = "Selected"),

	/**
	 * The UI object cannot be selected.
	 */
	SelectionState_Disabled UMETA(DisplayName = "Disabled"),
};

/**
 * Transition mode for a Selectable.
 */
UENUM(BlueprintType)
enum class ESelectableTransition : uint8
{
	/**
	 * No Transition.
	 */
	Transition_None UMETA(DisplayName = "None"),

	/**
	 * Use an color tint transition.
	 */
	Transition_ColorTint UMETA(DisplayName = "ColorTint"),

	/**
	 * Use a sprite swap transition.
	 */
	Transition_SpriteSwap UMETA(DisplayName = "SpriteSwap"),

	Transition_ColorTintAndSpriteSwap UMETA(DisplayName = "ColorTintAndSpriteSwap"),
};

/**
 * Simple selectable object - derived from to create a selectable control.
 */
UCLASS(Blueprintable, BlueprintType)
class UGUI_API USelectableComponent : public URectTransformComponent,
	public IPointerDownHandlerInterface, public IPointerUpHandlerInterface, public IPointerEnterHandlerInterface,
	public IPointerExitHandlerInterface, public ISelectHandlerInterface, public IDeselectHandlerInterface
{
	GENERATED_UCLASS_BODY()

	friend class UDropdownItemSubComponent;
	
protected:
	/**
	 * Type of the transition that occurs when the button state changes.
	 */
	UPROPERTY(EditAnywhere, Category = Selectable)
	ESelectableTransition Transition;
	
	UPROPERTY(EditAnywhere, Category = Selectable)
	FColorSpriteBlock ColorSpriteBlock;

protected:
	UPROPERTY(Transient)
	TScriptInterface<IGraphicElementInterface> TargetGraphic;
 
protected:
	uint8 bIsPointerInside : 1;
	uint8 bIsPointerDown : 1;
	uint8 bHasSelection : 1;

public:
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnInteractableStateChanged() override;
	//~ End BehaviourComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = Selectable)
	ESelectableTransition GetTransition() const
	{
		return Transition;
	}

	UFUNCTION(BlueprintCallable, Category = Selectable)
	void SetTransition(ESelectableTransition InTransition)
	{
		if (Transition != InTransition)
		{
			Transition = InTransition;
			OnSetProperty();
		}
	}

	UFUNCTION(BlueprintCallable, Category = Selectable)
	const FColorSpriteBlock& GetColorSpriteBlock() const
	{
		return ColorSpriteBlock;
	}

	UFUNCTION(BlueprintCallable, Category = Selectable)
	void SetColorSpriteBlock(FColorSpriteBlock InColorSpriteBlock)
	{
		ColorSpriteBlock = InColorSpriteBlock;
		OnSetProperty();
	}
	
	UFUNCTION(BlueprintCallable, Category = Selectable)
	const TScriptInterface<IGraphicElementInterface>& GetTargetGraphic() const
	{
		return TargetGraphic;
	}

	UFUNCTION(BlueprintCallable, Category = Selectable)
	void SetTargetGraphic(TScriptInterface<IGraphicElementInterface> InTargetGraphic)
	{
		if (TargetGraphic != InTargetGraphic)
		{
			TargetGraphic = InTargetGraphic;
			OnSetProperty();
		}
	}

private:
	void OnSetProperty();

protected:
	virtual void InstantClearState();

protected:
	ESelectableSelectionState GetCurrentSelectionState() const
	{
		if (!IsInteractableInHierarchy())
			return ESelectableSelectionState::SelectionState_Disabled;
		
		if (bIsPointerDown)
			return ESelectableSelectionState::SelectionState_Pressed;
		
		if (bHasSelection)
			return ESelectableSelectionState::SelectionState_Selected;
		
		if (bIsPointerInside)
			return ESelectableSelectionState::SelectionState_Highlighted;
		
		return ESelectableSelectionState::SelectionState_Normal;
	}

	/**
	 * Returns whether the selectable is currently 'highlighted' or not.
	 *
	 * Use this to check if the selectable UI element is currently highlighted.
	 */
	bool IsHighlighted() const
	{
		if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
			return false;
		return bIsPointerInside && !bIsPointerDown && !bHasSelection;
	}
	
	/**
	 * Whether the current selectable is being pressed.
	 */
	bool IsPressed() const
	{
		if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
			return false;
		return bIsPointerDown;
	}
	
	void StartColorTween(FLinearColor TargetColor, bool bInstant) const;
	
	void DoSpriteSwap(UPaperSprite* NewSprite) const;
	
	/**
	 * Transition the Selectable to the entered state.
	 *
	 * @param  InState  State to transition to
	 * @param  bInstant  Should the transition occur instantly
	 */
	virtual void DoStateTransition(ESelectableSelectionState InState, bool bInstant);

private:
	/**
	 * Change the button to the correct state
	 */
	void EvaluateAndTransitionToSelectionState()
	{
		if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
			return;

		DoStateTransition(GetCurrentSelectionState(), false);
	}

public:
	/**
	 * Evaluate current state and transition to pressed state.
	 */
	virtual void OnPointerDown(UPointerEventData* EventData) override;

	/**
	 * Evaluate eventData and transition to appropriate state.
	 */
	virtual void OnPointerUp(UPointerEventData* EventData) override;

	/**
	 * Evaluate current state and transition to appropriate state.
     * New state could be pressed or hover depending on pressed state.
	 */
	virtual void OnPointerEnter(UPointerEventData* EventData) override;

	/**
	 * Evaluate current state and transition to normal state.
	 */
	virtual void OnPointerExit(UPointerEventData* EventData) override;

	/**
	 * Set selection and transition to appropriate state.
	 */
	virtual void OnSelect(UBaseEventData* EventData) override;

	/**
	 * Unset selection and transition to appropriate state.
	 */
	virtual void OnDeselect(UBaseEventData* EventData) override;

public:
	virtual void Select();

protected:
	USceneComponent* FindChildBehaviourComponent(const TArray<int32>& ChildPath) const;

};
