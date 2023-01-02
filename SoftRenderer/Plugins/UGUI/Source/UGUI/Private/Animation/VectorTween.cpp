#include "Animation/VectorTween.h"

void FVectorTween::TweenValue(float InPercentage)
{
    OnVectorTweenCallback.Broadcast(FMath::Lerp(Start, Target, InPercentage));
}