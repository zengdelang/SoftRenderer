#include "Animation/TweenRunner.h"
#include "Animation/TweenUtility.h"

FTweenRunner::FTweenRunner()
{
	EasingFunc = ETweenEasingFunc::TweenEasingFunc_Linear;
	AnimationCurve = nullptr;
	bNeedSetTimer = false;
}

void FTweenRunner::Init(TSharedPtr<ITweenValue> InTweenValue, ETweenEasingFunc InEasingFunc, UCurveFloat* InAnimationCurve)
{
	TweenValue = InTweenValue;
	EasingFunc = InEasingFunc;
	AnimationCurve = InAnimationCurve;
	check(TweenValue.IsValid());
}

ITweenValue* FTweenRunner::GetTweenValue() const
{
	return TweenValue.IsValid() ? TweenValue.Get() : nullptr;
}

void FTweenRunner::StartTween(UWorld* InWorld, bool bIsValid, bool bResetElapsedTime)
{
	check(TweenValue.IsValid())
	
	StopTween();

	if (!bIsValid || TweenValue->GetDuration() <= 0)
	{
		TweenValue->TweenValue(1.0f);
		return;
	}

	if (bResetElapsedTime)
	{
		ResetElapsedTime();
	}

	bNeedSetTimer = true;
	
	World = InWorld;
	if (World.IsValid())
	{
		LastTimeSeconds = TweenValue->IgnoreTimeScale() ? World->GetUnpausedTimeSeconds() : World->GetTimeSeconds();
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateSP(this, &FTweenRunner::UpdateTweenValue),
			0.001f, false);
	}	
}

void FTweenRunner::StopTween()
{
	bNeedSetTimer = false;
	
	if (TimerHandle.IsValid())
	{
		if (World.IsValid())
		{
			World->GetTimerManager().ClearTimer(TimerHandle);
		}
	}
}

void FTweenRunner::ResetElapsedTime()
{
	ElapsedTime = 0;
}

void FTweenRunner::UpdateTweenValue()
{
	check(TweenValue.IsValid())

	if (World.IsValid() && ElapsedTime < TweenValue->GetDuration())
	{
		const float CurrentTimeSeconds = TweenValue->IgnoreTimeScale() ? World->GetUnpausedTimeSeconds() : World->GetTimeSeconds();
		ElapsedTime += CurrentTimeSeconds - LastTimeSeconds;
		
		LastTimeSeconds = CurrentTimeSeconds;
		float Percentage = 0.0f;
		
		if (ETweenEasingFunc::TweenEasingFunc_CustomCurve != EasingFunc)
		{
			Percentage = FMath::Clamp(FTweenUtility::EaseAlpha(EasingFunc, ElapsedTime / TweenValue->GetDuration()), 0.0f, 1.0f);
		}
		else
		{
			if (IsValid(AnimationCurve))
			{
				Percentage = FMath::Clamp(AnimationCurve->GetFloatValue(ElapsedTime / TweenValue->GetDuration()), 0.0f, 1.0f);
			}
		}
		
		TweenValue->TweenValue(Percentage);
	}
	else
	{
		if (ETweenEasingFunc::TweenEasingFunc_CustomCurve != EasingFunc)
		{
			TweenValue->TweenValue(1.0f);
		}
		
		StopTween();
		OnTweenRunnerFinished.Broadcast();
	}

	if (bNeedSetTimer && World.IsValid())
	{
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateSP(this, &FTweenRunner::UpdateTweenValue),
				0.001f, false);
	}
}
