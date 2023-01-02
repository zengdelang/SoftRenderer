#include "Animation/TweenPathSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/MultiVectorTween.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Animation/TweenUtility.h"

UTweenPathSubComponent::UTweenPathSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseScreenPosition = false;
}

void UTweenPathSubComponent::InternalStartTween()
{
	FMultiVectorTween* MultiVectorTween = static_cast<FMultiVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != MultiVectorTween)
	{
		MultiVectorTween->OnMultiVectorTweenCallback.Clear();
		MultiVectorTween->OnMultiVectorTweenCallback.AddUObject(this, &UTweenPathSubComponent::UpdateAnchoredPosition);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

TArray<FVector> UTweenPathSubComponent::GetFinalWayPoints()
{
	TArray<FVector> RetWayPoints;
	if (bUseScreenPosition)
	{
		for (int32 i = 0; i < FinalWayPoints.Num(); i++)
		{
			RetWayPoints.Emplace(FTweenUtility::ScreenPointToLocalPoint(AttachTransform, GetOwnerCanvas(), FinalWayPoints[i], GetWorld()));
		}
	}
	else
	{
		RetWayPoints = FinalWayPoints;
	}
	return RetWayPoints;
}

void UTweenPathSubComponent::UpdateAnchoredPosition(FVector InVector)
{
	if (IsEnabledInHierarchy() && IsValid(AttachTransform))
	{
		if (bUseScreenPosition)
		{
			AttachTransform->SetLocalLocation(InVector);
		}
		else
		{
			AttachTransform->SetAnchoredPosition3D(InVector);
		}
	}

}