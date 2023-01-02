#include "Core/Widgets/FocusSubComponent.h"
#include "EventSystem/EventSystemComponent.h"

/////////////////////////////////////////////////////
// UFocusSubComponent

UFocusSubComponent::UFocusSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UFocusSubComponent::Awake()
{
    Super::Awake();
    
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        UEventSystemComponent::OnSelectedGameObjectChangedEvent.AddUObject(this, &UFocusSubComponent::OnSelectedGameObjectChanged);
    }
#else
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.AddUObject(this, &UFocusSubComponent::OnSelectedGameObjectChanged);
#endif
}

void UFocusSubComponent::OnEnable()
{
    Super::OnEnable();

#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        UEventSystemComponent::OnSelectedGameObjectChangedEvent.Broadcast(GetWorld(), AttachTransform);
    }
#else
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.Broadcast(GetWorld(), AttachTransform);
#endif
}

void UFocusSubComponent::OnDisable()
{
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        OnLostFocus.Broadcast();
    }
#else
    OnLostFocus.Broadcast();
#endif
    
    Super::OnDisable();
}

void UFocusSubComponent::OnDestroy()
{
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.RemoveAll(this);
    Super::OnDestroy();
}

void UFocusSubComponent::OnSelectedGameObjectChanged(const UWorld* InWorld, const USceneComponent* InNewSelectedObj) const
{
    if (!IsEnabledInHierarchy())
    {
        return;
    }
    
    const auto World = GetWorld();
    if (!IsValid(World) || World != InWorld)
    {
        return;
    }

    bool bLostFocus = true;

    if (IsValid(InNewSelectedObj))
    {
        auto SceneComp = InNewSelectedObj;
        while (IsValid(SceneComp))
        {
            if (SceneComp == AttachTransform)
            {
                bLostFocus = false;
                break;
            }
            SceneComp = SceneComp->GetAttachParent();
        }
    }

    if (bLostFocus)
    {
        UBehaviourComponent* BehaviourAttachTransform = Cast<UBehaviourComponent>(AttachTransform);
        if (IsValid(BehaviourAttachTransform))
        {
            BehaviourAttachTransform->SetEnabled(false);
        }
    }
}

/////////////////////////////////////////////////////
