#include "Core/VertexModifiers/BaseMeshEffectSubComponent.h"

/////////////////////////////////////////////////////
// UBaseMeshEffectSubComponent

UBaseMeshEffectSubComponent::UBaseMeshEffectSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UBaseMeshEffectSubComponent::OnEnable()
{
	Super::OnEnable();

	Graphic = GetGraphic();
	
	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UBaseMeshEffectSubComponent::OnDisable()
{
	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
	
	Super::OnDisable();
}

#if WITH_EDITORONLY_DATA

void UBaseMeshEffectSubComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

void UBaseMeshEffectSubComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (Graphic)
	{
		Graphic->SetVerticesDirty();
	}
}

#endif

TScriptInterface<IGraphicElementInterface> UBaseMeshEffectSubComponent::GetGraphic()
{
	if (!Graphic)
	{
		UBehaviourComponent* OuterBehaviourComponent = Cast<UBehaviourComponent>(GetOuter());
		if (IsValid(OuterBehaviourComponent))
		{
			Graphic = OuterBehaviourComponent->GetComponentByInterface(UGraphicElementInterface::StaticClass(), true);
		}
	}
	
	return Graphic;
}

void UBaseMeshEffectSubComponent::SetGraphic(TScriptInterface<IGraphicElementInterface> InGraphic)
{
	if (Graphic != InGraphic)
	{
		Graphic = InGraphic;
		
		if (Graphic)
		{
			Graphic->SetVerticesDirty();
		}
	}
}

/////////////////////////////////////////////////////
