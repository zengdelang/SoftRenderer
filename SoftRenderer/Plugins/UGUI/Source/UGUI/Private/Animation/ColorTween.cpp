#include "Animation/ColorTween.h"

void FColorTween::TweenValue(float InPercentage)
{
    FLinearColor NewColor = FLinearColor::LerpUsingHSV(StartColor, TargetColor, InPercentage);
    if (TweenMode == EColorTweenMode::Alpha)
    {
        NewColor.R = StartColor.R;
        NewColor.G = StartColor.G;
        NewColor.B = StartColor.B;
    }
    else if (TweenMode == EColorTweenMode::RGB)
    {
        NewColor.A = StartColor.A;
    }
	
    OnColorTweenCallback.Broadcast(NewColor);
}