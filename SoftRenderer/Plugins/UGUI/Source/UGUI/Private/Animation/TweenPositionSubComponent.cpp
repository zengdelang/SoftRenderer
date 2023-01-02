#include "Animation/TweenPositionSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/VectorTween.h"
#include "Core/Layout/RectTransformComponent.h"

UTweenPositionSubComponent::UTweenPositionSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseScreenPosition = false;
}

void UTweenPositionSubComponent::InternalStartTween()
{
	FVectorTween* VectorTween = static_cast<FVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != VectorTween)
	{
		VectorTween->OnVectorTweenCallback.Clear();
		VectorTween->OnVectorTweenCallback.AddUObject(this, &UTweenPositionSubComponent::UpdateAnchoredPosition);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenPositionSubComponent::UpdateAnchoredPosition(FVector InVector)
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