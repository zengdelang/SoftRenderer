#include "Animation/TransformTween.h"

void FTransformTween::SetDuration(float InDuration)
{
    Duration = FMath::Max(0.001f, InDuration);
}

void FTransformTween::TweenValue(float InPercentage)
{
    FTweenTransform Tmp;
    Tmp.Position = FMath::Lerp(StartValue.Position, TargetValue.Position, InPercentage);
    Tmp.Rotation = FMath::Lerp(StartValue.Rotation, TargetValue.Rotation, InPercentage);
    Tmp.Scale = FMath::Lerp(StartValue.Scale, TargetValue.Scale, InPercentage);
    OnTransformTweenCallback.Broadcast(Tmp);
}