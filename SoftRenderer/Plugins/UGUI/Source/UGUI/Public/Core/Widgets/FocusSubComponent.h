#pragma once

#include "CoreMinimal.h"
#include "FocusComponent.h"
#include "EventSystem/Interfaces/SelectHandlerInterface.h"
#include "FocusSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Focus"))
class UGUI_API UFocusSubComponent : public UBehaviourSubComponent, public ISelectHandlerInterface
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
	void OnSelectedGameObjectChanged(const UWorld* InWorld, const USceneComponent* InNewSelectedObj) const;
	
};
