#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "ToggleGroupComponent.generated.h"

class UToggleComponent;

/**
 * A component that represents a group of UI.Toggles.
 *
 * When using a group reference the group from a UI.Toggle. Only one member of a group can be active at a time.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UToggleGroupComponent : public URectTransformComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(Transient)
	TArray<UToggleComponent*> Toggles;
	
	/**
	 * Is it allowed that no toggle is switched on?
	 *
	 * If this setting is enabled, pressing the toggle that is currently switched on will switch it off, so that no toggle is switched on. If this setting is disabled, pressing the toggle that is currently switched on will not change its state.
     * Note that even if allowSwitchOff is false, the Toggle Group will not enforce its constraint right away if no toggles in the group are switched on when the scene is loaded or when the group is instantiated. It will only prevent the user from switching a toggle off.
	 */
	UPROPERTY(EditAnywhere, Category = ToggleGroup)
	uint8 bAllowSwitchOff : 1;

public:
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	bool IsAllowSwitchOff() const
	{
		return bAllowSwitchOff;
	}

	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void SetAllowSwitchOff(bool bInAllowSwitchOff)
	{
		bAllowSwitchOff = bInAllowSwitchOff;
	}
	
public:
	/**
	 * Notify the group that the given toggle is enabled.
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void NotifyToggleOn(UToggleComponent* Toggle, bool bSendCallback = true);

	/**
	 * Unregister a toggle from the group.
	 *
	 * @param  Toggle  The toggle to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void UnregisterToggle(UToggleComponent* Toggle);

	/**
	 * Register a toggle with the toggle group so it is watched for changes and notified if another toggle in the group changes.
	 *
	 * @param  Toggle  The toggle to register with the group.
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void RegisterToggle(UToggleComponent* Toggle);

	/**
	 * Are any of the toggles on?
	 *
	 * @return  Are and of the toggles on?
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	bool AnyTogglesOn();

	/**
	 * Returns the toggles in this group that are active.
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void GetActiveToggles(TArray<UToggleComponent*>& OutToggles);

	/**
	 * Switch all toggles off.
	 *
	 * This method can be used to switch all toggles off, regardless of whether the allowSwitchOff property is enabled or not.
	 */
	UFUNCTION(BlueprintCallable, Category = ToggleGroup)
	void SetAllTogglesOff(bool bSendCallback = true);

private:
	bool ValidateToggleIsInGroup(UToggleComponent* Toggle) const;
	
};
