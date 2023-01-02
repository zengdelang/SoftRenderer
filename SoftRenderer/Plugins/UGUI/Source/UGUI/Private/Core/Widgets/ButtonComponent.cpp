#include "Core/Widgets/ButtonComponent.h"
#include "Animation/FloatTween.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UButtonComponent

UButtonComponent::UButtonComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UButtonComponent::OnDisable()
{
	if (bIsPointerDown)
	{
		OnReleased.Broadcast();
	}
	Super::OnDisable();
}

void UButtonComponent::OnPointerClick(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (EventData->ClickCount % 2 == 0 && OnDoubleClicked.IsBound())
	{
		if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
			return;

		OnDoubleClicked.Broadcast();
	}
	else
	{
		Press();
	}
}

void UButtonComponent::Press() const
{
	if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
		return;

	OnClicked.Broadcast();
}

void UButtonComponent::OnSubmit(UBaseEventData* EventData)
{
	Press();

	if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
		return;

	DoStateTransition(ESelectableSelectionState::SelectionState_Pressed, false);

	if (!TweenRunner.IsValid())
	{
		TweenRunner = MakeShareable(new FTweenRunner());
		TweenRunner->Init(MakeShareable(new FFloatTween()));
	}

	FFloatTween* FloatTween = static_cast<FFloatTween*>(TweenRunner->GetTweenValue());
	FloatTween->SetDuration(ColorSpriteBlock.FadeDuration);
	FloatTween->StartValue = 0;
	FloatTween->TargetValue = 0;
	FloatTween->bIgnoreTimeScale = false;
	FloatTween->OnFloatTweenCallback.Clear();

	TweenRunner->OnTweenRunnerFinished.Clear();
	TweenRunner->OnTweenRunnerFinished.AddUObject(this, &UButtonComponent::OnFinishSubmit);
	TweenRunner->StartTween(GetWorld(), IsActiveAndEnabled());
}

void UButtonComponent::OnFinishSubmit()
{
	DoStateTransition(GetCurrentSelectionState(), false);
}

void UButtonComponent::OnPointerDown(UPointerEventData* EventData)
{
	Super::OnPointerDown(EventData);

	if (IsActiveAndEnabled() && IsInteractableInHierarchy() && IsValid(EventData) && EventData->Button == EPointerInputButton::InputButton_Left)
	{
		OnPressed.Broadcast();
	}
}

void UButtonComponent::OnPointerUp(UPointerEventData* EventData)
{
	Super::OnPointerUp(EventData);

	if (IsActiveAndEnabled() && IsInteractableInHierarchy() && IsValid(EventData) && EventData->Button == EPointerInputButton::InputButton_Left)
	{
		OnReleased.Broadcast();
	}
}

void UButtonComponent::OnPointerEnter(UPointerEventData* EventData)
{
	Super::OnPointerEnter(EventData);

	OnHovered.Broadcast();
}

void UButtonComponent::OnPointerExit(UPointerEventData* EventData)
{
	Super::OnPointerExit(EventData);

	OnUnhovered.Broadcast();
}

/////////////////////////////////////////////////////
