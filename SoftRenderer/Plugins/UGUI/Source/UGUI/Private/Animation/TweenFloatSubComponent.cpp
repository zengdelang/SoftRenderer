#include "Animation/TweenFloatSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/FloatTween.h"

/////////////////////////////////////////////////////
// UTweenFloatSubComponent

UTweenFloatSubComponent::UTweenFloatSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	From = 0.0f;
	To = 0.0f;
}

void UTweenFloatSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FFloatTween()), EasingFunc, AnimationCurve);
}

void UTweenFloatSubComponent::InternalPlay()
{
	FFloatTween* FloatTween = static_cast<FFloatTween*>(TweenRunner->GetTweenValue());
	if (nullptr != FloatTween)
	{
		if (bTweenForward)
		{
			FloatTween->StartValue = bFromCurrent ? GetCurrent() : From;
			FloatTween->TargetValue = To;
		}
		else
		{
			FloatTween->StartValue = To;
			FloatTween->TargetValue = bFromCurrent ? GetCurrent() : From;
		}

		StartTween();
	}

}

void UTweenFloatSubComponent::InternalToggle()
{
	FFloatTween* FloatTween = static_cast<FFloatTween*>(TweenRunner->GetTweenValue());
	if (nullptr != FloatTween)
	{
		float Tmp = FloatTween->StartValue;
		FloatTween->StartValue = FloatTween->TargetValue;
		FloatTween->TargetValue = Tmp;

		StartTween();
	}
}

/////////////////////////////////////////////////////
