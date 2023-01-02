#include "Core/Widgets/FocusComponent.h"
#include "EventSystem/EventSystemComponent.h"

/////////////////////////////////////////////////////
// UFocusComponent

UFocusComponent::UFocusComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UFocusComponent::Awake()
{
    Super::Awake();
    
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        UEventSystemComponent::OnSelectedGameObjectChangedEvent.AddUObject(this, &UFocusComponent::OnSelectedGameObjectChanged);
    }
#else
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.AddUObject(this, &UFocusComponent::OnSelectedGameObjectChanged);
#endif
}

void UFocusComponent::OnEnable()
{
    Super::OnEnable();
    
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        UEventSystemComponent::OnSelectedGameObjectChangedEvent.Broadcast(GetWorld(), this);
    }
#else
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.Broadcast(GetWorld(), this);
#endif
}

void UFocusComponent::OnDisable()
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

void UFocusComponent::OnDestroy()
{
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.RemoveAll(this);
    Super::OnDestroy();
}

void UFocusComponent::OnSelectedGameObjectChanged(const UWorld* InWorld, const USceneComponent* InNewSelectedObj)
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
            if (SceneComp == this)
            {
                bLostFocus = false;
                break;
            }
            SceneComp = SceneComp->GetAttachParent();
        }
    }

    if (bLostFocus)
    {
        SetEnabled(false);
    }
}

/////////////////////////////////////////////////////
