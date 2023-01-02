#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/CanvasElementInterface.h"
#include "CanvasElementIndexedSet.generated.h"

USTRUCT()
struct FCanvasElementIndexedSetItem
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(Transient)
    UObject* Object;

    uint32 HashCode;
};

USTRUCT()
struct UGUI_API FCanvasElementIndexedSet
{
	GENERATED_USTRUCT_BODY()

protected:
    UPROPERTY(Transient)
	TArray<FCanvasElementIndexedSetItem> List;

    TMap<uint32, int32> Map;
	
public:
    FORCEINLINE bool AddUnique(ICanvasElementInterface* Item)
    {
        const auto Obj = Cast<UObject>(Item);
        if (!IsValid(Obj))
            return false;

        const uint32 HashCode = Item->GetHashCode();
        if (Map.Contains(HashCode))
            return false;

        FCanvasElementIndexedSetItem ObjectItem;
        ObjectItem.Object = Obj;
        ObjectItem.HashCode = HashCode;
        List.Emplace(ObjectItem);

        Map.Add(HashCode, List.Num() - 1);
        return true;
    }

    FORCEINLINE void Empty()
    {
        List.Empty();
        Map.Empty();
    }

    FORCEINLINE void Reset()
    {
        List.Reset();
        Map.Reset();
    }

    FORCEINLINE bool Contains(ICanvasElementInterface* Item) const
	{
        const auto Obj = Cast<UObject>(Item);
        if (!IsValid(Obj))
            return false;
        return Map.Contains(Item->GetHashCode());
    }

    FORCEINLINE int32 Num() const
    {
        return List.Num();
    }

    FORCEINLINE bool Remove(ICanvasElementInterface* Item)
    {
        const auto Obj = Cast<UObject>(Item);
        if (!IsValid(Obj))
            return false;

        const int32* IndexPtr = Map.Find(Item->GetHashCode());
        if (!IndexPtr)
            return false;

        RemoveAt(*IndexPtr);
        return true;
    }

    FORCEINLINE void RemoveAt(int32 Index)
    { 	
        uint32 HashCode = List[Index].HashCode;
        Map.Remove(HashCode);

        const int32 ReplaceItemIndex = List.Num() - 1;
        if (Index == ReplaceItemIndex)
        {
            List.RemoveAt(ReplaceItemIndex, 1, false);
        }
        else
        {
            HashCode = List[ReplaceItemIndex].HashCode;
            List[Index] = List[ReplaceItemIndex];
            Map.Add(HashCode, Index);
            List.RemoveAt(ReplaceItemIndex, 1, false);
        }
    }

    FORCEINLINE UObject* operator[](int32 Index)
    {
        return List[Index].Object;
    }

    struct FCompareParentCount
    {
        FORCEINLINE bool operator()(const FCanvasElementIndexedSetItem& A, const FCanvasElementIndexedSetItem& B) const
        {
            const auto CanvasElementA = Cast<ICanvasElementInterface>(A.Object);
            const auto CanvasElementB = Cast<ICanvasElementInterface>(B.Object);

            const auto T1 = CanvasElementA ? CanvasElementA->GetTransform() : nullptr;
            const auto T2 = CanvasElementB ? CanvasElementB->GetTransform() : nullptr;
        	
            return ParentCount(T1) < ParentCount(T2);
        }

        FORCEINLINE static int32 ParentCount(const USceneComponent* Child)
        {
            if (!IsValid(Child))
                return 0;

            auto Parent = Child->GetAttachParent();
            int32 Count = 0;
            while (IsValid(Parent))
            {
                ++Count;
                Parent = Parent->GetAttachParent();
            }
            return Count;
        }
    };

    FORCEINLINE void Sort()
    {
        if (List.Num() <= 0)
            return;
    	
        // There might be better ways to sort and keep the dictionary index up to date.
        List.Sort(FCompareParentCount());
    	
        // Rebuild the dictionary index.
    	for (int32 Index = 0, Count = List.Num(); Index < Count; ++Index)
        {
            const uint32 HashCode = List[Index].HashCode;
    	    Map.Add(HashCode, Index);
        }
    }
};
