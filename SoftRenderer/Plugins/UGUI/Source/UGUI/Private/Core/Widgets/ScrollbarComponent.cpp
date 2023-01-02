#include "Core/Widgets/ScrollbarComponent.h"
#include "Core/Layout/RectTransformUtility.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UScrollbarComponent

UScrollbarComponent::UScrollbarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HandleRect(nullptr)
	, ContainerRect(nullptr)
{
	Direction = EScrollBarDirection::Direction_LeftToRight;

	Value = 0;
	Size = 0.2f;
	NumberOfSteps = 0;

	HandleRectPath.Emplace(0);
	HandleRectPath.Emplace(0);

	Offset = FVector2D::ZeroVector;

	bIsPointerDownAndNotDragging = false;
	bDelayedUpdateVisuals = false;
	bNeedSetTimer = false;
}

void UScrollbarComponent::Awake()
{
	Super::Awake();

	SetHandleRect(Cast<URectTransformComponent>(FindChildBehaviourComponent(HandleRectPath)));
}

void UScrollbarComponent::OnEnable()
{
	Super::OnEnable();

	UpdateCachedReferences();
	Set(Value, false);
	// Update rects since they need to be initialized correctly.
	UpdateVisuals();
}

void UScrollbarComponent::OnRectTransformDimensionsChange()
{
	Super::OnRectTransformDimensionsChange();

	//This can be invoked before OnEnabled is called. So we shouldn't be accessing other objects, before OnEnable is called.
	if (!IsActiveAndEnabled())
		return;

	UpdateVisuals();
}

#if WITH_EDITORONLY_DATA

void UScrollbarComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//This can be invoked before OnEnabled is called. So we shouldn't be accessing other objects, before OnEnable is called.
	if (IsActiveAndEnabled())
	{
		UpdateCachedReferences();
		Set(Value, false);
		// Update rects (in next update) since other things might affect them even if value didn't change.
		bDelayedUpdateVisuals = true;

		const auto World = GetWorld();
		if (IsValid(World))
		{
 			World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UScrollbarComponent::DelayUpdateVisuals));
		}
	}
}

#endif

void UScrollbarComponent::DelayUpdateVisuals()
{
	if (bDelayedUpdateVisuals)
	{
		bDelayedUpdateVisuals = false;
		UpdateVisuals();
	}
}

void UScrollbarComponent::OnBeginDrag(UPointerEventData* EventData)
{
	bIsPointerDownAndNotDragging = false;

	if (!MayDrag(EventData))
		return;

	if (!IsValid(ContainerRect))
		return;

	if (!IsValid(HandleRect))
		return;

	Offset = FVector2D::ZeroVector;
	
	FVector LocalMousePos;
	if (FRectTransformUtility::RectangleContainsScreenPoint(HandleRect, OwnerCanvas, FVector2D(EventData->Position), LocalMousePos))
	{
		Offset = FVector2D(LocalMousePos) - HandleRect->GetRect().GetCenter();
	}
}

void UScrollbarComponent::OnDrag(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;

	UpdateDrag(EventData);
}

void UScrollbarComponent::OnInitializePotentialDrag(UPointerEventData* EventData)
{
	if (IsValid(EventData))
	{
		EventData->bUseDragThreshold = false;
	}
}

void UScrollbarComponent::OnPointerDown(UPointerEventData* EventData)
{
	if (!MayDrag(EventData))
		return;
	
	Super::OnPointerDown(EventData);

	bIsPointerDownAndNotDragging = true;

	bNeedSetTimer = true;
	
	const auto World = GetWorld();
	if (IsValid(World))
	{
		World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &UScrollbarComponent::ClickRepeat, 
			TWeakObjectPtr<UPointerEventData>(EventData)), 0.001f, false);
	}
}

void UScrollbarComponent::OnPointerUp(UPointerEventData* EventData)
{
	Super::OnPointerUp(EventData);
	bIsPointerDownAndNotDragging = false;
}

void UScrollbarComponent::UpdateCachedReferences()
{
	if (IsValid(HandleRect) && IsValid(HandleRect->GetAttachParent()))
	{
		ContainerRect = Cast<URectTransformComponent>(HandleRect->GetAttachParent());
	}
	else
	{
		ContainerRect = nullptr;
	}
}

void UScrollbarComponent::Set(float Input, bool bSendCallback)
{
	const float CurrentValue = Value;

	// clamp01 input in called before calling this function, this allows inertia from dragging content to go past extremities without being clamped
	Value = Input;

	// If the stepped value doesn't match the last one, it's time to update
	if (CurrentValue == GetValue())
		return;

	UpdateVisuals();
	if (bSendCallback)
	{
		OnValueChanged.Broadcast(GetValue());
	}
}

void UScrollbarComponent::UpdateVisuals()
{
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		UpdateCachedReferences();
	}
#endif

	if (IsValid(ContainerRect))
	{
		FVector2D TargetAnchorMin = FVector2D::ZeroVector;
		FVector2D TargetAnchorMax = FVector2D(1, 1);

		const float Movement = FMath::Clamp(GetValue(), 0.0f, 1.0f) * (1 - GetSize());
		if (ReverseValue())
		{
			const int32 AxisIndex = static_cast<int32>(GetAxis());
			TargetAnchorMin[AxisIndex] = 1 - Movement - GetSize();
			TargetAnchorMax[AxisIndex] = 1 - Movement;
		}
		else
		{
			const int32 AxisIndex = static_cast<int32>(GetAxis());
			TargetAnchorMin[AxisIndex] = Movement;
			TargetAnchorMax[AxisIndex] = Movement + GetSize();
		}

		if (IsValid(HandleRect))
		{
			HandleRect->SetAnchor(TargetAnchorMin, TargetAnchorMax);
		}
	}
}

void UScrollbarComponent::UpdateDrag(const UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (!IsValid(ContainerRect))
		return;

	if (!IsValid(HandleRect))
		return;

	FVector2D LocalCursor;
	if (!FRectTransformUtility::ScreenPointToLocalPointInRectangle(ContainerRect, OwnerCanvas, FVector2D(EventData->Position), LocalCursor))
		return;

	const FVector2D HandleCenterRelativeToContainerCorner = LocalCursor - Offset - ContainerRect->GetRect().GetPosition();
	const FVector2D HandleCorner = HandleCenterRelativeToContainerCorner - (HandleRect->GetRect().GetSize() - HandleRect->GetSizeDelta()) * 0.5f;

	const float ParentSize = static_cast<int32>(GetAxis()) == 0 ? ContainerRect->GetRect().Width : ContainerRect->GetRect().Height;
	const float RemainingSize = ParentSize * (1 - GetSize());
	if (RemainingSize <= 0)
		return;

	DoUpdateDrag(HandleCorner, RemainingSize);
}

void UScrollbarComponent::DoUpdateDrag(FVector2D HandleCorner, float RemainingSize)
{
	switch (Direction)
	{
	case EScrollBarDirection::Direction_LeftToRight:
		Set(FMath::Clamp(HandleCorner.X / RemainingSize, 0.0f, 1.0f));
		break;
	case EScrollBarDirection::Direction_RightToLeft:
		Set(FMath::Clamp(1 - (HandleCorner.X / RemainingSize), 0.0f, 1.0f));
		break;
	case EScrollBarDirection::Direction_BottomToTop:
		Set(FMath::Clamp(HandleCorner.Y / RemainingSize, 0.0f, 1.0f));
		break;
	case EScrollBarDirection::Direction_TopToBottom:
		Set(FMath::Clamp(1 - (HandleCorner.Y / RemainingSize), 0.0f, 1.0f));
		break;
	default:
		break;
	}
}

bool UScrollbarComponent::MayDrag(const UPointerEventData* EventData) const
{
	return IsActiveAndEnabled() && IsInteractableInHierarchy() && IsValid(EventData) && EventData->Button == EPointerInputButton::InputButton_Left;
}

void UScrollbarComponent::ClickRepeat(TWeakObjectPtr<UPointerEventData> EventData)
{
	if(bIsPointerDownAndNotDragging)
	{
		FVector LocalMousePos;
		if (EventData.IsValid() && !FRectTransformUtility::RectangleContainsScreenPoint(HandleRect, OwnerCanvas, FVector2D(EventData->Position), LocalMousePos, true))
		{
			const float AxisCoordinate = static_cast<int32>(GetAxis()) == 0 ? LocalMousePos.X : LocalMousePos.Y;
			const float Change = AxisCoordinate < 0 ? GetSize() : -GetSize();
			SetValue(GetValue() + (ReverseValue() ? Change : -Change));
		}
	}

	if (!bIsPointerDownAndNotDragging)
	{
		bNeedSetTimer = false;
		
		if (TimerHandle.IsValid())
		{
			const auto World = GetWorld();
			if (IsValid(World))
			{
				World->GetTimerManager().ClearTimer(TimerHandle);
			}
			TimerHandle.Invalidate();
		}
	}

	if (bNeedSetTimer)
	{
		const auto World = GetWorld();
		if (IsValid(World))
		{
			World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(this, &UScrollbarComponent::ClickRepeat, 
				TWeakObjectPtr<UPointerEventData>(EventData)), 0.001f, false);
		}
	}
}

/////////////////////////////////////////////////////
