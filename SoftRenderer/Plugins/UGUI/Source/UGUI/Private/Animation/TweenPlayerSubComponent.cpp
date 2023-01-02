#include "Animation/TweenPlayerSubComponent.h"
#include "Core/Layout/RectTransformComponent.h"

/////////////////////////////////////////////////////
// UTweenPlayerSubComponent

UTweenPlayerSubComponent::UTweenPlayerSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TweenGroup = 0;
	
	bAutoPlay = true;

#if WITH_EDITOR
	bEditPreview = false;
#endif

	bNeedSetTimer = false;

	ActiveTweenCount = 0;
}

void UTweenPlayerSubComponent::Awake()
{
	Super::Awake();

	// Get tween components.
	GetTweenComponents();
}

void UTweenPlayerSubComponent::OnEnable()
{
	Super::OnEnable();

#if WITH_EDITOR
	// 编辑器模式直接运行
	if (bEditPreview && EWorldType::EditorPreview == GetWorld()->WorldType)
	{
		Play();
	}
#endif

	if (bAutoPlay)
	{
		if (EWorldType::EditorPreview != GetWorld()->WorldType)
		{
			// 非编辑器预览模式的话等游戏运行后再Play（目前发现如果游戏没有运行时Play会导致把带有使用屏幕坐标的TweenPosition的UI拖到场景时计算出来的位置非常大导致动画不正确）
			if (GetWorld()->HasBegunPlay())
			{
				Play();
			}
			else
			{
				bNeedSetTimer = true;
				GetWorld()->GetTimerManager().SetTimer(WaitForRunTimerHandle, this, &UTweenPlayerSubComponent::CheckHasBegunPlay, 0.001f, false);
			}
		}
	}
}

void UTweenPlayerSubComponent::OnDisable()
{
	Super::OnDisable();

	Stop();
}

void UTweenPlayerSubComponent::CheckHasBegunPlay()
{
	if (GetWorld()->HasBegunPlay())
	{
		Play();

		bNeedSetTimer = false;
		if (WaitForRunTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(WaitForRunTimerHandle);
		}
	}

	if (bNeedSetTimer)
	{
		GetWorld()->GetTimerManager().SetTimer(WaitForRunTimerHandle, this, &UTweenPlayerSubComponent::CheckHasBegunPlay, 0.001f, false);
	}
}

void UTweenPlayerSubComponent::GetTweenComponents()
{
	if (IsValid(AttachTransform))
	{
		TweenComponents.Empty();
		
		TArray<UTweenBaseSubComponent*, TInlineAllocator<24>> AllTweenComponents;
		AttachTransform->GetComponents(AllTweenComponents, true);
		
		for (int32 Index = 0, Count = AllTweenComponents.Num(); Index < Count; ++Index)
		{
			UTweenBaseSubComponent* TweenComponent = AllTweenComponents[Index];
			if (IsValid(TweenComponent) && TweenGroupName == TweenComponent->TweenGroupName)
			{
				TweenComponent->OnFinished.RemoveDynamic(this, &UTweenPlayerSubComponent::OnTweenFinished);
				TweenComponent->OnFinished.AddUniqueDynamic(this, &UTweenPlayerSubComponent::OnTweenFinished);
				TweenComponents.Add(TweenComponent);
			}
		}
	}
}

void UTweenPlayerSubComponent::OnTweenFinished()
{
	--ActiveTweenCount;
	if (0 == ActiveTweenCount)
	{
		OnFinished.Broadcast();
	}
}

bool UTweenPlayerSubComponent::Play()
{
	if (IsEnabledInHierarchy())
	{
		ActiveTweenCount = 0;
		
		for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
		{
			UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
			if (IsValid(TweenComponent))
			{
				TweenComponent->Play();
				ActiveTweenCount++;
			}
		}
		
		return true;
	}
	
	return false;
}

bool UTweenPlayerSubComponent::PlayReverse()
{
	if (IsEnabledInHierarchy())
	{
		ActiveTweenCount = 0;
		
		for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
		{
			UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
			if (IsValid(TweenComponent))
			{
				TweenComponent->PlayReverse();
				ActiveTweenCount++;
			}
		}
		
		return true;
	}
	
	return false;
}

bool UTweenPlayerSubComponent::IsPlaying()
{
	return ActiveTweenCount > 0;
}

void UTweenPlayerSubComponent::Toggle()
{
	for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
	{
		UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
		if (IsValid(TweenComponent))
		{
			TweenComponent->Toggle();
		}
	}
}

void UTweenPlayerSubComponent::Stop()
{
	for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
	{
		UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
		if (IsValid(TweenComponent))
		{
			TweenComponent->Stop();
		}
	}
}

void UTweenPlayerSubComponent::GoToBegin()
{
	for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
	{
		UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
		if (IsValid(TweenComponent))
		{
			TweenComponent->GoToBegin();
		}
	}
}

void UTweenPlayerSubComponent::GoToEnd()
{
	for (int32 Index = 0, Count = TweenComponents.Num(); Index < Count; ++Index)
	{
		UTweenBaseSubComponent* TweenComponent = TweenComponents[Index];
		if (IsValid(TweenComponent))
		{
			TweenComponent->GoToEnd();
		}
	}
}

/////////////////////////////////////////////////////
