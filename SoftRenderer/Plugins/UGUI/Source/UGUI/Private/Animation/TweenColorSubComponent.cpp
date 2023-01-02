#include "Animation/TweenColorSubComponent.h"
#include "Animation/TweenRunner.h"
#include "Animation/ColorTween.h"

UTweenColorSubComponent::UTweenColorSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UTweenColorSubComponent::InitTweenRunner()
{
	TweenRunner->Init(MakeShareable(new FColorTween()), EasingFunc, AnimationCurve);
}

void UTweenColorSubComponent::InternalPlay()
{
	FColorTween* ColorTween = static_cast<FColorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != ColorTween)
	{
		if (bTweenForward)
		{
			ColorTween->StartColor = bFromCurrent ? GetCurrent() : From;
			ColorTween->TargetColor = To;
		}
		else
		{
			ColorTween->StartColor = To;
			ColorTween->TargetColor = bFromCurrent ? GetCurrent() : From;
		}

		StartTween();
	}

}

void UTweenColorSubComponent::InternalToggle()
{
	FColorTween* ColorTween = static_cast<FColorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != ColorTween)
	{
		FLinearColor Tmp = ColorTween->StartColor;
		ColorTween->StartColor = ColorTween->TargetColor;
		ColorTween->TargetColor = Tmp;

		StartTween();
	}
}

void UTweenColorSubComponent::InternalStartTween()
{
	GetRenderer();
	FColorTween* ColorTween = static_cast<FColorTween*>(TweenRunner->GetTweenValue());
	if (nullptr != ColorTween)
	{
		ColorTween->OnColorTweenCallback.Clear();
		ColorTween->OnColorTweenCallback.AddUObject(this, &UTweenColorSubComponent::UpdateColor);
		TweenRunner->StartTween(GetWorld(), IsEnabledInHierarchy(), false);
	}

}

void UTweenColorSubComponent::GetRenderer()
{
	if (!IsValid(Renderer))
	{
		Renderer = Cast<UCanvasRendererSubComponent>(GetComponent(UCanvasRendererSubComponent::StaticClass(), true));
	}
}

void UTweenColorSubComponent::UpdateColor(FLinearColor InColor)
{
	if (IsEnabledInHierarchy() && IsValid(Renderer))
	{
		Renderer->SetColor(InColor);
	}
}