#include "Core/Widgets/Mesh/UISimpleStaticMeshSubComponent.h"
#include "Core/Render/VertexHelper.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UUISimpleStaticMeshSubComponent

UUISimpleStaticMeshSubComponent::UUISimpleStaticMeshSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	StaticMesh = nullptr;
	Texture = nullptr;
	
	MeshRelativeLocation = FVector::ZeroVector;
	MeshRelativeRotation = FRotator(-90, 90, 0);
	MeshRelativeScale3D = FVector::OneVector;

	bAlignBoundingBoxCenter = false;
}

UMaterialInterface* UUISimpleStaticMeshSubComponent::GetMaterial() const
{
	if (!IsValid(Material))
	{
		if (IsValid(StaticMesh))
		{
			return StaticMesh->GetMaterial(0);
		}
	}
	return Material;
}

DECLARE_CYCLE_STAT(TEXT("UIGraphic --- SimpleStaticMesh::OnPopulateMesh"), STAT_UnrealGUI_Sub_SimpleStaticMeshOnPopulateMesh, STATGROUP_UnrealGUI);
void UUISimpleStaticMeshSubComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_Sub_SimpleStaticMeshOnPopulateMesh);
	
	VertexHelper.Reset();

	if (IsValid(StaticMesh))
	{
		FStaticMeshLODResources& LOD = StaticMesh->RenderData->LODResources[0];
		const int32 NumSections = LOD.Sections.Num();
		if (NumSections > 1)
		{
			UE_LOG(LogUGUI, Warning, TEXT("StaticMesh %s has %d sections. UUIStaticMeshComponent expects a static mesh with 1 section."), *StaticMesh->GetName(), NumSections);
		}
		else
		{
			// Populate Vertex Data
			{
				const FVector2D UV1 = GetUV1FromGraphicEffects();
				
				const uint32 NumVerts = LOD.VertexBuffers.PositionVertexBuffer.GetNumVertices();
				VertexHelper.ReserveVertexNum(NumVerts);

				static constexpr int32 MAX_SUPPORTED_UV_SETS = 3;
				const int32 TexCoordsPerVertex = LOD.GetNumTexCoords();
				if (TexCoordsPerVertex > MAX_SUPPORTED_UV_SETS)
				{
					UE_LOG(LogUGUI, Warning, TEXT("[%s] has %d UV sets; slate vertex data supports at most %d"), *StaticMesh->GetName(), TexCoordsPerVertex, MAX_SUPPORTED_UV_SETS);
				}

				const auto& PositionVertexBuffer = LOD.VertexBuffers.PositionVertexBuffer;
				const auto& ColorVertexBuffer = LOD.VertexBuffers.ColorVertexBuffer;
				const auto& StaticMeshVertexBuffer = LOD.VertexBuffers.StaticMeshVertexBuffer;

				const bool bAllowCPUAccess = LOD.VertexBuffers.StaticMeshVertexBuffer.GetAllowCPUAccess();
				if (!bAllowCPUAccess)
				{
					UE_LOG(LogUGUI, Warning, TEXT("StaticMesh %s does not allow CPU access, UISimpleStaticMeshComponent cannot get the UV data."), *StaticMesh->GetName());
				}
				
				FVector FinalMeshRelativeLocation = MeshRelativeLocation;
				if (bAlignBoundingBoxCenter)
				{
					const FTransform RelativeTransform(RelativeRotationCache.RotatorToQuat(MeshRelativeRotation), FVector::ZeroVector, MeshRelativeScale3D);
					const FVector& Position = RelativeTransform.TransformPosition(StaticMesh->GetBoundingBox().GetCenter());
					FinalMeshRelativeLocation = -Position;
				}

				const FTransform RelativeTransform(RelativeRotationCache.RotatorToQuat(MeshRelativeRotation), FinalMeshRelativeLocation, MeshRelativeScale3D);

				for (uint32 Index = 0; Index < NumVerts; ++Index)
				{
					// Copy Position
					const FVector& Position = RelativeTransform.TransformPosition(PositionVertexBuffer.VertexPosition(Index));

					// Copy Color
					FLinearColor FinalColor = (ColorVertexBuffer.GetNumVertices() > 0) ? ColorVertexBuffer.VertexColor(Index).ReinterpretAsLinear() : FLinearColor::White;
					FinalColor = FinalColor * Color;

					// Copy all the UVs that we have, and as many as we can fit.
					const FVector2D& UV0 = (TexCoordsPerVertex > 0 && bAllowCPUAccess) ? StaticMeshVertexBuffer.GetVertexUV(Index, 0) : FVector2D(1, 1);
					//const FVector2D& UV1 = (TexCoordsPerVertex > 1) ? StaticMeshVertexBuffer.GetVertexUV(Index, 1) : FVector2D(1, 1);
					const FVector2D& UV2 = (TexCoordsPerVertex > 2 && bAllowCPUAccess) ? StaticMeshVertexBuffer.GetVertexUV(Index, 2) : FVector2D(1, 1);
					VertexHelper.AddVert(Position, FinalColor, UV0, UV1, UV2);
				}
			}

			// Populate Index data
			{
				FIndexArrayView SourceIndexes = LOD.IndexBuffer.GetArrayView();
				const int32 NumIndexes = SourceIndexes.Num();
				VertexHelper.ReserveIndexNum(NumIndexes);

				for (int32 Index = 0; Index < NumIndexes; ++Index)
				{
					VertexHelper.AddIndex(static_cast<uint16>(SourceIndexes[Index]));
				}
			}
		}
	}
}

float UUISimpleStaticMeshSubComponent::GetPreferredWidth()
{
	if (!IsValid(StaticMesh))
		return 0;

	const FBox& BoundingBox = StaticMesh->GetBoundingBox();
	const FVector& BoxMin = BoundingBox.Min;
	const FVector& BoxMax = BoundingBox.Max;

	FVector BoxPoints[8];
	BoxPoints[0] = BoxMin;
	BoxPoints[1] = BoxMax;
	BoxPoints[2] = FVector(BoxMin.X, BoxMin.Y, BoxMax.Z);
	BoxPoints[3] = FVector(BoxMax.X, BoxMin.Y, BoxMin.Z);
	BoxPoints[4] = FVector(BoxMax.X, BoxMin.Y, BoxMax.Z);
	BoxPoints[5] = FVector(BoxMin.X, BoxMax.Y, BoxMin.Z);
	BoxPoints[6] = FVector(BoxMin.X, BoxMax.Y, BoxMax.Z);
	BoxPoints[7] = FVector(BoxMax.X, BoxMax.Y, BoxMin.Z);

	const FTransform RelativeTransform(RelativeRotationCache.RotatorToQuat(MeshRelativeRotation), MeshRelativeLocation, MeshRelativeScale3D);
	float MaxX = -MAX_flt;
	float MinX = MAX_flt;
	for (int32 Index = 0; Index < 8; ++Index)
	{
		const FVector& Position = RelativeTransform.TransformPosition(BoxPoints[Index]);
		MaxX = FMath::Max(MaxX, Position.X);
		MinX = FMath::Min(MinX, Position.X);
	}

	return FMath::Abs(MaxX - MinX);
}

float UUISimpleStaticMeshSubComponent::GetPreferredHeight()
{
	if (!IsValid(StaticMesh))
		return 0;

	const FBox& BoundingBox = StaticMesh->GetBoundingBox();
	const FVector& BoxMin = BoundingBox.Min;
	const FVector& BoxMax = BoundingBox.Max;

	FVector BoxPoints[8];
	BoxPoints[0] = BoxMin;
	BoxPoints[1] = BoxMax;
	BoxPoints[2] = FVector(BoxMin.X, BoxMin.Y, BoxMax.Z);
	BoxPoints[3] = FVector(BoxMax.X, BoxMin.Y, BoxMin.Z);
	BoxPoints[4] = FVector(BoxMax.X, BoxMin.Y, BoxMax.Z);
	BoxPoints[5] = FVector(BoxMin.X, BoxMax.Y, BoxMin.Z);
	BoxPoints[6] = FVector(BoxMin.X, BoxMax.Y, BoxMax.Z);
	BoxPoints[7] = FVector(BoxMax.X, BoxMax.Y, BoxMin.Z);

	const FTransform RelativeTransform(RelativeRotationCache.RotatorToQuat(MeshRelativeRotation), MeshRelativeLocation, MeshRelativeScale3D);
	float MaxY = -MAX_flt;
	float MinY = MAX_flt;
	for (int32 Index = 0; Index < 8; ++Index)
	{
		const FVector& Position = RelativeTransform.TransformPosition(BoxPoints[Index]);
		MaxY = FMath::Max(MaxY, Position.Y);
		MinY = FMath::Min(MinY, Position.Y);
	}

	return FMath::Abs(MaxY - MinY);
}

/////////////////////////////////////////////////////
