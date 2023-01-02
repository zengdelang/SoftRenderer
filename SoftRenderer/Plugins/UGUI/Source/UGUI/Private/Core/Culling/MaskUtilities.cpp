#include "Core/Culling/MaskUtilities.h"
#include "Core/Culling/ClippableInterface.h"
#include "Core/Widgets/RectMask2DSubComponent.h"
#include "Core/Render/CanvasSubComponent.h"

/////////////////////////////////////////////////////
// FMaskUtilities

void FMaskUtilities::Notify2DMaskStateChanged(USceneComponent* MaskComp)
{
	if (!IsValid(MaskComp))
		return;

    const auto BehaviourChild = Cast<UBehaviourComponent>(MaskComp);
    auto ToNotify = Cast<IClippableInterface>(MaskComp);

    if (ToNotify)
    {
        if (BehaviourChild == nullptr || (IsValid(BehaviourChild) && BehaviourChild->IsActiveAndEnabled()))
        {
            ToNotify->RecalculateClipping();
        }
    }

    if (IsValid(BehaviourChild) && BehaviourChild->IsActiveAndEnabled())
    {
        const auto& AllSubComponents = BehaviourChild->GetAllSubComponents();
        for (const auto& SubComponent : AllSubComponents)
        {
            if (IsValid(SubComponent) && SubComponent->IsActiveAndEnabled())
            {
                ToNotify = Cast<IClippableInterface>(SubComponent);
                if (ToNotify)
                {
                    ToNotify->RecalculateClipping();
                }
            }
        }
    }

    const auto BehaviourComp = Cast<UBehaviourComponent>(MaskComp);
    if (BehaviourComp)
    {
        bool bNeedLoop = true;
        while(bNeedLoop)
        {
            bNeedLoop = false;
            BehaviourComp->ResetAttachChildChanged();

            const auto& BehaviourChildren = BehaviourComp->GetAttachChildren();
            for (int32 Index = 0, Count = BehaviourChildren.Num(); Index < Count; ++Index)
            {
                Notify2DMaskStateChanged(BehaviourChildren[Index]);
                
                if (BehaviourComp->IsAttachChildChanged())
                {
                    bNeedLoop = true;
                    break;
                }
            }
        }
    }
    else
    {
        const TArray<USceneComponent*>& AttachChildren = MaskComp->GetAttachChildren();
        for (int32 Index = 0, Count = AttachChildren.Num(); Index < Count; ++Index)
        {
            Notify2DMaskStateChanged(AttachChildren[Index]);
        }
    }
}

bool FMaskUtilities::IsDescendantOrSelf(const USceneComponent* Father, const USceneComponent* Child)
{
	if (!IsValid(Father) || !IsValid(Child))
	{
        return false;
	}

    if (Father == Child)
        return true;

    auto Parent = Child->GetAttachParent();
    while (IsValid(Parent))
    {
        if (Parent == Father)
            return true;

        Parent = Parent->GetAttachParent();
    }

    return false;
}

URectMask2DSubComponent* FMaskUtilities::GetRectMaskForClippable(IClippableInterface* Clippable)
{
    if (!Clippable)
        return nullptr;

    URectMask2DSubComponent* ComponentToReturn = nullptr;

    TArray<URectMask2DSubComponent*, TInlineAllocator<24>> RectMaskComponents;
	const auto BehaviourChild = Cast<UBehaviourComponent>(Clippable->GetSceneComponent());
    if (IsValid(BehaviourChild))
    {
        BehaviourChild->GetComponentsInParent(RectMaskComponents);
    }
	
    if (RectMaskComponents.Num() > 0)
    {
        TArray<UCanvasSubComponent*, TInlineAllocator<24>> CanvasComponents;
        if (IsValid(BehaviourChild))
        {
            BehaviourChild->GetComponentsInParent(CanvasComponents);
        }

        for (int32 Index = 0, Count = RectMaskComponents.Num(); Index < Count; ++Index)
        {
            ComponentToReturn = RectMaskComponents[Index];

            if (Cast<UObject>(ComponentToReturn->GetOuter()) == Cast<UObject>(Clippable))
            {
                ComponentToReturn = nullptr;
                continue;
            }

            if (!ComponentToReturn->IsActiveAndEnabled())
            {
                ComponentToReturn = nullptr;
                continue;
            }
        	
            for (int32 CanvasIndex = CanvasComponents.Num() - 1; CanvasIndex >= 0; --CanvasIndex)
            {
                if (!IsDescendantOrSelf(CanvasComponents[CanvasIndex]->GetRectTransform(), Cast<USceneComponent>(ComponentToReturn->GetOuter())) && CanvasComponents[CanvasIndex]->IsOverrideSorting())
                {
                    ComponentToReturn = nullptr;
                    break;
                }
            }
            break;
        }
    }

    return ComponentToReturn;
}

void FMaskUtilities::GetRectMasksForClip(const URectMask2DSubComponent* Clipper, TArray<URectMask2DSubComponent*>& Masks)
{
    Masks.Reset();

    if (!IsValid(Clipper))
        return;

    TArray<URectMask2DSubComponent*, TInlineAllocator<24>> RectMaskComponents;
    const auto BehaviourChild = Cast<UBehaviourComponent>(Clipper->GetOuter());
    if (IsValid(BehaviourChild))
    {
        BehaviourChild->GetComponentsInParent(RectMaskComponents);
    }
	
	if (RectMaskComponents.Num() > 0)
	{
        TArray<UCanvasSubComponent*, TInlineAllocator<24>> CanvasComponents;
        if (IsValid(BehaviourChild))
        {
            BehaviourChild->GetComponentsInParent(CanvasComponents);
        }

		for (int32 Index = RectMaskComponents.Num() - 1; Index >= 0; --Index)
		{
            bool bShouldAdd = true;

			for (int32 CanvasIndex = CanvasComponents.Num() - 1; CanvasIndex >= 0; --CanvasIndex)
			{
				if (!IsDescendantOrSelf(CanvasComponents[CanvasIndex]->GetRectTransform(), Cast<USceneComponent>(RectMaskComponents[Index]->GetOuter())) && CanvasComponents[CanvasIndex]->IsOverrideSorting())
				{
                    bShouldAdd = false;
                    break;
				}
			}

			if (bShouldAdd)
			{
                Masks.Add(RectMaskComponents[Index]);
			}
		}
	}
}

/////////////////////////////////////////////////////
