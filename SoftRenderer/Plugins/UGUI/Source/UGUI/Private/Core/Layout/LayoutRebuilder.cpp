#include "Core/Layout/LayoutRebuilder.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "Core/Layout/LayoutGroupInterface.h"
#include "Core/Layout/LayoutSelfControllerInterface.h"

/////////////////////////////////////////////////////
// ULayoutRebuilder

ULayoutRebuilder::ULayoutRebuilder(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    ToRebuildTransform = nullptr;
}

void ULayoutRebuilder::Rebuild(ECanvasUpdate Executing)
{
    switch (Executing)
    {
    case ECanvasUpdate::CanvasUpdate_Layout:
        // It's unfortunate that we'll perform the same GetComponents querys for the tree 2 times,
        // but each tree have to be fully iterated before going to the next action,
        // so reusing the results would entail storing results in a Dictionary or similar,
        // which is probably a bigger overhead than performing GetComponents multiple times.
        PerformLayoutCalculation(ToRebuildTransform.Get(), true);
        PerformLayoutControl(ToRebuildTransform.Get(), true);
        PerformLayoutCalculation(ToRebuildTransform.Get(), false);
        PerformLayoutControl(ToRebuildTransform.Get(), false);
        break;
    default:
        break;
    }

    ICanvasElementInterface* CanvasElement = Cast<ICanvasElementInterface>(ToRebuildTransform);
    if (CanvasElement)
    {
        CanvasElement->Rebuild(Executing);
    }
}

void ULayoutRebuilder::LayoutComplete()
{
    ICanvasElementInterface* CanvasElement = Cast<ICanvasElementInterface>(ToRebuildTransform);
    if (CanvasElement)
    {
        CanvasElement->LayoutComplete();
    }
	
    FLayoutRebuilder::ReleaseLayoutRebuilder(this);
}

void ULayoutRebuilder::PerformLayoutCalculation(URectTransformComponent* Rect, bool bIsHorizontal) const
{
    if (!IsValid(Rect) || Rect->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return;

    TArray<ILayoutElementInterface*, TInlineAllocator<8>> Components;
    Rect->GetComponents(Components);

    TArray<ILayoutGroupInterface*, TInlineAllocator<1>> LayoutGroupComponents;
    Rect->GetComponents(LayoutGroupComponents);
    
    // If there are no controllers on this rect we can skip this entire sub-tree
    // We don't need to consider controllers on children deeper in the sub-tree either,
    // since they will be their own roots.
    if (Components.Num() > 0 || LayoutGroupComponents.Num() > 0)
    {
        // Layout calculations needs to executed bottom up with children being done before their parents,
        // because the parent calculated sizes rely on the sizes of the children.
        const auto AttachChildren = Rect->GetAttachChildren();
        for (int32 Index = 0, Count = AttachChildren.Num(); Index < Count; ++Index)
        {
            PerformLayoutCalculation(Cast<URectTransformComponent>(AttachChildren[Index]), bIsHorizontal);
        }

        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            if (bIsHorizontal)
            {
                Components[Index]->CalculateLayoutInputHorizontal();
            }
            else
            {
                Components[Index]->CalculateLayoutInputVertical();
            }
        }
    }
}

void ULayoutRebuilder::PerformLayoutControl(URectTransformComponent* Rect, bool bIsHorizontal) const
{
    if (!IsValid(Rect) || Rect->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return;

    TArray<ILayoutControllerInterface*, TInlineAllocator<8>> Components;
    Rect->GetComponents(Components);

    // If there are no controllers on this rect we can skip this entire sub-tree
    // We don't need to consider controllers on children deeper in the sub-tree either,
    // since they will be their own roots.
    if (Components.Num() > 0)
    {
        // Layout control needs to executed top down with parents being done before their children,
        // because the children rely on the sizes of the parents.

        // First call layout controllers that may change their own RectTransform
        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            const auto LayoutSelfControllerComp = Cast<ILayoutSelfControllerInterface>(Components[Index]);
            if (LayoutSelfControllerComp)
            {
                if (bIsHorizontal)
                {
                    LayoutSelfControllerComp->SetLayoutHorizontal();
                }
                else
                {
                    LayoutSelfControllerComp->SetLayoutVertical();
                }
            }   
        }

        // Then call the remaining, such as layout groups that change their children, taking their own RectTransform size into account.
        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            const auto LayoutSelfControllerComp = Cast<ILayoutSelfControllerInterface>(Components[Index]);
            if (!LayoutSelfControllerComp)
            {
                if (bIsHorizontal)
                {
                    Components[Index]->SetLayoutHorizontal();
                }
                else
                {
                    Components[Index]->SetLayoutVertical();
                }
            }
        }

        const auto AttachChildren = Rect->GetAttachChildren();
        for (int32 Index = 0, Count = AttachChildren.Num(); Index < Count; ++Index)
        {
            PerformLayoutControl(Cast<URectTransformComponent>(AttachChildren[Index]), bIsHorizontal);
        }
    }
}

/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// FLayoutRebuilder

TArray<ULayoutRebuilder*> FLayoutRebuilder::Rebuilders;

void FLayoutRebuilder::Shutdown()
{
	for (int32 Index = 0, Count = Rebuilders.Num(); Index < Count; ++Index)
	{
        Rebuilders[Index]->RemoveFromRoot();
	}
	
    Rebuilders.Empty();
}

void FLayoutRebuilder::ForceRebuildLayoutImmediate(URectTransformComponent* LayoutRoot)
{
    if (!IsValid(LayoutRoot) || LayoutRoot->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return;

    const auto Rebuilder = GetLayoutRebuilder();
	if (IsValid(Rebuilder))
	{
        Rebuilder->Initialize(LayoutRoot);
        Rebuilder->Rebuild(ECanvasUpdate::CanvasUpdate_Layout);
        ReleaseLayoutRebuilder(Rebuilder);
	}
}

void FLayoutRebuilder::MarkLayoutForRebuild(URectTransformComponent* Rect)
{
    if (!IsValid(Rect) || Rect->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return;

    bool bValidLayoutGroup = true;
    auto LayoutRoot = Rect;
    auto Parent = Cast<URectTransformComponent>(Rect->GetAttachParent());
    while (bValidLayoutGroup && IsValid(Parent))
    {
        bValidLayoutGroup = false;

        TArray<ILayoutGroupInterface*, TInlineAllocator<8>> Components;
        Parent->GetComponents(Components);

        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            if (Components[Index])
            {
                bValidLayoutGroup = true;
                LayoutRoot = Parent;
                break;
            }
        }

        Parent = Cast<URectTransformComponent>(Parent->GetAttachParent());
    }

    // We know the layout root is valid if it's not the same as the rect,
    // since we checked that above. But if they're the same we still need to check.
    if (LayoutRoot == Rect && !ValidController(LayoutRoot))
    {
        return;
    }

    MarkLayoutRootForRebuild(LayoutRoot);
}

bool FLayoutRebuilder::ValidController(URectTransformComponent* LayoutRoot)
{
    if (!IsValid(LayoutRoot) || LayoutRoot->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return false;

    if (LayoutRoot->IsActiveAndEnabled())
    {
        TArray<ILayoutControllerInterface*, TInlineAllocator<8>> Components;
        LayoutRoot->GetComponents(Components);
    	
        for (int32 Index = 0, Count = Components.Num(); Index < Count; ++Index)
        {
            if (Components[Index])
            {
                return true;
            }
        }
    }

    return false;
}

void FLayoutRebuilder::MarkLayoutRootForRebuild(URectTransformComponent* Controller)
{
    if (!IsValid(Controller) || Controller->HasAnyFlags(RF_BeginDestroyed) || IsGarbageCollecting())
        return;

    const auto Rebuilder = GetLayoutRebuilder();
    if (IsValid(Rebuilder))
    {
        Rebuilder->Initialize(Controller);
        if (!FCanvasUpdateRegistry::TryRegisterCanvasElementForLayoutRebuild(Rebuilder))
            ReleaseLayoutRebuilder(Rebuilder);
    }
}

ULayoutRebuilder* FLayoutRebuilder::GetLayoutRebuilder()
{
	if (Rebuilders.Num() > 0)
	{	
        return Rebuilders.Pop();
	}

    UPackage* Package = GetTransientPackage();
	if (IsValid(Package))
	{
        ULayoutRebuilder* Rebuilder = NewObject<ULayoutRebuilder>(Package);
        if (IsValid(Rebuilder))
        {
            Rebuilder->AddToRoot();
            return Rebuilder;
        }
	}
    return nullptr;;
}

void FLayoutRebuilder::ReleaseLayoutRebuilder(ULayoutRebuilder* Rebuilder)
{
	if (IsValid(Rebuilder))
	{
        Rebuilder->Clear();

        if (Rebuilders.Num() > 100)
        {
            const auto CurTime = FApp::GetCurrentTime();
            int32 RemoveCount = 0;
        	
            for (int32 Index = 0, Count = Rebuilders.Num(); Index < Count; ++Index)
            {
            	if (CurTime - Rebuilders[Index]->CacheTime > 60)
            	{
                    ++RemoveCount;
                    Rebuilders[Index]->RemoveFromRoot();
            	}
                else
                {
                    break;
                }
            }

        	if (RemoveCount > 0)
        	{
                Rebuilders.RemoveAtSwap(0, RemoveCount, false);
        	}  
        }
		
        Rebuilders.Push(Rebuilder);
	}
}
