#include "Animation/TweenTransformSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/TransformTween.h"
#include "Animation/TweenUtility.h"

UTweenTransformSubComponent::UTweenTransformSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenTransformSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FTransformTween()), EasingFunc, AnimationCurve);
}

void UTweenTransformSubComponent::InternalPlay()
{
	FTransformTween* TransformTween = static_cast<FTransformTween*>(TweenRunner->GetTweenValue());
	if (nullptr != TransformTween)
	{
		if (bTweenForward)
		{
			TransformTween->StartValue = bFromCurrent ? GetCurrent() : From;
			TransformTween->TargetValue = To;
		}
		else
		{
			TransformTween->StartValue = To;
			TransformTween->TargetValue = bFromCurrent ? GetCurrent() : From;
		}

		UpdateTweenValue(TransformTween->StartValue);
		UpdateTweenValue(TransformTween->TargetValue);

		StartTween();
	}

}

void UTweenTransformSubComponent::InternalToggle()
{
	FTransformTween* TransformTween = static_cast<FTransformTween*>(TweenRunner->GetTweenValue());
	if (nullptr != TransformTween)
	{
		FTweenTransform Tmp = TransformTween->StartValue;
		TransformTween->StartValue = TransformTween->TargetValue;
		TransformTween->TargetValue = Tmp;

		UpdateTweenValue(TransformTween->StartValue);
		UpdateTweenValue(TransformTween->TargetValue);

		StartTween();
	}
}

void UTweenTransformSubComponent::UpdateTweenValue(FTweenTransform& InOutTransform)
{
	if (bUseScreenPosition)
	{
		InOutTransform.Position = FTweenUtility::ScreenPointToLocalPoint(AttachTransform, GetOwnerCanvas(), InOutTransform.Position, GetWorld());
	}
}

void UTweenTransformSubComponent::InternalStartTween()
{
	FTransformTween* TransformTween = static_cast<FTransformTween*>(TweenRunner->GetTweenValue());
	if (nullptr != TransformTween)
	{
		TransformTween->OnTransformTweenCallback.Clear();
		TransformTween->OnTransformTweenCallback.AddUObject(this, &UTweenTransformSubComponent::UpdateTransform);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenTransformSubComponent::UpdateTransform(FTweenTransform InTransform)
{
	if (IsEnabledInHierarchy() && IsValid(AttachTransform))
	{
		if (bUseScreenPosition)
		{
			AttachTransform->SetLocalLocation(InTransform.Position);
		}
		else
		{
			AttachTransform->SetAnchoredPosition3D(InTransform.Position);
		}
		AttachTransform->SetLocalRotation(InTransform.Rotation);
		AttachTransform->SetLocalScale(InTransform.Scale);
	}
}