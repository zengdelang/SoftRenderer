#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "TweenRotationSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Rotation", HideEnableCheckBox = true))
class UGUI_API UTweenRotationSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = TweenRotation)
	void SetFrom(FRotator InFrom)
	{
		From = InFrom;
	}

	UFUNCTION(BlueprintCallable, Category = TweenRotation)
	void SetTo(FRotator InTo)
	{
		To = InTo;
	}

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual FRotator GetCurrent() const
	{
		if (IsValid(AttachTransform))
		{
			return AttachTransform->GetLocalRotation();
		}
		return FRotator::ZeroRotator;
	}

protected:
	UPROPERTY(EditAnywhere, Category = TweenRotation)
	FRotator From = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = TweenRotation)
	FRotator To = FRotator::ZeroRotator;

private:
	void UpdateLocalRotation(FRotator InRotator);


};