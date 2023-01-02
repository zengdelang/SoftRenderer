#pragma once

#include "CoreMinimal.h"
#include "TweenDefines.h"
#include "TweenRunner.h"
#include "TweenInterface.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "TweenBaseSubComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UTweenBaseSubComponent : public UBehaviourSubComponent, public ITweenInterface
{
	GENERATED_UCLASS_BODY()

	friend class FTweenUtility;
	friend class UTweenPlayerSubComponent;
	
protected:
	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Group Name"))
	FString TweenGroupName;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Group"))
	int32 TweenGroup;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Group Index"))
	int32 TweenGroupIndex;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Auto Play"))
	uint8 bAutoPlay : 1;
	
	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "From Current"))
	uint8 bFromCurrent : 1;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Ignore TimeScale"))
	uint8 bIgnoreTimeScale : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "EditPreview"))
	uint8 bEditPreview : 1;
#endif

	uint8 bTweenForward : 1;
	uint8 bIgnoreDelay : 1;
	uint8 bIsPlaying : 1;
	uint8 bNeedSetTimer : 1;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Duration"))
	float Duration;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Start Delay"))
	float StartDelay;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Easing Func"))
	ETweenEasingFunc EasingFunc;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Curve"))
	UCurveFloat* AnimationCurve;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Play Style"))
	ETweenPlayStyle Style;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "End Way"))
	ETweenEndWay EndWay;

	FTimerHandle WaitForRunTimerHandle;
	TSharedPtr<FTweenRunner> TweenRunner;
	FTimerHandle TimerHandle;
	
public:
	UPROPERTY(BlueprintAssignable, Category = TweenBase)
	FOnTweenFinished OnFinished;

protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourSubComponent Interface

protected:
	void CheckHasBegunPlay();
	void StartTween();

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override { }
	virtual void InternalPlay() override { }
	virtual void InternalGoToBegin() override;
	virtual void InternalGoToEnd() override;
	virtual void InternalToggle() override { }
	virtual void InternalStartTween() override { }
	//~ End ITweenInterface Interface

protected:
	void OnTweenRunnerFinished();
	
public:
	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void Play()
	{
		bTweenForward = true;
		bIgnoreDelay = false;
		InternalPlay();
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void PlayReverse()
	{
		bTweenForward = false;
		bIgnoreDelay = false;
		InternalPlay();
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	bool IsPlaying()
	{
		return bIsPlaying;
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void Toggle()
	{
		bTweenForward = !bTweenForward;
		bIgnoreDelay = false;
		TweenRunner->ResetElapsedTime();
		InternalToggle();
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void Stop()
	{
		if (TweenRunner.IsValid())
		{
			TweenRunner->StopTween();
		}

		if (WaitForRunTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(WaitForRunTimerHandle);
		}

		if (ETweenEndWay::TweenEndWay_GoToBegin == EndWay)
		{
			if (bTweenForward)
			{
				InternalGoToBegin();
			}
			else
			{
				InternalGoToEnd();
			}
		}
		else if (ETweenEndWay::TweenEndWay_GoToEnd == EndWay)
		{
			if (bTweenForward)
			{
				InternalGoToEnd();
			}
			else
			{
				InternalGoToBegin();
			}
		}
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void GoToBegin()
	{
		InternalGoToBegin();
	}

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void GoToEnd()
	{
		InternalGoToEnd();
	}

	bool IsFromCurrent() const
	{
		return bFromCurrent;
	}

	ETweenEasingFunc GetEasingFunc() const
	{
		return EasingFunc;
	}
};
