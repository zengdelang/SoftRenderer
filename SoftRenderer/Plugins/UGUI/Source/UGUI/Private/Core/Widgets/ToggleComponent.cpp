#include "Core/Widgets/ToggleComponent.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Render/CanvasRendererSubComponent.h"
#include "Core/Widgets/ToggleGroupComponent.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UToggleComponent

UToggleComponent::UToggleComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, Group(nullptr)
{
	ToggleTransition = EToggleTransition::ToggleTransition_Fade;

	bIsOn = true;

	BackgroundPath.Emplace(0);

	CheckMarkPath.Emplace(0);
	CheckMarkPath.Emplace(0);
}

void UToggleComponent::Rebuild(ECanvasUpdate Executing)
{           
#if WITH_EDITOR
	if (Executing == ECanvasUpdate::CanvasUpdate_Prelayout)
		OnValueChanged.Broadcast(bIsOn);
#endif
}

void UToggleComponent::Awake()
{
	Super::Awake();

	SetBackground(FindChildBehaviourComponent(BackgroundPath));
	SetCheckMark(FindChildBehaviourComponent(CheckMarkPath));

	auto Parent = GetAttachParent();
	while(!IsValid(Group) && IsValid(Parent))
	{
		const auto ToggleGroupComp = Cast<UToggleGroupComponent>(Parent);
		if (IsValid(ToggleGroupComp))
		{
			Group = ToggleGroupComp;
			break;
		}		
		Parent = Parent->GetAttachParent();
	}
}

void UToggleComponent::OnEnable()
{
	Super::OnEnable();
	
	SetToggleGroup(Group, false);
	PlayEffect(true);
}

void UToggleComponent::OnDisable()
{
	SetToggleGroup(nullptr, false);
	Super::OnDisable();
}

#if WITH_EDITORONLY_DATA

void UToggleComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!HasAnyFlags(EObjectFlags::RF_ArchetypeObject))
	{
		FCanvasUpdateRegistry::RegisterCanvasElementForLayoutRebuild(this);
	}
}

#endif


void UToggleComponent::OnPointerClick(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;
	
	InternalToggle();
}

void UToggleComponent::OnSubmit(UBaseEventData* EventData)
{
	InternalToggle();
}

void UToggleComponent::SetToggleGroup(UToggleGroupComponent* NewGroup, bool bSetMemberValue)
{
	// Sometimes IsActive returns false in OnDisable so don't check for it.
	// Rather remove the toggle too often than too little.
    if (IsValid(Group))
		Group->UnregisterToggle(this);

    // At runtime the group variable should be set but not when calling this method from OnEnable or OnDisable.
    // That's why we use the setMemberValue parameter.
    if (bSetMemberValue)
		Group = NewGroup;

    // Only register to the new group if this Toggle is active.
    if (IsValid(NewGroup) && IsActiveAndEnabled())
		NewGroup->RegisterToggle(this);

    // If we are in a new group, and this toggle is on, notify group.
    // Note: Don't refer to m_Group here as it's not guaranteed to have been set.
    if (IsValid(NewGroup) && IsOn() && IsActiveAndEnabled())
		NewGroup->NotifyToggleOn(this);
}

void UToggleComponent::Set(bool bValue, bool bSendCallback)
{
	if (IsOn() == bValue)
		return;

	// if we are in a group and set to true, do group logic
	bIsOn = bValue;
	if (IsValid(Group) && IsActiveAndEnabled())
	{
		if (bIsOn || (!Group->AnyTogglesOn() && !Group->IsAllowSwitchOff()))
		{
			bIsOn = true;
			Group->NotifyToggleOn(this, bSendCallback);
		}
	}

	// Always send event when toggle is clicked, even if value didn't change
	// due to already active toggle in a toggle group being clicked.
	// Controls like Dropdown rely on this.
	// It's up to the user to ignore a selection being set to the same value it already was, if desired.
	PlayEffect(ToggleTransition == EToggleTransition::ToggleTransition_None);
	if (bSendCallback)
	{
		OnValueChanged.Broadcast(bIsOn);
	}
}

void UToggleComponent::PlayEffect(bool bInstant) const
{
	if (!CheckMark)
		return;

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		const auto CanvasRendererComp = CheckMark->GetCanvasRenderer();
		if (IsValid(CanvasRendererComp))
		{
			CanvasRendererComp->SetAlpha(bIsOn ? 1 : 0);
		}
		return;
	}
#endif

	CheckMark->CrossFadeAlpha(IsOn() ? 1 : 0, bInstant ? 0 : 0.1f, true);
}

void UToggleComponent::InternalToggle()
{
	if (!IsActiveAndEnabled() || !IsInteractableInHierarchy())
		return;

	SetIsOn(!IsOn());
}

/////////////////////////////////////////////////////
