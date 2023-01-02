#pragma once

#include "CoreMinimal.h"
#include "Core/Layout/RectTransformComponent.h"

struct FHashQuadTreeNode
{
	TSet<TWeakObjectPtr<URectTransformComponent>> Objects;
	uint32 LocationCode;
	uint8 ChildrenMask;

public:
	FHashQuadTreeNode(): LocationCode(0), ChildrenMask(0)
	{
	}
};

class FHashQuadTree
{
public:
	FHashQuadTree(uint32 InMaxLayer = 16, uint32 InCellSize = 32, uint32 InCenterOffset = 32)
	{
		Init(InMaxLayer, InCellSize, InCenterOffset);
	}

	void Init(uint32 InMaxLayer, uint32 InCellSize, uint32 InCenterOffset)
	{
		CellSize = FMath::Max(1u, InCellSize);
		MaxLayer = FMath::Clamp(FMath::Max(1u, InMaxLayer) - 1, 0u, 15u);
		MaxQuadSize = FMath::Pow(2, MaxLayer) - 1;
		CenterOffset = InCenterOffset;
		PositionOffset = (MaxQuadSize + 1) * 0.5 + InCenterOffset;
	}

protected:
	FORCEINLINE void GetQuadPosition(float X, float Y, FVector2D& Point) const
	{
		Point.X = X / CellSize + PositionOffset;
		Point.Y = Y / CellSize + PositionOffset;
	}
	
public:
	void AddElement(URectTransformComponent* RectTransform, FVector2D BottomLeft, FVector2D TopLeft, FVector2D BottomRight, FVector2D TopRight);
	void RemoveElement(URectTransformComponent* RectTransform);

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
	*   FHashQuadTree Tree;
	*	Tree.FindElements([](FVector2D Center, FVector2D Extent)
	*	{
	*		return true;
	*	}, [](URectTransformComponent* Rect)
	*	{
	*		
	*	});
	*/
	template<typename IterateBoundTestFunc, typename IterateObjectTestFunc>
	void FindElements(const IterateBoundTestFunc& BoundTestFunc, const IterateObjectTestFunc& ObjectTestFunc)
	{
		const float Center = CenterOffset * CellSize;
		FindElementsInternal(1, BoundTestFunc, ObjectTestFunc, FVector2D(-Center), FVector2D((MaxQuadSize + 1) * 0.5 * CellSize));
	}

protected:
	template<typename IterateBoundTestFunc, typename IterateObjectTestFunc>
	void FindElementsInternal(uint32 LocationCode, const IterateBoundTestFunc& BoundTestFunc, const IterateObjectTestFunc& ObjectTestFunc, const FVector2D& Center, const FVector2D& Extent)
	{
		if (const auto NodePtr = Nodes.Find(LocationCode))
		{
			TArray<TWeakObjectPtr<URectTransformComponent>, TInlineAllocator<8>> InvalidObjects;
			
			for (const auto& Object : NodePtr->Objects)
			{
				if (Object.IsValid())
				{
					ObjectTestFunc(Object.Get());
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
				FVector2D ChildExtent = Extent * 0.5f;
				for (int32 Index = 0; Index < 4; ++Index)
				{
					if (NodePtr->ChildrenMask & (1 << Index))
					{
						FVector2D ChildCenter = Center + ChildExtent * FVector2D(Index % 2 == 0 ? -1 : 1, Index <= 1 ? 1 : -1);
						if (BoundTestFunc(ChildCenter, ChildExtent))
						{
							const uint32 ChildLocationCode = NodePtr->LocationCode << 2 | Index;
							FindElementsInternal(ChildLocationCode, BoundTestFunc, ObjectTestFunc, ChildCenter, ChildExtent);
						}
					}
				}
			}
		}
	}
	
	void AddNode(URectTransformComponent* RectTransform, uint32 LocationCode, int32 TreeDepth);
	void AddParentNode(uint32 LocationCode);
	void RemoveNode(uint32 LocationCode);
	
private:
	static TArray<uint32> DepthCellSizes;

	uint32 CellSize;
	uint16 CenterOffset;
	uint16 PositionOffset;
	uint16 MaxQuadSize;
	uint16 MaxLayer;
	
	TMap<uint32, FHashQuadTreeNode> Nodes;
	TMap<TWeakObjectPtr<URectTransformComponent>, uint32> ObjectCodeMap;
};
