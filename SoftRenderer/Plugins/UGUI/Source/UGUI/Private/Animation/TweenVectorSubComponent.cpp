#include "Animation/TweenVectorSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/VectorTween.h"

UTweenVectorSubComponent::UTweenVectorSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenVectorSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FVectorTween()), EasingFunc, AnimationCurve);
}

void UTweenVectorSubComponent::InternalPlay()
{
	FVectorTween* VectorTween = static_cast<FVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != VectorTween)
	{
		if (bTweenForward)
		{
			VectorTween->Start = bFromCurrent ? GetCurrent() : GetFrom();
			VectorTween->Target = GetTo();
		}
		else
		{
			VectorTween->Start = GetTo();
			VectorTween->Target = bFromCurrent ? GetCurrent() : GetFrom();
		}

		StartTween();
	}

}

void UTweenVectorSubComponent::InternalToggle()
{
	FVectorTween* VectorTween = static_cast<FVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != VectorTween)
	{
		FVector Tmp = VectorTween->Start;
		VectorTween->Start = VectorTween->Target;
		VectorTween->Target = Tmp;

		StartTween();
	}
}