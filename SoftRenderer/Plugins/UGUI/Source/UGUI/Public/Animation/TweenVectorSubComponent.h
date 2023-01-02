#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "TweenVectorSubComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UTweenVectorSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = TweenVector)
	void SetFrom(FVector InFrom)
	{
		From = InFrom;
	}

	UFUNCTION(BlueprintCallable, Category = TweenVector)
	void SetTo(FVector InTo)
	{
		To = InTo;
	}

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	//~ End ITweenInterface Interface

	virtual FVector GetCurrent() const { return FVector::ZeroVector; }

	virtual FVector GetTo() { return To; }
	virtual FVector GetFrom() { return From; }

protected:
	UPROPERTY(EditAnywhere, Category = TweenVector)
	FVector From = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = TweenVector)
	FVector To = FVector::ZeroVector;



};