#pragma once

#include "CoreMinimal.h"
#include "TweenVectorSubComponent.h"
#include "TweenUtility.h"
#include "TweenPositionSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Position", HideEnableCheckBox = true))
class UGUI_API UTweenPositionSubComponent : public UTweenVectorSubComponent
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

	virtual FVector GetCurrent() const override
	{
		if (IsValid(AttachTransform))
		{
			return AttachTransform->GetAnchoredPosition3D();
		}
		return FVector::ZeroVector;
	}

	virtual FVector GetTo() override
	{
		return bUseScreenPosition ? FTweenUtility::ScreenPointToLocalPoint(AttachTransform, GetOwnerCanvas(), To, GetWorld()) : To;
	}

	virtual FVector GetFrom() override
	{
		return bUseScreenPosition ? FTweenUtility::ScreenPointToLocalPoint(AttachTransform, GetOwnerCanvas(), From, GetWorld()) : From;
	}

private:
	void UpdateAnchoredPosition(FVector InVector);


};