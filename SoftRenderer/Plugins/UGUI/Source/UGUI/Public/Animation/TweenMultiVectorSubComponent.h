#pragma once

#include "CoreMinimal.h"
#include "TweenBaseSubComponent.h"
#include "Animation/MultiVectorTween.h"
#include "TweenMultiVectorSubComponent.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API UTweenMultiVectorSubComponent : public UTweenBaseSubComponent
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = TweenMultiVector)
	void AddPoint(FVector InPoint)
	{
		WayPoints.Add(InPoint);
		UpdateWayPoints();
	}

	UFUNCTION(BlueprintCallable, Category = TweenMultiVector)
	void InsertPoint(FVector InPoint, int32 InIndex)
	{
		WayPoints.Insert(InPoint, InIndex);
		UpdateWayPoints();
	}

	UFUNCTION(BlueprintCallable, Category = TweenMultiVector)
	void RemovePoint(FVector InPoint)
	{
		WayPoints.Remove(InPoint);
		UpdateWayPoints();
	}

	UFUNCTION(BlueprintCallable, Category = TweenMultiVector)
	void UpdatePoint(int32 InIndex, FVector InPoint)
	{
		if (InIndex < WayPoints.Num())
		{
			WayPoints[InIndex] = InPoint;
		}
		UpdateWayPoints();
	}

protected:
	//~ Begin ITweenInterface Interface
	virtual void InitTweenRunner() override;
	virtual void InternalPlay() override;
	virtual void InternalToggle() override;
	//~ End ITweenInterface Interface

	virtual FVector GetCurrent() const { return FVector::ZeroVector; }
	virtual TArray<FVector> GetFinalWayPoints() { return FinalWayPoints; }

private:
	void UpdateWayPoints();
	void UpdateTweenWayPoints(bool bInTweenForward)
	{
		FMultiVectorTween* MultiVectorTween = static_cast<FMultiVectorTween*>(TweenRunner->GetTweenValue());
		if (nullptr != MultiVectorTween && FinalWayPoints.Num() > 1)
		{
			if (bInTweenForward)
			{
				MultiVectorTween->SetWayPoints(FinalWayPoints);
			}
			else
			{
				MultiVectorTween->SetWayPoints(ReverseFinalWayPoints);
			}
		}
	}

protected:
	UPROPERTY(EditAnywhere, Category = TweenMultiVector)
	TArray<FVector> WayPoints;

protected:
	TArray<FVector> FinalWayPoints;
	TArray<FVector> ReverseFinalWayPoints;
	uint8 bPreTweenForward : 1;

};