#include "Animation/RotatorTween.h"

void FRotatorTween::TweenValue(float InPercentage)
{
    OnRotatorTweenCallback.Broadcast(FMath::Lerp(Start, Target, InPercentage));
}