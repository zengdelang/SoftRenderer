#include "Animation/TweenAlphaSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/FloatTween.h"

UTweenAlphaSubComponent::UTweenAlphaSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenAlphaSubComponent::InternalStartTween()
{
	GetRenderer();
	FFloatTween* FloatTween = static_cast<FFloatTween*>(TweenRunner->GetTweenValue());
	if (nullptr != FloatTween)
	{
		FloatTween->OnFloatTweenCallback.Clear();
		FloatTween->OnFloatTweenCallback.AddUObject(this, &UTweenAlphaSubComponent::UpdateAlpha);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenAlphaSubComponent::GetRenderer()
{
	if (!IsValid(Renderer))
	{
		Renderer = Cast<UCanvasRendererSubComponent>(GetComponent(UCanvasRendererSubComponent::StaticClass(), true));
	}
}

void UTweenAlphaSubComponent::UpdateAlpha(float InAlpha)
{
	if (IsEnabledInHierarchy() && IsValid(AttachTransform))
	{
		AttachTransform->SetRenderOpacity(InAlpha);
	}

}