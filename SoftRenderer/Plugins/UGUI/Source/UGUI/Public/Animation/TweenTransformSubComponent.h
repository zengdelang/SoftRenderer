#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "TweenTransformSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Transform", HideEnableCheckBox = true))
class UGUI_API UTweenTransformSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

public:

protected:
	UPROPERTY(EditAnywhere, Category = TweenPosition, meta = (DisplayName = "Use Screen Position"))
	uint8 bUseScreenPosition : 1;

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	virtual void InternalStartTween() override;
	//~ End ITweenInterface Interface

	virtual FTweenTransform GetCurrent()
	{ 
		FTweenTransform Tmp;
		if (IsValid(AttachTransform))
		{
			Tmp.Position = AttachTransform->GetAnchoredPosition3D();
			Tmp.Rotation = AttachTransform->GetLocalRotation();
			Tmp.Scale = AttachTransform->GetLocalScale();
		}
		return Tmp;
	}

protected:
	UPROPERTY(EditAnywhere, Category = TweenTransform)
	FTweenTransform From;

	UPROPERTY(EditAnywhere, Category = TweenTransform)
	FTweenTransform To;

private:
	void UpdateTweenValue(FTweenTransform& InOutTransform);
	void UpdateTransform(FTweenTransform InTransform);



};