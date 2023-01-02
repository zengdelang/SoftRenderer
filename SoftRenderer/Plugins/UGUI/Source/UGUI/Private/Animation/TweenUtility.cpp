#include "Animation/TweenUtility.h"
#include "Core/Layout/RectTransformUtility.h"

float FTweenUtility::EaseAlpha(ETweenEasingFunc InEasingFunc, float InAlpha, float BlendExp)
{
	switch (InEasingFunc)
	{
		case ETweenEasingFunc::TweenEasingFunc_EaseIn:
		{
			return FMath::InterpEaseIn<float>(0.f, 1.f, InAlpha, BlendExp);
		}
		case ETweenEasingFunc::TweenEasingFunc_EaseOut:
		{
			return FMath::InterpEaseOut<float>(0.f, 1.f, InAlpha, BlendExp);
		}
		case ETweenEasingFunc::TweenEasingFunc_EaseInOut:
		{
			return FMath::InterpEaseInOut<float>(0.f, 1.f, InAlpha, BlendExp);
		}
	}
	return InAlpha;
}

UTweenPlayerSubComponent* FTweenUtility::GetTweenPlayer(URectTransformComponent* RectTransform)
{
	if (IsValid(RectTransform))
	{
		return Cast<UTweenPlayerSubComponent>(RectTransform->GetComponent(UTweenPlayerSubComponent::StaticClass(), true));
	}
	return nullptr;
}

UTweenBaseSubComponent* FTweenUtility::GetTween(URectTransformComponent* RectTransform, const FString& TweenGroupName, int32 TweenGroupIndex)
{
	if (IsValid(RectTransform))
	{
		TArray<UTweenBaseSubComponent*, TInlineAllocator<24>> AllTweenComponents;
		RectTransform->GetComponents(AllTweenComponents, true);
		for (int32 Index = 0, Count = AllTweenComponents.Num(); Index < Count; Index++)
		{
			UTweenBaseSubComponent* TweenComponent = AllTweenComponents[Index];
			if (IsValid(TweenComponent) && TweenGroupName == TweenComponent->TweenGroupName && TweenGroupIndex == TweenComponent->TweenGroupIndex)
			{
				return TweenComponent;
			}
		}
	}
	return nullptr;
}

void FTweenUtility::PlayTween(URectTransformComponent* RectTransform, const FString& TweenGroupName, int32 TweenGroupIndex)
{
	UTweenBaseSubComponent* TweenComponent = FTweenUtility::GetTween(RectTransform, TweenGroupName, TweenGroupIndex);
	if (IsValid(TweenComponent))
	{
		TweenComponent->Play();
	}
}

void FTweenUtility::PlayTweenGroup(URectTransformComponent* RectTransform, const FString& TweenGroupName)
{
	UTweenPlayerSubComponent* TweenPlayer = FTweenUtility::GetTweenPlayer(RectTransform);
	if (IsValid(TweenPlayer))
	{
		TweenPlayer = Cast<UTweenPlayerSubComponent>(RectTransform->AddSubComponentByClass(UTweenPlayerSubComponent::StaticClass()));
	}
	if (IsValid(TweenPlayer))
	{
		TweenPlayer->SetGroupName(TweenGroupName);
		TweenPlayer->Play();
	}
}

FVector FTweenUtility::ScreenPointToLocalPoint(URectTransformComponent* RectTransform, UCanvasSubComponent* Canvas, const FVector& ScreenPosition, UWorld* World)
{
	if (IsValid(RectTransform) && IsValid(Canvas) && IsValid(World))
	{
		// TODO 为了让计算正确，临时这样处理（待查找原因）
		RectTransform->SetAnchorMin(FVector2D(0.5f, 0.5f));
		RectTransform->SetAnchorMax(FVector2D(0.5f, 0.5f));
		//RectTransform->SetPivot(FVector2D(0.5f, 0.5f));
		RectTransform->SetAnchoredPosition(FVector2D::ZeroVector);

		FVector2D TempPosition = FVector2D::ZeroVector;
		if (FRectTransformUtility::ScreenPointToLocalPoint(RectTransform, Canvas, FVector2D(ScreenPosition.X, ScreenPosition.Y), TempPosition))
		{
			FVector LocalPosition = FVector::ZeroVector;
			LocalPosition.X = TempPosition.X;
			LocalPosition.Y = TempPosition.Y;
			LocalPosition.Z = ScreenPosition.Z;
			return LocalPosition;
		}
	}
	return FVector::ZeroVector;
}


///////////////////////////////////////////////////////////////////////////////////////////////
UTweenPlayerSubComponent* UTweenLibrary::GetTweenPlayer(URectTransformComponent* RectTransform)
{
	return FTweenUtility::GetTweenPlayer(RectTransform);
}

UTweenBaseSubComponent* UTweenLibrary::GetTween(URectTransformComponent* RectTransform, FString TweenGroupName, int32 TweenGroupIndex)
{
	return FTweenUtility::GetTween(RectTransform, TweenGroupName, TweenGroupIndex);
}

void UTweenLibrary::PlayTween(URectTransformComponent* RectTransform, FString TweenGroupName, int32 TweenGroupIndex)
{
	FTweenUtility::PlayTween(RectTransform, TweenGroupName, TweenGroupIndex);
}

void UTweenLibrary::PlayTweenGroup(URectTransformComponent* RectTransform, FString TweenGroupName)
{
	FTweenUtility::PlayTweenGroup(RectTransform, TweenGroupName);
}