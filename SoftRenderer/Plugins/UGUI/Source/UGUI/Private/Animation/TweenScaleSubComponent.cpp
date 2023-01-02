#include "Animation/TweenScaleSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/VectorTween.h"
#include "Core/Layout/RectTransformComponent.h"

UTweenScaleSubComponent::UTweenScaleSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenScaleSubComponent::InternalStartTween()
{
	FVectorTween* VectorTween = static_cast<FVectorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != VectorTween)
	{
		VectorTween->OnVectorTweenCallback.Clear();
		VectorTween->OnVectorTweenCallback.AddUObject(this, &UTweenScaleSubComponent::UpdateLocalScale);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenScaleSubComponent::UpdateLocalScale(FVector InVector)
{
	if (IsEnabledInHierarchy() && IsValid(AttachTransform))
	{
		AttachTransform->SetLocalScale(InVector);
	}

}