#include "Animation/FloatTween.h"

void FFloatTween::SetDuration(float InDuration)
{
    Duration = FMath::Max(0.001f, InDuration);
}

void FFloatTween::TweenValue(float InPercentage)
{
    const float NewValue = FMath::Lerp(StartValue, TargetValue, InPercentage);
    OnFloatTweenCallback.Broadcast(NewValue);
}