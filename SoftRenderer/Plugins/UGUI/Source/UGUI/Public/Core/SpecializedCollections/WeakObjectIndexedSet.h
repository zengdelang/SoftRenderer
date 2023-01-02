#pragma once

#include "CoreTypes.h"

struct FWeakObjectIndexedSetItem
{
public:
    TWeakObjectPtr<UObject> Object;

    uint32 HashCode;
};

class FWeakObjectIndexedSet
{
private:
    TArray<FWeakObjectIndexedSetItem> List;
	
    TMap<uint32, int32> Map;
	
public:
    FORCEINLINE bool AddUnique(UObject* Item)
    {
        if (!IsValid(Item))
            return false;

        const uint32 HashCode = GetTypeHash(Item);
        if (Map.Contains(HashCode))
            return false;

        FWeakObjectIndexedSetItem ObjectItem;
        ObjectItem.Object = Item;
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
    
    FORCEINLINE int32 Num() const
    {
        return List.Num();
    }

    FORCEINLINE bool Remove(UObject* Item)
    {
        const int32* IndexPtr = Map.Find(GetTypeHash(Item));
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
        return List[Index].Object.Get();
    }
};
