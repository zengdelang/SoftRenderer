#pragma once

#include "CoreMinimal.h"
#include "SelectableComponent.h"
#include "Core/CanvasElementInterface.h"
#include "EventSystem/Interfaces/PointerClickHandlerInterface.h"
#include "EventSystem/Interfaces/SubmitHandlerInterface.h"
#include "ToggleComponent.generated.h"

class UToggleGroupComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToggleValueChangedEvent, bool, bIsOn);

/**
 * Display settings for when a toggle is activated or deactivated.
 */
UENUM(BlueprintType)
enum class EToggleTransition : uint8
{
	/**
	 * Show / hide the toggle instantly
	 */
	ToggleTransition_None UMETA(DisplayName = "None"),

	/**
	 * Fade the toggle in / out smoothly.
	 */
	 ToggleTransition_Fade UMETA(DisplayName = "Fade"),
};

/**
 * A standard toggle that has an on / off state.
 *
 * The toggle component is a Selectable that controls a child graphic which displays the on / off state.
 * When a toggle event occurs a callback is sent to any registered listeners of UI.Toggle.NnValueChanged.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UToggleComponent : public USelectableComponent, public IPointerClickHandlerInterface, public ISubmitHandlerInterface, public ICanvasElementInterface
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * Transition mode for the toggle.
	 */
	UPROPERTY(EditAnywhere, Category = Toggle)
	EToggleTransition ToggleTransition;

protected:
	UPROPERTY(EditAnywhere, Category = Toggle)
	TArray<int32> BackgroundPath;

	UPROPERTY(EditAnywhere, Category = Toggle)
	TArray<int32> CheckMarkPath;
	
protected:
	/**
	 * Graphic the toggle should be working with.
	 */
	UPROPERTY(Transient)
	TScriptInterface<IGraphicElementInterface> CheckMark;
	
	UPROPERTY(Transient)
	UToggleGroupComponent* Group;

	UPROPERTY(EditAnywhere, Category = Toggle)
	uint8 bIsOn : 1;

public:
	/**
	 * Allow for delegate-based subscriptions for faster events than 'eventReceiver', and allowing for multiple receivers.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnToggleValueChangedEvent OnValueChanged;

public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return this; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface
	
public:	
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void OnPointerClick(UPointerEventData* EventData) override;
	virtual void OnSubmit(UBaseEventData* EventData) override;
	
public:
	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetToggleTransition(EToggleTransition InToggleTransition)
	{
		ToggleTransition = InToggleTransition;
	}
	
	UFUNCTION(BlueprintCallable, Category = Toggle)
	const TScriptInterface<IGraphicElementInterface>& GetCheckMark() const
	{
		return CheckMark;
	}

	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetCheckMark(TScriptInterface<IGraphicElementInterface> InCheckMark)
	{
		if (CheckMark != InCheckMark)
		{
			CheckMark = InCheckMark;
		}
	}

	UFUNCTION(BlueprintCallable, Category = Toggle)
	const TScriptInterface<IGraphicElementInterface>& GetBackground() const
	{
		return TargetGraphic;
	}

	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetBackground(TScriptInterface<IGraphicElementInterface> InBackground)
	{
		if (TargetGraphic != InBackground)
		{
			TargetGraphic = InBackground;
		}
	}

	UFUNCTION(BlueprintCallable, Category = Toggle)
	UToggleGroupComponent* GetGroup() const
	{
		return Group;
	}

	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetGroup(UToggleGroupComponent* InGroup)
	{
		if (Group != InGroup)
		{
			Group = InGroup;
			SetToggleGroup(Group, true);
			PlayEffect(true);
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = Toggle)
	bool IsOn() const
	{
		return bIsOn;
	}
	
	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetIsOn(bool bInIsOn)
	{
		Set(bInIsOn);
	}

	/**
	 * Set isOn without invoking onValueChanged callback.
	 *
	 * @param  bInIsOn  New Value for isOn.
	 */
	UFUNCTION(BlueprintCallable, Category = Toggle)
	void SetIsOnWithoutNotify(bool bInIsOn)
	{
		Set(bInIsOn, false);
	}
	
private:
	void SetToggleGroup(UToggleGroupComponent* NewGroup, bool bSetMemberValue);

	/**
	 * Whether the toggle is currently active.
	 */
	void Set(bool bValue, bool bSendCallback = true);

	/**
	 * Play the appropriate effect.
	 */
	void PlayEffect(bool bInstant) const;

	void InternalToggle();

};
