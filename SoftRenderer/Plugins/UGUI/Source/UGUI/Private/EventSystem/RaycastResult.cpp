#include "EventSystem/RaycastResult.h"
#include "Engine/Canvas.h"

/////////////////////////////////////////////////////
// FRaycastResult

bool FRaycastResult::IsValidResult() const
{
    return IsValid(GameObject);
}

FString FRaycastResult::ToString() const
{
	if (!IsValidResult())
	{
        return TEXT("");
	}

	FString FinalString = TEXT("");
	FinalString += FString::Printf(TEXT("Name: %s\n"), *GameObject->GetFName().ToString());
	FinalString += FString::Printf(TEXT("ScreenPosition: %.2f, %.2f\n"), ScreenPosition.X, ScreenPosition.Y);
	return FinalString;
}

void FRaycastResult::ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (!IsValidResult())
	{
		return;
	}
	
	{
		const UFont* RenderFont = GEngine->GetMediumFont();
		Canvas->SetDrawColor(FColor::Yellow);

		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Name: %s\n"), *GameObject->GetFName().ToString()), 84, YPos);
		YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("ScreenPosition: %.2f, %.2f\n"), ScreenPosition.X, ScreenPosition.Y), 84, YPos);
	}
#endif
}

/////////////////////////////////////////////////////
