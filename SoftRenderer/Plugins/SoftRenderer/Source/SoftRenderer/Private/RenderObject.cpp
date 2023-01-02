#include "RenderObject.h"

/////////////////////////////////////////////////////
// URenderObject

URenderObject::URenderObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), Material()
{
	WorldLocation = FVector::ZeroVector;
	WorldRotation = FRotator::ZeroRotator;
	WorldScale = FVector::OneVector;
}

FMatrix URenderObject::GetLocalToWorld() const
{
	return FTransform(WorldRotation.Quaternion(), WorldLocation, WorldScale).ToMatrixWithScale();
}

/////////////////////////////////////////////////////

