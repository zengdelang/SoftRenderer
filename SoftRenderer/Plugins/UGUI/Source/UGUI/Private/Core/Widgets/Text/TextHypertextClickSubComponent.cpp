#include "Core/Widgets/Text/TextHypertextClickSubComponent.h"
#include "Core/Widgets/Text/TextSubComponent.h"
#include "Core/Layout/RectTransformUtility.h"

/////////////////////////////////////////////////////
// UTextHypertextClickSubComponent

UTextHypertextClickSubComponent::UTextHypertextClickSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UTextHypertextClickSubComponent::OnPointerClick(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (!TextElement)
	{
		UBehaviourComponent* OuterBehaviourComponent = Cast<UBehaviourComponent>(GetOuter());
		if (IsValid(OuterBehaviourComponent))
		{
			TextElement = OuterBehaviourComponent->GetComponentByInterface(UTextElementInterface::StaticClass(), true);
		}
	}

	if (TextElement)
	{
		FVector2D LocalCursorPos = FVector2D::ZeroVector;
		if (FRectTransformUtility::ScreenPointToLocalPointInRectangle(AttachTransform, GetOwnerCanvas(), FVector2D(EventData->Position), LocalCursorPos))
		{
			for (int32 Index = 0, Count = TextElement->HypertextRegionList.Num(); Index < Count; ++Index)
			{
				const auto& HypertextRegion = TextElement->HypertextRegionList[Index];
				if (LocalCursorPos.X >= HypertextRegion.BottomLeft.X && LocalCursorPos.X <= HypertextRegion.TopRight.X
					&& LocalCursorPos.Y <= HypertextRegion.TopRight.Y && LocalCursorPos.Y >= HypertextRegion.BottomLeft.Y)
				{
					TextElement->GetOnClickHyperlinkEvent().Broadcast(HypertextRegion.Hypertext);
					break;
				}
			}
		}
	}
}

void UTextHypertextClickSubComponent::OnPointerDown(UPointerEventData* EventData)
{
	
}

/////////////////////////////////////////////////////
