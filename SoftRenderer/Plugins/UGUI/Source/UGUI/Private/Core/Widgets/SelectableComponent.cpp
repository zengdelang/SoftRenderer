#include "Core/Widgets/SelectableComponent.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "EventSystem/EventSystemComponent.h"
#include "PaperSprite.h"
#include "Core/Widgets/ImageElementInterface.h"
#include "Core/Widgets/ImageSubComponent.h"

/////////////////////////////////////////////////////
// USelectableComponent

USelectableComponent::USelectableComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    Transition = ESelectableTransition::Transition_ColorTint;

    bIsPointerInside = false;
    bIsPointerDown = false;
    bHasSelection = false;

    const auto CanvasRenderSubComp = CreateDefaultSubobject<UCanvasRendererSubComponent>(TEXT("CanvasRendererSubComp0"));
    if (CanvasRenderSubComp)
    {
        SubComponents.Emplace(CanvasRenderSubComp);
    }
	
    const auto ImageSubComp = CreateDefaultSubobject<UImageSubComponent>(TEXT("ImageSubComp0"));
    if (ImageSubComp)
    {
        SubComponents.Emplace(ImageSubComp);
    }
}

void USelectableComponent::Awake()
{
    Super::Awake();

    if (!TargetGraphic)
    {
        TargetGraphic = GetComponentByInterface(UGraphicElementInterface::StaticClass(), true);
    }     
}

void USelectableComponent::OnEnable()
{
    Super::OnEnable();

    /*if (s_IsDirty)
        RemoveInvalidSelectables();

    m_WillRemove = false;

    if (s_SelectableCount == s_Selectables.Length)
    {
        Selectable[] temp = new Selectable[s_Selectables.Length * 2];
        Array.Copy(s_Selectables, temp, s_Selectables.Length);
        s_Selectables = temp;
    }
    s_Selectables[s_SelectableCount++] = this;*/

	bIsPointerDown = false;
    DoStateTransition(GetCurrentSelectionState(), true);
}

void USelectableComponent::OnDisable()
{
    /*
    m_WillRemove = true;
    s_IsDirty = true;
    */

    InstantClearState();
    Super::OnDisable();
}

void USelectableComponent::OnInteractableStateChanged()
{
    Super::OnInteractableStateChanged();
    
    if (!IsInteractableInHierarchy())
    {
        const auto World = GetWorld();
        const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
        if (IsValid(EventSystem) && EventSystem->GetCurrentSelectedGameObject() == this)
        {
            EventSystem->SetSelectedGameObject(nullptr);
        }
    }
    
    OnSetProperty();
}

#if WITH_EDITORONLY_DATA

void USelectableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // OnValidate can be called before OnEnable, this makes it unsafe to access other components
	// since they might not have been initialized yet.
    if (IsActiveAndEnabled())
    {
        if (!IsInteractableInHierarchy())
        {
            const auto World = GetWorld();
            const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
            if (IsValid(EventSystem) && EventSystem->GetCurrentSelectedGameObject() == this)
            {
                EventSystem->SetSelectedGameObject(nullptr);
            }
        }

        // Need to clear out the override image on the target...
        DoSpriteSwap(nullptr);

        // If the transition mode got changed, we need to clear all the transitions, since we don't know what the old transition mode was.
        StartColorTween(FLinearColor::White, true);
 
        // And now go to the right state.
        DoStateTransition(GetCurrentSelectionState(), true);
    }
}

#endif

void USelectableComponent::OnSetProperty()
{
#if WITH_EDITOR
    if (GetWorld() && !GetWorld()->IsGameWorld())
    {
        DoStateTransition(GetCurrentSelectionState(), true);
	    
    }
    else
    {
        DoStateTransition(GetCurrentSelectionState(), false); 
    }
#else
    DoStateTransition(GetCurrentSelectionState(), false);
#endif
}

void USelectableComponent::InstantClearState()
{
    bIsPointerInside = false;
    bIsPointerDown = false;
    bHasSelection = false;

    switch (Transition)
    {
    case ESelectableTransition::Transition_ColorTint:
        StartColorTween(FLinearColor::White, true);
        break;
    case ESelectableTransition::Transition_SpriteSwap:
        DoSpriteSwap(nullptr);
        break;
    case ESelectableTransition::Transition_ColorTintAndSpriteSwap:
        StartColorTween(FLinearColor::White, true);
        DoSpriteSwap(nullptr);
        break;
    default:
        break;
    }
}

void USelectableComponent::StartColorTween(FLinearColor TargetColor, bool bInstant) const
{
    if (!TargetGraphic)
        return;

    TargetGraphic->CrossFadeColor(TargetColor, bInstant ? 0 : ColorSpriteBlock.FadeDuration, true, true);
}

void USelectableComponent::DoSpriteSwap(UPaperSprite* NewSprite) const
{
    IImageElementInterface* ImageElem = Cast<IImageElementInterface>(TargetGraphic.GetObject());
    if (!ImageElem)
        return;
	
    ImageElem->SetOverrideSprite(NewSprite);
}

void USelectableComponent::DoStateTransition(ESelectableSelectionState InState, bool bInstant)
{
    if (!IsActiveAndEnabled())
        return;

    FLinearColor TintColor;
    UPaperSprite* TransitionSprite;
    switch (InState)
    {
    case ESelectableSelectionState::SelectionState_Normal:
        TintColor = ColorSpriteBlock.NormalColor;
        TransitionSprite = ColorSpriteBlock.NormalSprite;
        break;
    case ESelectableSelectionState::SelectionState_Highlighted:
        TintColor = ColorSpriteBlock.HighlightedColor;
        TransitionSprite = ColorSpriteBlock.HighlightedSprite;
        break;
    case ESelectableSelectionState::SelectionState_Pressed:
        TintColor = ColorSpriteBlock.PressedColor;
        TransitionSprite = ColorSpriteBlock.PressedSprite;
        break;
    case ESelectableSelectionState::SelectionState_Selected:
        TintColor = ColorSpriteBlock.SelectedColor;
        TransitionSprite = ColorSpriteBlock.SelectedSprite;
        break;
    case ESelectableSelectionState::SelectionState_Disabled:
        TintColor = ColorSpriteBlock.DisabledColor;
        TransitionSprite = ColorSpriteBlock.DisabledSprite;
        break;
    default:
        TintColor = FLinearColor::Black;
        TransitionSprite = nullptr;
        break;
    }

    switch (Transition)
    {
    case ESelectableTransition::Transition_ColorTint:
        StartColorTween(TintColor * ColorSpriteBlock.ColorMultiplier, bInstant);
        break;
    case ESelectableTransition::Transition_SpriteSwap:
        DoSpriteSwap(TransitionSprite);
        break;
    case ESelectableTransition::Transition_ColorTintAndSpriteSwap:
        StartColorTween(TintColor * ColorSpriteBlock.ColorMultiplier, bInstant);
        DoSpriteSwap(TransitionSprite);
        break;
    default:
        break;
    }
}

void USelectableComponent::OnPointerDown(UPointerEventData* EventData)
{
    if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
        return;

    // Selection tracking
    /*if (IsInteractable() && navigation.mode != Navigation.Mode.None && EventSystem.current != null)
        EventSystem.current.SetSelectedGameObject(gameObject, eventData);*/

    bIsPointerDown = true;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::OnPointerUp(UPointerEventData* EventData)
{
    if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
        return;

    bIsPointerDown = false;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::OnPointerEnter(UPointerEventData* EventData)
{
    bIsPointerInside = true;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::OnPointerExit(UPointerEventData* EventData)
{
    bIsPointerInside = false;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::OnSelect(UBaseEventData* EventData)
{
    bHasSelection = true;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::OnDeselect(UBaseEventData* EventData)
{
    bHasSelection = false;
    EvaluateAndTransitionToSelectionState();
}

void USelectableComponent::Select()
{
    const auto World = GetWorld();
    const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
    if (!IsValid(EventSystem) || EventSystem->AlreadySelecting())
        return;

    EventSystem->SetSelectedGameObject(this);
}

USceneComponent* USelectableComponent::FindChildBehaviourComponent(const TArray<int32>& ChildPath) const
{
    const USceneComponent* CurComp = this;
    USceneComponent* TargetComp = nullptr;
	
    for (const auto& Index : ChildPath)
    {
        auto ChildAttachChildren = CurComp->GetAttachChildren();

        if (ChildAttachChildren.IsValidIndex(Index))
        {
            if (IsValid(ChildAttachChildren[Index]))
            {
                TargetComp = ChildAttachChildren[Index];
                CurComp = TargetComp;
            }
            else
            {
                break;
            }
        }
    }
	
    return TargetComp;
}

/////////////////////////////////////////////////////
