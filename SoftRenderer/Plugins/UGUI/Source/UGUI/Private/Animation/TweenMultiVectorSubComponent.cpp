#include "Animation/TweenMultiVectorSubComponent.h"
#include "Animation/TweenRunner.h"

UTweenMultiVectorSubComponent::UTweenMultiVectorSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenMultiVectorSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FMultiVectorTween()), EasingFunc, AnimationCurve);
	bPreTweenForward = true;
	UpdateWayPoints();
}

void UTweenMultiVectorSubComponent::UpdateWayPoints()
{
	FinalWayPoints = WayPoints;
	if (bFromCurrent)
	{
		FinalWayPoints.Insert(AttachTransform->GetAnchoredPosition3D(), 0);
	}
	FinalWayPoints = GetFinalWayPoints();
	ReverseFinalWayPoints = FinalWayPoints;
	Algo::Reverse(ReverseFinalWayPoints);
	UpdateTweenWayPoints(bPreTweenForward);
}

void UTweenMultiVectorSubComponent::InternalPlay()
{
	FMultiVectorTween* MultiVectorTween = static_cast<FMultiVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != MultiVectorTween && WayPoints.Num() > 1)
	{
		if (bTweenForward != bPreTweenForward)
		{
			UpdateTweenWayPoints(bTweenForward);
			bPreTweenForward = bTweenForward;
		}

		StartTween();
	}

}

void UTweenMultiVectorSubComponent::InternalToggle()
{
	InternalPlay();
}