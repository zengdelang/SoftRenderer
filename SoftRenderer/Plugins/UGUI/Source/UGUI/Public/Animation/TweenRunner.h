#pragma once

#include "CoreMinimal.h"
#include "ITweenValue.h"
#include "TweenDefines.h"

DECLARE_MULTICAST_DELEGATE(FOnTweenRunnerFinished);

class FTweenRunner;

class UGUI_API FTweenRunner : public TSharedFromThis<FTweenRunner>
{
protected:
	TSharedPtr<ITweenValue> TweenValue;
	ETweenEasingFunc EasingFunc;
	UCurveFloat* AnimationCurve;
	TWeakObjectPtr<UWorld> World;

	FTimerHandle TimerHandle;
	float ElapsedTime = 0;
	float LastTimeSeconds = 0;

	uint8 bNeedSetTimer : 1;

public:
	FOnTweenRunnerFinished OnTweenRunnerFinished;
	
public:
	FTweenRunner();
	
	void Init(TSharedPtr<ITweenValue> InTweenValue, ETweenEasingFunc InEasingFunc = ETweenEasingFunc::TweenEasingFunc_Linear, UCurveFloat* InAnimationCurve = nullptr);
	ITweenValue* GetTweenValue() const;
	
public:
	void StartTween(UWorld* InWorld, bool bIsValid, bool bResetElapsedTime = true);
	void StopTween();
	void ResetElapsedTime();

protected:
	void UpdateTweenValue();
	
};
