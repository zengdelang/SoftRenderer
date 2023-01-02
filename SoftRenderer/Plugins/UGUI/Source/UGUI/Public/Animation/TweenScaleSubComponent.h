#pragma once

#include "CoreMinimal.h"
#include "TweenVectorSubComponent.h"
#include "TweenScaleSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Scale", HideEnableCheckBox = true))
class UGUI_API UTweenScaleSubComponent : public UTweenVectorSubComponent
{
	GENERATED_UCLASS_BODY()

public:

protected:
	//~ Begin ITweenInterface Interface
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual FVector GetCurrent() const override
	{
		if (IsValid(AttachTransform))
		{
			return AttachTransform->GetLocalScale();
		}
		return FVector::OneVector;
	}

private:
	void UpdateLocalScale(FVector InVector);


};