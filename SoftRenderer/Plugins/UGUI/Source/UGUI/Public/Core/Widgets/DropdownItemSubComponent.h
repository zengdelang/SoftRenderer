#pragma once

#include "CoreMinimal.h"
#include "EventSystem/Interfaces/CancelHandlerInterface.h"
#include "EventSystem/Interfaces/PointerEnterHandlerInterface.h"
#include "Text/TextElementInterface.h"
#include "DropdownItemSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Dropdown Item"))
class UGUI_API UDropdownItemSubComponent : public UBehaviourSubComponent, public IPointerEnterHandlerInterface, public ICancelHandlerInterface
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = DropdownItem)
	TArray<int32> TogglePath;
	
	/**
     * The Text component to hold the text of the item.
    */
	UPROPERTY(EditAnywhere, Category = DropdownItem)
	TArray<int32> ItemTextPath;

	/**
	 * The Image component to hold the image of the item
	 */
	UPROPERTY(EditAnywhere, Category = DropdownItem)
	TArray<int32> ItemImagePath;

protected:
	UPROPERTY(Transient)
	class UToggleComponent* Toggle;
	
	UPROPERTY(Transient)
	TScriptInterface<ITextElementInterface> ItemText;

	UPROPERTY(Transient)
	TScriptInterface<IImageElementInterface> ItemImage;

	int32 Index;
	
	UPROPERTY(Transient)
	class UDropdownComponent* Dropdown;

public:
	virtual void OnPointerEnter(UPointerEventData* EventData) override;
	virtual void OnCancel(UBaseEventData* EventData) override;

protected:
	UFUNCTION()
	void OnToggleValueChanged(bool bIsOn);

public:
	void SetDropdown(class UDropdownComponent* InDropdown);
	void SetData(int32 InIndex, bool bIsOn, const struct FOptionData& Data);

protected:
	USceneComponent* FindChildBehaviourComponent(const TArray<int32>& ChildPath) const;
	
};
