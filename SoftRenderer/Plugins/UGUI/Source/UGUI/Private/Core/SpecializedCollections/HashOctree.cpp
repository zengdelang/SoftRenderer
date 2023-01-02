#include "Core/SpecializedCollections/HashOctree.h"
#include "Core/SpecializedCollections/Morton.h"
#include "Core/Widgets/GraphicElementInterface.h"

/////////////////////////////////////////////////////
// FHashOctree

TArray<uint32> FHashOctree::DepthCellSizes =
{
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
};

void FHashOctree::AddElement(UObject* GraphicElement)
{
	if (!IsValid(GraphicElement))
		return;

	const auto Graphic = Cast<IGraphicElementInterface>(GraphicElement);
	if (!Graphic)
		return;

	const URectTransformComponent* RectTransform = Graphic->GetTransformComponent();
	if (!IsValid(RectTransform))
		return;
	
	FVector WorldCorners[4];
	RectTransform->GetWorldCorners(WorldCorners);

	FVector Min(1e8, 1e8, 1e8);
	FVector Max(-1e8, -1e8, -1e8);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector WorldCorner = WorldCorners[Index];

		Min.X = FMath::Min(Min.X, WorldCorner.X);
		Min.Y = FMath::Min(Min.Y, WorldCorner.Y);
		Min.Z = FMath::Min(Min.Z, WorldCorner.Z);

		Max.X = FMath::Max(Max.X, WorldCorner.X);
		Max.Y = FMath::Max(Max.Y, WorldCorner.Y);
		Max.Z = FMath::Max(Max.Z, WorldCorner.Z);
	}
	
	FVector Corner0 = FVector(Min.X, Min.Y, Min.Z);
	FVector Corner1 = FVector(Min.X, Min.Y, Max.Z);
	FVector Corner2 = FVector(Min.X, Max.Y, Max.Z);
	FVector Corner3 = FVector(Min.X, Max.Y, Min.Z);
	FVector Corner4 = FVector(Max.X, Min.Y, Min.Z);
	FVector Corner5 = FVector(Max.X, Min.Y, Max.Z);
	FVector Corner6 = FVector(Max.X, Max.Y, Min.Z);
	FVector Corner7 = FVector(Max.X, Max.Y, Max.Z);
	
	GetOctreePosition(Corner0.X, Corner0.Y, Corner0.Z, Corner0);
	GetOctreePosition(Corner1.X, Corner1.Y, Corner1.Z, Corner1);
	GetOctreePosition(Corner2.X, Corner2.Y, Corner2.Z, Corner2);
	GetOctreePosition(Corner3.X, Corner3.Y, Corner3.Z, Corner3);
	GetOctreePosition(Corner4.X, Corner4.Y, Corner4.Z, Corner4);
	GetOctreePosition(Corner5.X, Corner5.Y, Corner5.Z, Corner5);
	GetOctreePosition(Corner6.X, Corner6.Y, Corner6.Z, Corner6);
	GetOctreePosition(Corner7.X, Corner7.Y, Corner7.Z, Corner7);
		
	if (Corner0.X < 0 || Corner0.X > MaxOctreeSize || Corner0.Y < 0 || Corner0.Y > MaxOctreeSize || Corner0.Z < 0 || Corner0.Z > MaxOctreeSize ||
		Corner1.X < 0 || Corner1.X > MaxOctreeSize || Corner1.Y < 0 || Corner1.Y > MaxOctreeSize || Corner1.Z < 0 || Corner1.Z > MaxOctreeSize ||
		Corner2.X < 0 || Corner2.X > MaxOctreeSize || Corner2.Y < 0 || Corner2.Y > MaxOctreeSize || Corner2.Z < 0 || Corner2.Z > MaxOctreeSize ||
		Corner3.X < 0 || Corner3.X > MaxOctreeSize || Corner3.Y < 0 || Corner3.Y > MaxOctreeSize || Corner3.Z < 0 || Corner3.Z > MaxOctreeSize ||
		Corner4.X < 0 || Corner4.X > MaxOctreeSize || Corner4.Y < 0 || Corner4.Y > MaxOctreeSize || Corner4.Z < 0 || Corner4.Z > MaxOctreeSize ||
		Corner5.X < 0 || Corner5.X > MaxOctreeSize || Corner5.Y < 0 || Corner5.Y > MaxOctreeSize || Corner5.Z < 0 || Corner5.Z > MaxOctreeSize ||
		Corner6.X < 0 || Corner6.X > MaxOctreeSize || Corner6.Y < 0 || Corner6.Y > MaxOctreeSize || Corner6.Z < 0 || Corner6.Z > MaxOctreeSize ||
		Corner7.X < 0 || Corner7.X > MaxOctreeSize || Corner7.Y < 0 || Corner7.Y > MaxOctreeSize || Corner7.Z < 0 || Corner7.Z > MaxOctreeSize )
	{
		AddNode(GraphicElement, 0, 0, WorldCorners);
		return;
	}
		
	const uint32 MaxSize = FMath::CeilToInt(FMath::Max3(FMath::Abs(Max.X - Min.X), FMath::Abs(Max.Y - Min.Y), FMath::Abs(Max.Z - Min.Z)) / CellSize);
	const int32 Layer = Algo::LowerBound(DepthCellSizes, MaxSize);
	const uint32 LayerCellSIze = DepthCellSizes[Layer];
	uint8 Depth = MaxLayer - Layer;
	
	uint32 Corner0LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner0.X / LayerCellSIze), static_cast<uint32>(Corner0.Y / LayerCellSIze), static_cast<uint32>(Corner0.Z / LayerCellSIze));
	uint32 Corner1LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner1.X / LayerCellSIze), static_cast<uint32>(Corner1.Y / LayerCellSIze), static_cast<uint32>(Corner1.Z / LayerCellSIze));
	uint32 Corner2LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner2.X / LayerCellSIze), static_cast<uint32>(Corner2.Y / LayerCellSIze), static_cast<uint32>(Corner2.Z / LayerCellSIze));
	uint32 Corner3LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner3.X / LayerCellSIze), static_cast<uint32>(Corner3.Y / LayerCellSIze), static_cast<uint32>(Corner3.Z / LayerCellSIze));
	uint32 Corner4LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner4.X / LayerCellSIze), static_cast<uint32>(Corner4.Y / LayerCellSIze), static_cast<uint32>(Corner4.Z / LayerCellSIze));
	uint32 Corner5LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner5.X / LayerCellSIze), static_cast<uint32>(Corner5.Y / LayerCellSIze), static_cast<uint32>(Corner5.Z / LayerCellSIze));
	uint32 Corner6LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner6.X / LayerCellSIze), static_cast<uint32>(Corner6.Y / LayerCellSIze), static_cast<uint32>(Corner6.Z / LayerCellSIze));
	uint32 Corner7LocationCode = FMortonUtility::EncodeMorton3(static_cast<uint32>(Corner7.X / LayerCellSIze), static_cast<uint32>(Corner7.Y / LayerCellSIze), static_cast<uint32>(Corner7.Z / LayerCellSIze));
		
	while (true)
	{
		if (Corner0LocationCode == Corner1LocationCode && Corner2LocationCode == Corner3LocationCode &&
			Corner0LocationCode == Corner2LocationCode && Corner0LocationCode == Corner4LocationCode &&
			Corner4LocationCode == Corner5LocationCode && Corner6LocationCode == Corner7LocationCode &&
			Corner4LocationCode == Corner7LocationCode)
			break;

		--Depth;
			
		Corner0LocationCode = Corner0LocationCode >> 3;
		Corner1LocationCode = Corner1LocationCode >> 3;
		Corner2LocationCode = Corner2LocationCode >> 3;
		Corner3LocationCode = Corner3LocationCode >> 3;
		Corner4LocationCode = Corner4LocationCode >> 3;
		Corner5LocationCode = Corner5LocationCode >> 3;
		Corner6LocationCode = Corner6LocationCode >> 3;
		Corner7LocationCode = Corner7LocationCode >> 3;
	}

	check(Depth <= MaxLayer)
	AddNode(GraphicElement, Corner0LocationCode, Depth, WorldCorners);
}

void FHashOctree::RemoveElement(UObject* GraphicElement)
{
	if (!IsValid(GraphicElement))
		return;

	if (const uint32* LocationCodePtr = ObjectCodeMap.Find(GraphicElement))
	{
		if (const auto NodePtr = Nodes.Find(*LocationCodePtr))
		{
			NodePtr->Objects.Remove(GraphicElement);
			if (NodePtr->Objects.Num() == 0 && NodePtr->ChildrenMask == 0)
			{
				RemoveNode(*LocationCodePtr);
			}
		}

		ObjectCodeMap.Remove(GraphicElement);
	}
}

void FHashOctree::AddNode(UObject* GraphicElement, uint32 LocationCode, int32 TreeDepth, FVector WorldCorners[4])
{
	LocationCode = LocationCode | (0x1 << (TreeDepth * 3));

	if (const uint32* LocationCodePtr = ObjectCodeMap.Find(GraphicElement))
	{
		if (*LocationCodePtr == LocationCode)
		{
			if (const auto NodePtr = Nodes.Find(*LocationCodePtr))
			{
				if (const auto ObjectItemPtr = NodePtr->Objects.Find(GraphicElement))
				{
					ObjectItemPtr->WorldCorner0 = WorldCorners[0];
					ObjectItemPtr->WorldCorner1 = WorldCorners[1];
					ObjectItemPtr->WorldCorner2 = WorldCorners[2];
					ObjectItemPtr->WorldCorner3 = WorldCorners[3];
				}
			}
			return;
		}

		if (const auto NodePtr = Nodes.Find(*LocationCodePtr))
		{
			NodePtr->Objects.Remove(GraphicElement);
			if (NodePtr->Objects.Num() == 0 && NodePtr->ChildrenMask == 0)
			{
				RemoveNode(*LocationCodePtr);
			}
		}

		ObjectCodeMap.Remove(GraphicElement);
	}

	if (const auto NodePtr = Nodes.Find(LocationCode))
	{
		FHashOctreeObjectItem ObjectItem;
		ObjectItem.WorldCorner0 = WorldCorners[0];
		ObjectItem.WorldCorner1 = WorldCorners[1];
		ObjectItem.WorldCorner2 = WorldCorners[2];
		ObjectItem.WorldCorner3 = WorldCorners[3];

		NodePtr->Objects.Emplace(GraphicElement, ObjectItem);
	}
	else
	{
		FHashOctreeObjectItem ObjectItem;
		ObjectItem.WorldCorner0 = WorldCorners[0];
		ObjectItem.WorldCorner1 = WorldCorners[1];
		ObjectItem.WorldCorner2 = WorldCorners[2];
		ObjectItem.WorldCorner3 = WorldCorners[3];
	
		FHashOctreeNode Node;
		Node.LocationCode = LocationCode;
		Node.Objects.Emplace(GraphicElement, ObjectItem);
		Nodes.Emplace(LocationCode, Node);
	}
	
	ObjectCodeMap.Emplace(GraphicElement, LocationCode);

	AddParentNode(LocationCode);
}

void FHashOctree::AddParentNode(uint32 LocationCode)
{
	const uint32 ChildIndex = LocationCode & 0x7;
	const uint32 ParentLocationCode = LocationCode >> 3;
	if (const auto ParentNodePtr = Nodes.Find(ParentLocationCode))
	{
		ParentNodePtr->ChildrenMask |= 1 << ChildIndex;
	}
	else if (ParentLocationCode != 0)
	{
		FHashOctreeNode Node;
		Node.LocationCode = ParentLocationCode;
		Node.ChildrenMask |= 1 << ChildIndex;
		Nodes.Emplace(ParentLocationCode, Node);

		AddParentNode(ParentLocationCode);
	}
}

void FHashOctree::RemoveNode(uint32 LocationCode)
{
	Nodes.Remove(LocationCode);

	const uint32 ChildIndex = LocationCode & 0x7;
	const uint32 ParentLocationCode = LocationCode >> 3;
	if (const auto ParentNodePtr = Nodes.Find(ParentLocationCode))
	{
		ParentNodePtr->ChildrenMask &= ~(1 << ChildIndex);
		if (ParentNodePtr->Objects.Num() == 0 && ParentNodePtr->ChildrenMask == 0)
		{
			RemoveNode(ParentLocationCode);
		}
	}
}

/////////////////////////////////////////////////////

