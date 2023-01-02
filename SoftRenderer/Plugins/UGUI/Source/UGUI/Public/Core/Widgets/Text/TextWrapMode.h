#pragma once

#include "CoreMinimal.h"
#include "TextWrapMode.generated.h"

UENUM(BlueprintType)
enum class EHorizontalWrapMode : uint8
{
    HorizontalWrapMode_Wrap UMETA(DisplayName = "Wrap"),

    HorizontalWrapMode_Overflow UMETA(DisplayName = "Overflow"),
};

UENUM(BlueprintType)
enum class EVerticalWrapMode : uint8
{
    VerticalWrapMode_Truncate UMETA(DisplayName = "Truncate"),
	
    VerticalWrapMode_Overflow UMETA(DisplayName = "Overflow"),
};
