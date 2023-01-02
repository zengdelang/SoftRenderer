#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"
#include "EventSystem/Interfaces/SelectHandlerInterface.h"
#include "FocusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLostFocusEvent);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (UI), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UFocusComponent : public URectTransformComponent, public ISelectHandlerInterface
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnLostFocusEvent OnLostFocus;
	
public:
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;

public:
	virtual void OnSelect(UBaseEventData* EventData) override {}
	
private:
	void OnSelectedGameObjectChanged(const UWorld* InWorld, const USceneComponent* InNewSelectedObj);
	
};

