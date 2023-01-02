#pragma once

#include "CoreMinimal.h"
#include "TextFontStyle.generated.h"

UENUM(BlueprintType)
enum class EFontStyle : uint8
{
    FontStyle_Normal UMETA(DisplayName = "Normal"),
	
    FontStyle_Bold UMETA(DisplayName = "Bold"),
	
    FontStyle_Italic UMETA(DisplayName = "Italic"),
	
    FontStyle_BoldAndItalic UMETA(DisplayName = "BoldAndItalic"),
};
