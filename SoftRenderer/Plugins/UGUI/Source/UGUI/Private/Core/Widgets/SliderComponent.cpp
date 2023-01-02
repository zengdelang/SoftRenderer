#include "Core/Widgets/SliderComponent.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/Layout/RectTransformUtility.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// USliderComponent

USliderComponent::USliderComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HandleRect(nullptr)
	, FillRect(nullptr)
	, FillContainerRect(nullptr)
	, HandleContainerRect(nullptr)
{
	Direction = ESliderDirection::SliderDirection_LeftToRight;

	MinValue = 0;
	MaxValue = 1;
	Value = 0;

	HandleRectPath.Emplace(2);
	HandleRectPath.Emplace(0);

	FillRectPath.Emplace(1);
	FillRectPath.Emplace(0);

	Offset = FVector2D::ZeroVector;

	bWholeNumbers = false;
	bDelayedUpdateVisuals = false;
}

void USliderComponent::Rebuild(ECanvasUpdate Executing)
{
#if WITH_EDITOR
	if (Executing == ECanvasUpdate::CanvasUpdate_Prelayout)
		OnValueChanged.Broadcast(GetValue());
#endif
}

void USliderComponent::Awake()
{
	Super::Awake();

	SetFillRect(Cast<URectTransformComponent>(FindChildBehaviourComponent(FillRectPath)));
	SetHandleRect(Cast<URectTransformComponent>(FindChildBehaviourComponent(HandleRectPath)));
}

void USliderComponent::OnEnable()
{
	Super::OnEnable();

	UpdateCachedReferences();
	Set(GetValue(), false);
	// Update rects since they need to be initialized correctly.
	UpdateVisuals();
}

void USliderComponent::OnRectTransformDimensionsChange()
{
	Super::OnRectTransformDimensionsChange();

	//This can be invoked before OnEnabled is called. So we shouldn't be accessing other objects, before OnEnable is called.
	if (!IsActiveAndEnabled())
		return;

	UpdateVisuals();
}

#if WITH_EDITORONLY_DATA

void USliderComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bWholeNumbers)
	{
		MinValue = FMath::RoundToFloat(MinValue);
		MaxValue = FMath::RoundToFloat(MaxValue);
		Value = FMath::Clamp(Value, MinValue, MaxValue);
		Value = FMath::RoundToFloat(Value);
	}

	//This can be invoked before OnEnabled is called. So we shouldn't be accessing other objects, before OnEnable is called.
	if (IsActiveAndEnabled())
	{
		UpdateCachedReferences();
		Set(GetValue(), false);
		// Update rects (in next update) since other things might affect them even if value didn't change.
		bDelayedUpdateVisuals = true;

		const auto World = GetWorld();
		if (IsValid(World))
		{
 			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &USliderComponent::DelayUpdateVisuals));
		}
	}

	if (!HasAnyFlags(EObjectFlags::RF_ArchetypeObject))
	{
		FCanvasUpdateRegistry::RegisterCanvasElementForLayoutRebuild(this);
	}
}

#endif

void USliderComponent::DelayUpdateVisuals()
{
	if (bDelayedUpdateVisuals)
	{
		bDelayedUpdateVisuals = false;
		UpdateVisuals();
	}
}

void USliderComponent::OnDrag(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;

	UpdateDrag(EventData);
}

void USliderComponent::OnInitializePotentialDrag(UPointerEventData* EventData)
{
	if (IsValid(EventData))
	{
		EventData->bUseDragThreshold = false;
	}
}

void USliderComponent::OnPointerDown(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;
	
	Super::OnPointerDown(EventData);

	OnPressed.Broadcast();
	
	Offset = FVector2D::ZeroVector;

	FVector LocalMousePos;
	if (IsValid(HandleContainerRect) && FRectTransformUtility::RectangleContainsScreenPoint(HandleRect, OwnerCanvas, FVector2D(EventData->Position), LocalMousePos))
	{
		Offset = FVector2D(LocalMousePos);
	}
	else
	{
		// Outside the slider handle - jump to this point instead
		UpdateDrag(EventData);
	}
}

void USliderComponent::OnPointerUp(UPointerEventData* EventData)
{
	Super::OnPointerUp(EventData);

	if (IsActiveAndEnabled() && IsInteractableInHierarchy() && IsValid(EventData) && EventData->Button == EPointerInputButton::InputButton_Left)
	{
		OnReleased.Broadcast();
	}
}

void USliderComponent::UpdateCachedReferences()
{
	if (IsValid(FillRect) && FillRect != this)
	{
		FillImage = FillRect->GetComponentByInterface(UImageElementInterface::StaticClass(), true);
		if (IsValid(FillRect->GetAttachParent()))
		{
			FillContainerRect = Cast<URectTransformComponent>(FillRect->GetAttachParent());
		}
	}
	else
	{
		FillRect = nullptr;
		FillContainerRect = nullptr;
		FillImage = nullptr;
	}

	if (IsValid(HandleRect) && HandleRect != this)
	{
		if (IsValid(HandleRect->GetAttachParent()))
		{
			HandleContainerRect = Cast<URectTransformComponent>(HandleRect->GetAttachParent());
		}
	}
	else
	{
		HandleRect = nullptr;
		HandleContainerRect = nullptr;
	}
}

float USliderComponent::ClampValue(float Input) const
{
	float NewValue = FMath::Clamp(Input, MinValue, MaxValue);
	if (bWholeNumbers)
		NewValue = FMath::RoundToFloat(NewValue);
	return NewValue;
}

void USliderComponent::Set(float Input, bool bSendCallback)
{
	// Clamp the input
	const float NewValue = ClampValue(Input);

	// If the stepped value doesn't match the last one, it's time to update
	if (Value == NewValue)
		return;
	
	Value = NewValue;
	UpdateVisuals();
	if (bSendCallback)
	{
		OnValueChanged.Broadcast(GetValue());
	}
}

void USliderComponent::UpdateVisuals()
{
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		UpdateCachedReferences();
	}
#endif

	if (IsValid(FillContainerRect))
	{
		FVector2D TargetAnchorMin = FVector2D::ZeroVector;
		FVector2D TargetAnchorMax = FVector2D(1, 1);

		if (FillImage && FillImage->GetImageType() == EImageFillType::ImageFillType_Filled)
		{
			FillImage->SetFillAmount(GetNormalizedValue());
		}
		else
		{
			if (ReverseValue())
			{
				const int32 AxisIndex = static_cast<int32>(GetAxis());
				TargetAnchorMin[AxisIndex] = 1 - GetNormalizedValue();

			}
			else
			{
				const int32 AxisIndex = static_cast<int32>(GetAxis());
				TargetAnchorMax[AxisIndex] = GetNormalizedValue();
			}
		}

		if (IsValid(FillRect))
		{
			FillRect->SetAnchor(TargetAnchorMin, TargetAnchorMax);
		}
	}

	if (IsValid(HandleContainerRect))
	{
		FVector2D TargetAnchorMin = FVector2D::ZeroVector;
		FVector2D TargetAnchorMax = FVector2D(1, 1);

		if (ReverseValue())
		{
			const int32 AxisIndex = static_cast<int32>(GetAxis());
			const float TargetAnchor = 1 - GetNormalizedValue();
			TargetAnchorMin[AxisIndex] = TargetAnchor;
			TargetAnchorMax[AxisIndex] = TargetAnchor;
		}
		else
		{
			const int32 AxisIndex = static_cast<int32>(GetAxis());
			const float TargetAnchor = GetNormalizedValue();
			TargetAnchorMin[AxisIndex] = TargetAnchor;
			TargetAnchorMax[AxisIndex] = TargetAnchor;
		}

		if (IsValid(HandleRect))
		{
			HandleRect->SetAnchor(TargetAnchorMin, TargetAnchorMax);
		}
	}
}

void USliderComponent::UpdateDrag(const UPointerEventData* EventData)
{
	if (!IsValid(EventData))
		return;

	const URectTransformComponent* ClickRect = HandleContainerRect;
	if (!IsValid(ClickRect))
	{
		ClickRect = FillContainerRect;
	}

	if (IsValid(ClickRect))
	{
		const auto& ClickRectTransformRect = ClickRect->GetRect();
		const float AxisValue = ClickRectTransformRect.GetSize()[static_cast<int32>(GetAxis())];

		if (AxisValue > 0)
		{
			FVector2D LocalCursor;
			if (!FRectTransformUtility::ScreenPointToLocalPointInRectangle(ClickRect, OwnerCanvas, FVector2D(EventData->Position), LocalCursor))
				return;
			LocalCursor -= ClickRectTransformRect.GetPosition();

			const float Val = FMath::Clamp((LocalCursor - Offset)[static_cast<int32>(GetAxis())] / AxisValue, 0.0f, 1.0f);
			SetNormalizedValue(ReverseValue() ? 1 - Val : Val);
		}
	}
}

bool USliderComponent::MayDrag(const UPointerEventData* EventData) const
{
	return IsActiveAndEnabled() && IsInteractableInHierarchy() && IsValid(EventData) && EventData->Button == EPointerInputButton::InputButton_Left;
}

/////////////////////////////////////////////////////
