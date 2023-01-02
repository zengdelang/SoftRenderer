#pragma once

#include "CoreMinimal.h"
#include "TweenMultiVectorSubComponent.h"
#include "TweenPathSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Path", HideEnableCheckBox = true))
class UGUI_API UTweenPathSubComponent : public UTweenMultiVectorSubComponent
{
	GENERATED_UCLASS_BODY()

public:

protected:
	UPROPERTY(EditAnywhere, Category = TweenPosition, meta = (DisplayName = "Use Screen Position"))
	uint8 bUseScreenPosition : 1;

protected:
	//~ Begin ITweenInterface Interface
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual TArray<FVector> GetFinalWayPoints() override;

private:
	void UpdateAnchoredPosition(FVector InVector);


};