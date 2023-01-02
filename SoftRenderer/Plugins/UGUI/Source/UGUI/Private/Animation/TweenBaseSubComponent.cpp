#include "Animation/TweenBaseSubComponent.h"
#include "Animation/ITweenValue.h"

/////////////////////////////////////////////////////
// UTweenBaseSubComponent

UTweenBaseSubComponent::UTweenBaseSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TweenGroup = 0;
	TweenGroupIndex = 0;
	
	bAutoPlay = false;
	bFromCurrent = false;
	bIgnoreTimeScale = true;

#if WITH_EDITOR
	bEditPreview = false;
#endif
	
	bTweenForward = true;
	bIgnoreDelay = false;
	bIsPlaying = false;
	bNeedSetTimer = false;
	
	Duration = 1.0f;
	StartDelay = 0.0f;

	EasingFunc = ETweenEasingFunc::TweenEasingFunc_Linear;
	
	AnimationCurve = nullptr;

	Style = ETweenPlayStyle::TweenPlayStyle_Once;
	EndWay = ETweenEndWay::TweenEndWay_GoToEnd;
}

void UTweenBaseSubComponent::Awake()
{
	Super::Awake();

	if (!TweenRunner.IsValid())
	{
		TweenRunner = MakeShareable(new FTweenRunner());
	}
	InitTweenRunner();

	ITweenValue* Tween = TweenRunner->GetTweenValue();
	Tween->bIgnoreTimeScale = bIgnoreTimeScale;
	Tween->StartDelay = StartDelay > 0.01f ? StartDelay : 0.01f;
	Tween->SetDuration(Duration);
	
	TweenRunner->OnTweenRunnerFinished.Clear();
	TweenRunner->OnTweenRunnerFinished.AddUObject(this, &UTweenBaseSubComponent::OnTweenRunnerFinished);
}

void UTweenBaseSubComponent::OnEnable()
{
	Super::OnEnable();

#if WITH_EDITOR
	// 编辑器模式直接运行
	if (bEditPreview && EWorldType::EditorPreview == GetWorld()->WorldType)
	{
		Play();
	}
#endif

	if (bAutoPlay)
	{
		if (EWorldType::EditorPreview != GetWorld()->WorldType)
		{
			// 非编辑器预览模式的话等游戏运行后再Play（目前发现如果游戏没有运行时Play会导致把带有使用屏幕坐标的TweenPosition的UI拖到场景时计算出来的位置非常大导致动画不正确）
			if (GetWorld()->HasBegunPlay())
			{
				Play();
			}
			else
			{
				bNeedSetTimer = true;
				GetWorld()->GetTimerManager().SetTimer(WaitForRunTimerHandle, this, &UTweenBaseSubComponent::CheckHasBegunPlay, 0.001f, false);
			}
		}
	}
}

void UTweenBaseSubComponent::OnDisable()
{
	Super::OnDisable();

	Stop();
}

void UTweenBaseSubComponent::CheckHasBegunPlay()
{
	if (GetWorld()->HasBegunPlay())
	{
		Play();

		bNeedSetTimer = false;
		if (WaitForRunTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(WaitForRunTimerHandle);
		}
	}

	if (bNeedSetTimer)
	{
		GetWorld()->GetTimerManager().SetTimer(WaitForRunTimerHandle, this, &UTweenBaseSubComponent::CheckHasBegunPlay, 0.001f, false);
	}
}

void UTweenBaseSubComponent::StartTween()
{
	bIsPlaying = true;

	if (!bIgnoreDelay && StartDelay > 0.01f)
	{
		bNeedSetTimer = false;
		if (TimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		}
		
		float FinalStartDelay = StartDelay;
		if (EWorldType::EditorPreview == GetWorld()->WorldType)
		{
			// TODO 编辑器预览时延迟时间会翻倍，这里除2临时处理让编辑器和运行时表现一致（待查找原因）
			FinalStartDelay = StartDelay / 2;
		}
		
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UTweenBaseSubComponent::InternalStartTween, FinalStartDelay, false);
	}
	else
	{
		InternalStartTween();
	}
}

void UTweenBaseSubComponent::InternalGoToBegin()
{
	TweenRunner->ResetElapsedTime();
	
	ITweenValue* TweenValue = TweenRunner->GetTweenValue();
	if (nullptr != TweenValue)
	{
		if (bTweenForward)
		{
			TweenValue->TweenValue(0.0f);
		}
		else
		{
			TweenValue->TweenValue(1.0f);
		}
	}
}

void UTweenBaseSubComponent::InternalGoToEnd()
{
	TweenRunner->ResetElapsedTime();
	
	ITweenValue* TweenValue = TweenRunner->GetTweenValue();
	if (nullptr != TweenValue)
	{
		if (bTweenForward)
		{
			TweenValue->TweenValue(1.0);
		}
		else
		{
			TweenValue->TweenValue(0.0f);
		}
	}
}

void UTweenBaseSubComponent::OnTweenRunnerFinished()
{
	if (Style == ETweenPlayStyle::TweenPlayStyle_Once)
	{
		bIsPlaying = false;
		Stop();
		OnFinished.Broadcast();
	}
	else if (Style == ETweenPlayStyle::TweenPlayStyle_Loop)
	{
		TweenRunner->ResetElapsedTime();
		bTweenForward = true;
		bIgnoreDelay = true;
		InternalPlay();
	}
	else if (Style == ETweenPlayStyle::TweenPlayStyle_PingPong)
	{
		TweenRunner->ResetElapsedTime();
		bTweenForward = !bTweenForward;
		bIgnoreDelay = true;
		InternalToggle();
	}
}

/////////////////////////////////////////////////////
