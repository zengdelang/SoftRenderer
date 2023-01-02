#include "EventSystem/EventData/PointerEventData.h"
#include "Engine/Canvas.h"

#define LOCTEXT_NAMESPACE "UGUI"

/////////////////////////////////////////////////////
// UPointerEventData

UPointerEventData::UPointerEventData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PointerPress = nullptr;
	LastPress = nullptr;
	
    PointerEnter = nullptr;
    RawPointerPress = nullptr;
    PointerDrag = nullptr;
	
	PointerId = -1;

	Position = FVector::ZeroVector;
	Delta = FVector::ZeroVector;

	PressPosition = FVector::ZeroVector;
	
	ClickTime = 0;
	ClickCount = 0;

	ScrollDelta = 0;	
	Button = EPointerInputButton::InputButton_Left;

	bEligibleForClick = false;
	bUseDragThreshold = true;
	bDragging = false;
}

FString UPointerEventData::ToString() const
{
	FString FinalString = TEXT("");
	FinalString += FString::Printf(TEXT("Position: %.2f, %.2f, %.2f\n"), Position.X, Position.Y, Position.Z);
	FinalString += FString::Printf(TEXT("Delta: %.2f, %.2f, %.2f\n"), Delta.X, Delta.Y, Delta.Z);
	FinalString += FString::Printf(TEXT("bEligibleForClick: %d\n"), bEligibleForClick);
	FinalString += FString::Printf(TEXT("PointerEnter: %s\n"), IsValid(PointerEnter) ? *PointerEnter->GetFName().ToString() : TEXT("nullptr"));
	FinalString += FString::Printf(TEXT("PointerPress: %s\n"), IsValid(PointerPress) ? *PointerPress->GetFName().ToString() : TEXT("nullptr"));
	FinalString += FString::Printf(TEXT("LastPress: %s\n"), IsValid(LastPress) ? *LastPress->GetFName().ToString() : TEXT("nullptr"));
	FinalString += FString::Printf(TEXT("PointerDrag: %s\n"), IsValid(PointerDrag) ? *PointerDrag->GetFName().ToString() : TEXT("nullptr"));
	FinalString += FString::Printf(TEXT("bUseDragThreshold: %d\n"), bUseDragThreshold);
	FinalString += FString::Printf(TEXT("Current Raycast:\n"));
	FinalString += PointerCurrentRaycast.ToString();
	FinalString += FString::Printf(TEXT("Press Raycast:\n"));
	FinalString += PointerPressRaycast.ToString();
	return FinalString;
}

void UPointerEventData::ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	{
		const UFont* RenderFont = GEngine->GetMediumFont();
		Canvas->SetDrawColor(FColor::Yellow);

		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Position: %.2f, %.2f, %.2f\n"), Position.X, Position.Y, Position.Z), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Delta: %.2f, %.2f, %.2f\n"), Delta.X, Delta.Y, Delta.Z), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("bEligibleForClick: %d\n"), bEligibleForClick), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("PointerEnter: [ %s ]\n"), IsValid(PointerEnter) ? *PointerEnter->GetReadableName() : TEXT("nullptr")), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("PointerPress: [ %s ]\n"), IsValid(PointerPress) ? *PointerPress->GetReadableName() : TEXT("nullptr")), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("LastPress: [ %s ]\n"), IsValid(LastPress) ? *LastPress->GetReadableName() : TEXT("nullptr")), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("PointerDrag: [ %s ]\n"), IsValid(PointerDrag) ? *PointerDrag->GetReadableName() : TEXT("nullptr")), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("bUseDragThreshold: %d\n"), bUseDragThreshold), 64, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Current Raycast:\n")), 64, YPos);
		PointerCurrentRaycast.ShowDebugInfo(HUD, Canvas, DisplayInfo, YL, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Press Raycast:\n")), 64, YPos);
		PointerPressRaycast.ShowDebugInfo(HUD, Canvas, DisplayInfo, YL, YPos);
	}
#endif
}

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
