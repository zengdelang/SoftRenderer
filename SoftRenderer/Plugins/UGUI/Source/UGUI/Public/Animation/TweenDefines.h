#pragma once

#include "CoreMinimal.h"
#include "TweenDefines.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTweenFinished);


UENUM(BlueprintType)
enum class ETweenPlayStyle : uint8
{
    TweenPlayStyle_Once UMETA(DisplayName = "Once"),

    TweenPlayStyle_Loop UMETA(DisplayName = "Loop"),

    TweenPlayStyle_PingPong UMETA(DisplayName = "PingPong"),
    
};

UENUM(BlueprintType)
enum class ETweenEasingFunc : uint8
{
	TweenEasingFunc_CustomCurve UMETA(DisplayName = "CustomCurve"),

	TweenEasingFunc_Linear UMETA(DisplayName = "Linear"),

	TweenEasingFunc_EaseIn UMETA(DisplayName = "EaseIn"),

	TweenEasingFunc_EaseOut UMETA(DisplayName = "EaseOut"),

	TweenEasingFunc_EaseInOut UMETA(DisplayName = "EaseInOut"),

	

};

UENUM(BlueprintType)
enum class ETweenEndWay : uint8
{
	TweenEndWay_StayToCurrent UMETA(DisplayName = "StayToCurrent"),

	TweenEndWay_GoToBegin UMETA(DisplayName = "GoToBegin"),

	TweenEndWay_GoToEnd UMETA(DisplayName = "GoToEnd"),

};

USTRUCT(BlueprintType)
struct FTweenTransform
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = TweenTransform, meta = (DisplayName = "Position"))
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = TweenTransform, meta = (DisplayName = "Rotation"))
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = TweenTransform, meta = (DisplayName = "Scale"))
	FVector Scale = FVector::OneVector;

public:
	FTweenTransform& operator= (const FTweenTransform& Other)
	{
		Position = Other.Position;
		Rotation = Other.Rotation;
		Scale = Other.Scale;
		return *this;
	}

};