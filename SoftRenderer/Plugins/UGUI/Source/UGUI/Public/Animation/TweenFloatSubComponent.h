#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "TweenFloatSubComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UTweenFloatSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = TweenFloat)
	float From;

	UPROPERTY(EditAnywhere, Category = TweenFloat)
	float To;
	
protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	//~ End ITweenInterface Interface

protected:
	virtual float GetCurrent()
	{
		return 0.0f;
	}
};
