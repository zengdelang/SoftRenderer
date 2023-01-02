#include "Core/VertexModifiers/OutlineSubComponent.h"
#include "UGUISettings.h"

/////////////////////////////////////////////////////
// UOutlineSubComponent

UOutlineSubComponent::UOutlineSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UOutlineSubComponent::ModifyMesh(FVertexHelper& VertexHelper)
{
	if (!IsActiveAndEnabled())
		return;

	VertexHelper.GetUIVertexStream(FVertexHelper::VertexStream);
	FLinearColor FinalEffectColor = FLinearColor(0, 0, 0, 0.5f);
	FVector FinalEffectDistance = FVector(1, -1, 0);

	if (bUseExternalEffect)
	{
		const auto CustomEffectPtr = UUGUISettings::Get()->CustomEffectMap.Find(ExternalEffectType);
		if (CustomEffectPtr)
		{
			FinalEffectColor = CustomEffectPtr->EffectColor;
			FinalEffectDistance = CustomEffectPtr->EffectDistance;
		}
	}
	else
	{
		FinalEffectColor = EffectColor;
		FinalEffectDistance = EffectDistance;
	}
	
	const int32 NeededCapacity = FVertexHelper::VertexStream.Num() * 5;
	FVertexHelper::VertexStream.Reserve(NeededCapacity);

	int32 Start = 0;
	int32 End = FVertexHelper::VertexStream.Num();
	ApplyShadow(FVertexHelper::VertexStream, FinalEffectColor, Start, End, FinalEffectDistance.X, FinalEffectDistance.Y, FinalEffectDistance.Z);

	Start = End;
	End = FVertexHelper::VertexStream.Num();
	ApplyShadow(FVertexHelper::VertexStream, FinalEffectColor, Start, End, FinalEffectDistance.X, -FinalEffectDistance.Y, FinalEffectDistance.Z);

	Start = End;
	End = FVertexHelper::VertexStream.Num();
	ApplyShadow(FVertexHelper::VertexStream, FinalEffectColor, Start, End, -FinalEffectDistance.X, FinalEffectDistance.Y, FinalEffectDistance.Z);

	Start = End;
	End = FVertexHelper::VertexStream.Num();
	ApplyShadow(FVertexHelper::VertexStream, FinalEffectColor, Start, End, -FinalEffectDistance.X, -FinalEffectDistance.Y, FinalEffectDistance.Z);

	VertexHelper.Reset();
	VertexHelper.AddUIVertexTriangleStream(FVertexHelper::VertexStream);
}

/////////////////////////////////////////////////////
