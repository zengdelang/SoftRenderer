#include "Animation/TweenRotationSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/RotatorTween.h"
#include "Core/Layout/RectTransformComponent.h"

UTweenRotationSubComponent::UTweenRotationSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenRotationSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FRotatorTween()), EasingFunc);
}

void UTweenRotationSubComponent::InternalPlay()
{
	FRotatorTween* RotatorTween = static_cast<FRotatorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != RotatorTween)
	{
		if (bTweenForward)
		{
			RotatorTween->Start = bFromCurrent ? GetCurrent() : From;
			RotatorTween->Target = To;
		}
		else
		{
			RotatorTween->Start = To;
			RotatorTween->Target = bFromCurrent ? GetCurrent() : From;
		}

		StartTween();
	}

}

void UTweenRotationSubComponent::InternalToggle()
{
	FRotatorTween* RotatorTween = static_cast<FRotatorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != RotatorTween)
	{
		FRotator Tmp = RotatorTween->Start;
		RotatorTween->Start = RotatorTween->Target;
		RotatorTween->Target = Tmp;

		StartTween();
	}
}

void UTweenRotationSubComponent::InternalStartTween()
{
	FRotatorTween* RotatorTween = static_cast<FRotatorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != RotatorTween)
	{
		RotatorTween->OnRotatorTweenCallback.Clear();
		RotatorTween->OnRotatorTweenCallback.AddUObject(this, &UTweenRotationSubComponent::UpdateLocalRotation);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenRotationSubComponent::UpdateLocalRotation(FRotator InRotator)
{
	if (IsEnabledInHierarchy() && IsValid(AttachTransform))
	{
		AttachTransform->SetLocalRotation(InRotator);
	}

}