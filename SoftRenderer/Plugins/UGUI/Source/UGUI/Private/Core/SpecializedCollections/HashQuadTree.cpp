#include "Core/SpecializedCollections/HashQuadTree.h"
#include "Core/SpecializedCollections/Morton.h"

/////////////////////////////////////////////////////
// FHashQuadTree

TArray<uint32> FHashQuadTree::DepthCellSizes =
{
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
};

void FHashQuadTree::AddElement(URectTransformComponent* RectTransform, FVector2D BottomLeft, FVector2D TopLeft, FVector2D BottomRight, FVector2D TopRight)
{
	if (!IsValid(RectTransform))
		return;
		
	GetQuadPosition(BottomLeft.X, BottomLeft.Y, BottomLeft);
	GetQuadPosition(TopLeft.X, TopLeft.Y, TopLeft);
	GetQuadPosition(BottomRight.X, BottomRight.Y, BottomRight);
	GetQuadPosition(TopRight.X, TopRight.Y, TopRight);
		
	if (BottomLeft.X < 0 || BottomLeft.X > MaxQuadSize || BottomLeft.Y < 0 || BottomLeft.Y > MaxQuadSize ||
		TopLeft.X < 0 || TopLeft.X > MaxQuadSize || TopLeft.Y < 0 || TopLeft.Y > MaxQuadSize ||
		BottomRight.X < 0 || BottomRight.X > MaxQuadSize || BottomRight.Y < 0 || BottomRight.Y > MaxQuadSize ||
		TopRight.X < 0 || TopRight.X > MaxQuadSize || TopRight.Y < 0 || TopRight.Y > MaxQuadSize)
	{
		AddNode(RectTransform, 0, 0);
		return;
	}
		
	const uint32 MaxSize = FMath::CeilToInt(FMath::Max(FMath::Abs(TopLeft.X - BottomLeft.X), FMath::Abs(TopRight.Y - BottomRight.Y)));
	const int32 Layer = Algo::LowerBound(DepthCellSizes, MaxSize);
	const uint32 LayerCellSIze = DepthCellSizes[Layer];
	uint8 Depth = MaxLayer - Layer;
		
	uint32 BottomLeftLocationCode = FMortonUtility::EncodeMorton2(static_cast<uint32>(BottomLeft.X / LayerCellSIze), static_cast<uint32>(BottomLeft.Y / LayerCellSIze));
	uint32 TopLeftLocationCode = FMortonUtility::EncodeMorton2(static_cast<uint32>(TopLeft.X / LayerCellSIze), static_cast<uint32>(TopLeft.Y / LayerCellSIze));
	uint32 BottomRightLocationCode = FMortonUtility::EncodeMorton2(static_cast<uint32>(BottomRight.X / LayerCellSIze), static_cast<uint32>(BottomRight.Y / LayerCellSIze));
	uint32 TopRightLocationCode = FMortonUtility::EncodeMorton2(static_cast<uint32>(TopRight.X / LayerCellSIze), static_cast<uint32>(TopRight.Y / LayerCellSIze));

	while (true)
	{
		if (BottomLeftLocationCode == BottomRightLocationCode && BottomRightLocationCode == TopRightLocationCode && BottomLeftLocationCode == BottomRightLocationCode)
			break;

		--Depth;
			
		BottomLeftLocationCode = BottomLeftLocationCode >> 2;
		TopLeftLocationCode = TopLeftLocationCode >> 2;
		BottomRightLocationCode = BottomRightLocationCode >> 2;
		TopRightLocationCode = TopRightLocationCode >> 2;
	}

	check(Depth <= MaxLayer)
	AddNode(RectTransform, BottomLeftLocationCode, Depth);
}

void FHashQuadTree::RemoveElement(URectTransformComponent* RectTransform)
{
	if (!IsValid(RectTransform))
		return;

	if (const uint32* LocationCodePtr = ObjectCodeMap.Find(RectTransform))
	{
		if (const auto NodePtr = Nodes.Find(*LocationCodePtr))
		{
			NodePtr->Objects.Remove(RectTransform);
			if (NodePtr->Objects.Num() == 0 && NodePtr->ChildrenMask == 0)
			{
				RemoveNode(*LocationCodePtr);
			}
		}

		ObjectCodeMap.Remove(RectTransform);
	}
}

void FHashQuadTree::AddNode(URectTransformComponent* RectTransform, uint32 LocationCode, int32 TreeDepth)
{
	LocationCode = LocationCode | (0x1 << (TreeDepth * 2));

	if (const uint32* LocationCodePtr = ObjectCodeMap.Find(RectTransform))
	{
		if (*LocationCodePtr == LocationCode)
			return;

		if (const auto NodePtr = Nodes.Find(*LocationCodePtr))
		{
			NodePtr->Objects.Remove(RectTransform);
			if (NodePtr->Objects.Num() == 0 && NodePtr->ChildrenMask == 0)
			{
				RemoveNode(*LocationCodePtr);
			}
		}

		ObjectCodeMap.Remove(RectTransform);
	}

	if (const auto NodePtr = Nodes.Find(LocationCode))
	{
		NodePtr->Objects.Add(RectTransform);
	}
	else
	{
		FHashQuadTreeNode Node;
		Node.LocationCode = LocationCode;
		Node.Objects.Add(RectTransform);
		Nodes.Emplace(LocationCode, Node);
	}
	
	ObjectCodeMap.Emplace(RectTransform, LocationCode);

	AddParentNode(LocationCode);
}

void FHashQuadTree::AddParentNode(uint32 LocationCode)
{
	const uint32 ChildIndex = LocationCode & 0x3;
	const uint32 ParentLocationCode = LocationCode >> 2;
	if (const auto ParentNodePtr = Nodes.Find(ParentLocationCode))
	{
		ParentNodePtr->ChildrenMask |= 1 << ChildIndex;
	}
	else if (ParentLocationCode != 0)
	{
		FHashQuadTreeNode Node;
		Node.LocationCode = ParentLocationCode;
		Node.ChildrenMask |= 1 << ChildIndex;
		Nodes.Emplace(ParentLocationCode, Node);

		AddParentNode(ParentLocationCode);
	}
}

void FHashQuadTree::RemoveNode(uint32 LocationCode)
{
	Nodes.Remove(LocationCode);

	const uint32 ChildIndex = LocationCode & 0x3;
	const uint32 ParentLocationCode = LocationCode >> 2;
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
