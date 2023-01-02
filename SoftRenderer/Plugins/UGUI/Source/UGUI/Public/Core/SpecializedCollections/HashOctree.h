#pragma once

#include "CoreMinimal.h"

struct FHashOctreeObjectItem
{
	FVector WorldCorner0;
	FVector WorldCorner1;
	FVector WorldCorner2;
	FVector WorldCorner3;
};

struct FHashOctreeNode
{
	TMap<TWeakObjectPtr<UObject>, FHashOctreeObjectItem> Objects;
	uint32 LocationCode;
	uint8 ChildrenMask;

public:
	FHashOctreeNode(): LocationCode(0), ChildrenMask(0)
	{
	}
};

class FHashOctree
{
public:
	FHashOctree(uint32 InMaxLayer = 11, uint32 InCellSize = 32, uint32 InCenterOffset = 32)
	{
		Init(InMaxLayer, InCellSize, InCenterOffset);
	}
	
	void Init(uint32 InMaxLayer, uint32 InCellSize, uint32 InCenterOffset)
	{
		CellSize = FMath::Max(1u, InCellSize);
		MaxLayer = FMath::Clamp(FMath::Max(1u, InMaxLayer) - 1, 0u, 10u);
		MaxOctreeSize = FMath::Pow(2, MaxLayer) - 1;
		CenterOffset = InCenterOffset;
		PositionOffset = (MaxOctreeSize + 1) * 0.5 + InCenterOffset;
	}

protected:
	FORCEINLINE void GetOctreePosition(float X, float Y, float Z, FVector& Point) const
	{
		Point.X = X / CellSize + PositionOffset;
		Point.Y = Y / CellSize + PositionOffset;
		Point.Z = Z / CellSize + PositionOffset;
	}
	
public:
	void AddElement(UObject* GraphicElement);
	void RemoveElement(UObject* GraphicElement);

	int32 GetElementNum() const
	{
		return ObjectCodeMap.Num();
	}
	
	void Clear()
	{
		Nodes.Empty();
		ObjectCodeMap.Empty();
	}

	/**
	 *  FHashOctree Tree;
	 *  CanvasOctree.Value.FindElements([&VectorRayOrigin, &VectorInvRayDirection](FVector Center, FVector Extent)
	 *  {
	 *      return true;
	 *  }, [](UObject* Rect, const struct FHashOctreeObjectItem& ObjectItem)
	 *  {
	 *  
	 *  });
	 */
	template<typename IterateBoundTestFunc, typename IterateObjectTestFunc>
	void FindElements(const IterateBoundTestFunc& BoundTestFunc, const IterateObjectTestFunc& ObjectTestFunc)
	{
		const float Center = CenterOffset * CellSize;
		FindElementsInternal(1, BoundTestFunc, ObjectTestFunc, FVector(-Center), FVector((MaxOctreeSize + 1) * 0.5 * CellSize));
	}

protected:
	template<typename IterateBoundTestFunc, typename IterateObjectTestFunc>
	void FindElementsInternal(uint32 LocationCode, const IterateBoundTestFunc& BoundTestFunc, const IterateObjectTestFunc& ObjectTestFunc, const FVector& Center, const FVector& Extent)
	{
		if (const auto NodePtr = Nodes.Find(LocationCode))
		{
			TArray<TWeakObjectPtr<UObject>, TInlineAllocator<8>> InvalidObjects;
			
			for (const auto& ObjectPair : NodePtr->Objects)
			{
				const auto& Object = ObjectPair.Key;
				if (Object.IsValid())
				{
					ObjectTestFunc(Object.Get(), ObjectPair.Value);
				}
				else
				{
					InvalidObjects.Add(Object);
				}
			}

			for (const auto& InvalidObject : InvalidObjects)
			{
				NodePtr->Objects.Remove(InvalidObject);
			}
			
			if (NodePtr->ChildrenMask != 0)
			{
				FVector ChildExtent = Extent * 0.5f;
				for (int32 Index = 0; Index < 8; ++Index)
				{
					if (NodePtr->ChildrenMask & (1 << Index))
					{
						FVector ChildCenter = Center + ChildExtent * FVector(Index % 2 == 0 ? -1 : 1,    Index % 4 < 2 ? -1 : 1, Index <= 3 ? -1 : 1);
						if (BoundTestFunc(ChildCenter, ChildExtent))
						{
							const uint32 ChildLocationCode = NodePtr->LocationCode << 3 | Index;
							FindElementsInternal(ChildLocationCode, BoundTestFunc, ObjectTestFunc, ChildCenter, ChildExtent);
						}
					}
				}
			}
		}
	}
	
	void AddNode(UObject* GraphicElement, uint32 LocationCode, int32 TreeDepth, FVector WorldCorners[4]);
	void AddParentNode(uint32 LocationCode);
	void RemoveNode(uint32 LocationCode);

private:
	static TArray<uint32> DepthCellSizes;

	uint32 CellSize;
	uint16 CenterOffset;
	uint16 PositionOffset;
	uint16 MaxOctreeSize;
	uint16 MaxLayer;
	
	TMap<uint32, FHashOctreeNode> Nodes;
	TMap<TWeakObjectPtr<UObject>, uint32> ObjectCodeMap;
};
