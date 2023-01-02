#include "Animation/MultiVectorTween.h"

void FMultiVectorTween::SetWayPoints(const TArray<FVector>& InWayPoints)
{
    WayPoints = InWayPoints;
    PercentageArray.Empty(WayPoints.Num() - 1);
    // Compute length.
    float TotalLength = 0.0f;
    TArray<float> LengthArray;
    LengthArray.Add(0.0f);
    for (int32 Index = 1, Count = WayPoints.Num(); Index < Count; Index++)
    {
        TotalLength += FVector::Dist(WayPoints[Index], WayPoints[Index - 1]);
        LengthArray.Add(TotalLength);
    }
    // Compute percentage.
    for (int32 Index = 0, Count = LengthArray.Num(); Index < Count; Index++)
    {
        if (TotalLength > 0.001f)
        {
            PercentageArray.Add(LengthArray[Index] / TotalLength);
        }
    }

}

void FMultiVectorTween::TweenValue(float InPercentage)
{
    for (int32 Index = 0, Count = PercentageArray.Num(); Index < Count; Index++)
    {
        if (InPercentage < PercentageArray[Index])
        {
            OnMultiVectorTweenCallback.Broadcast(FMath::Lerp(WayPoints[Index-1], WayPoints[Index]
                , (InPercentage - PercentageArray[Index-1]) / (PercentageArray[Index] - PercentageArray[Index-1])));
            break;
        }
    }
}