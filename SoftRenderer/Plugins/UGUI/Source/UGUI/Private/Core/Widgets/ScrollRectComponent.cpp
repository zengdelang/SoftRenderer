#include "Core/Widgets/ScrollRectComponent.h"
#include "Core/CanvasUpdateRegistry.h"
#include "Core/MathUtility.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Core/Layout/RectTransformUtility.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Render/CanvasManager.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UScrollRectComponent

UScrollRectComponent::UScrollRectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MovementType = EScrollRectMovementType::MovementType_Elastic;
	Elasticity = 0.1f;
	DecelerationRate = 0.135f;
	ScrollSensitivity = 1.0f;

	ContentPath.Emplace(0);
	ContentPath.Emplace(0);
	ViewportPath.Emplace(0);
	HorizontalScrollbarPath.Emplace(1);
	VerticalScrollbarPath.Emplace(2);

	HorizontalScrollbarVisibility = EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport;
	VerticalScrollbarVisibility = EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport;
	
	HorizontalScrollbarSpacing = -3;
	VerticalScrollbarSpacing = -3;

	PointerStartLocalCursor = FVector2D::ZeroVector;
	ContentStartPosition = FVector2D::ZeroVector;
	
	Velocity = FVector2D::ZeroVector;

	PrevPosition = FVector2D::ZeroVector;

	HSliderHeight = 0;
	VSliderWidth = 0;

	OldContentPosition = FVector2D::ZeroVector;
	
	bLateUpdated = false;
	ContentPosDirtyMode = 0;
	
	bHorizontal = true;
	bVertical = true;
	bInertia = true;

	bDragging = false;
	bScrolling = false;

	bHasRebuiltLayout = false;

	bHSliderExpand = false;
	bVSliderExpand = false;

	const auto CanvasSubComp = CreateDefaultSubobject<UCanvasSubComponent>(TEXT("CanvasSubComp0"));
	if (CanvasSubComp)
	{
		SubComponents.Emplace(CanvasSubComp);
	}
}

void UScrollRectComponent::SetContentPosDirtyMode(EContentPosDirtyMode DirtyMode)
{
	ContentPosDirtyMode |= DirtyMode;
	
	if (bLateUpdated.TryGetValue(false))
	{
		FVector2D Position = FVector2D::ZeroVector;
		if (IsValid(Content))
		{
			Position = Content->GetAnchoredPosition();
		}
		
		OnContentPosChangedEvent.Broadcast(ContentPosDirtyMode, Position, Position);
		ContentPosDirtyMode = 0;
	}
}

void UScrollRectComponent::Rebuild(ECanvasUpdate Executing)
{
	if (Executing == ECanvasUpdate::CanvasUpdate_Prelayout)
	{
		UpdateCachedData();
	}
	else if (Executing == ECanvasUpdate::CanvasUpdate_PostLayout)
	{
		UpdateScrollRectBounds();
		UpdateScrollbars(FVector2D::ZeroVector);
		UpdatePrevData();

		bHasRebuiltLayout = true;
	}
}

void UScrollRectComponent::UpdateCachedData()
{
	HorizontalScrollbarRect = IsValid(HorizontalScrollbar) ? HorizontalScrollbar : nullptr;
	VerticalScrollbarRect = IsValid(VerticalScrollbar) ? VerticalScrollbar : nullptr;

	// These are true if either the elements are children, or they don't exist at all.
	const bool bViewIsChild = (GetViewRect()->GetAttachParent() == this);
	const bool bHScrollbarIsChild = (!IsValid(HorizontalScrollbarRect) || HorizontalScrollbarRect->GetAttachParent() == this);
	const bool bVScrollbarIsChild = (!IsValid(VerticalScrollbarRect) || VerticalScrollbarRect->GetAttachParent() == this);
	const bool bAllAreChildren = (bViewIsChild && bHScrollbarIsChild && bVScrollbarIsChild);

	bHSliderExpand = bAllAreChildren && IsValid(HorizontalScrollbarRect) && HorizontalScrollbarVisibility == EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport;
	bVSliderExpand = bAllAreChildren && IsValid(VerticalScrollbarRect) && VerticalScrollbarVisibility == EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport;
	HSliderHeight = !IsValid(HorizontalScrollbarRect) ? 0 : HorizontalScrollbarRect->GetRect().Height;
	VSliderWidth = !IsValid(VerticalScrollbarRect) ? 0 : VerticalScrollbarRect->GetRect().Width;
}

void UScrollRectComponent::Awake()
{
	Super::Awake();

	Content = Cast<URectTransformComponent>(FindChildBehaviourComponent(ContentPath));
	SetViewport(Cast<URectTransformComponent>(FindChildBehaviourComponent(ViewportPath)));
	SetHorizontalScrollbar(Cast<UScrollbarComponent>(FindChildBehaviourComponent(HorizontalScrollbarPath)));
	SetVerticalScrollbar(Cast<UScrollbarComponent>(FindChildBehaviourComponent(VerticalScrollbarPath)));
}

void UScrollRectComponent::OnEnable()
{
	Super::OnEnable();

	if (IsValid(HorizontalScrollbar))
	{
		HorizontalScrollbar->OnValueChanged.AddUniqueDynamic(this, &UScrollRectComponent::InternalSetHorizontalNormalizedPosition);
	}
	
	if (IsValid(VerticalScrollbar))
	{
		VerticalScrollbar->OnValueChanged.AddUniqueDynamic(this, &UScrollRectComponent::InternalSetVerticalNormalizedPosition);
	}

	FCanvasUpdateRegistry::RegisterCanvasElementForLayoutRebuild(this);
	SetDirty();

	FCanvasManager::AddLateUpdateObject(this);
}

void UScrollRectComponent::OnDisable()
{
	FCanvasUpdateRegistry::UnRegisterCanvasElementForRebuild(this);

	if (IsValid(HorizontalScrollbar))
	{
		HorizontalScrollbar->OnValueChanged.RemoveAll(this);
	}

	if (IsValid(VerticalScrollbar))
	{
		VerticalScrollbar->OnValueChanged.RemoveAll(this);
	}

	bDragging = false;
	bScrolling = false;
	bHasRebuiltLayout = false;
	Velocity = FVector2D::ZeroVector;
	FLayoutRebuilder::MarkLayoutForRebuild(this);

	FCanvasManager::RemoveLateUpdateObject(this);
	
	Super::OnDisable();
}

void UScrollRectComponent::OnRectTransformDimensionsChange()
{
	Super::OnRectTransformDimensionsChange();

	SetDirty();
}

#if WITH_EDITORONLY_DATA

void UScrollRectComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SetDirtyCaching();
}

#endif

bool UScrollRectComponent::IsScrollRectActive() const
{
	return IsActiveAndEnabled() && IsValid(Content);
}

void UScrollRectComponent::EnsureLayoutHasRebuilt() const
{
	if (!bHasRebuiltLayout && !FCanvasUpdateRegistry::IsRebuildingLayout())
	{
		UCanvasSubComponent::ForceUpdateCanvases();
	}
}

void UScrollRectComponent::OnScroll(UPointerEventData* EventData)
{
	if (!IsValid(EventData))
		return;
	
	if (!IsScrollRectActive())
		return;

	EnsureLayoutHasRebuilt();
	UpdateScrollRectBounds();

	FVector2D Delta = FVector2D(0, EventData->ScrollDelta);
	// Down is positive for scroll events, while in UI system up is positive.
	Delta.Y *= -1;
	if (bVertical && !bHorizontal)
	{
		if (FMath::Abs(Delta.X) > FMath::Abs(Delta.Y))
			Delta.Y = Delta.X;
		Delta.X = 0;
	}
	
	if (bHorizontal && !bVertical)
	{
		if (FMath::Abs(Delta.Y) > FMath::Abs(Delta.X))
			Delta.X = Delta.Y;
		Delta.Y = 0;
	}

	if (EventData->IsScrolling())
		bScrolling = true;

	FVector2D Position = Content->GetAnchoredPosition();
	Position += Delta * ScrollSensitivity;
	if (MovementType == EScrollRectMovementType::MovementType_Clamped)
	{
		Position += CalculateOffset(Position - Content->GetAnchoredPosition());
	}

	SetContentAnchoredPosition(Position);
	UpdateScrollRectBounds();
}

void UScrollRectComponent::OnInitializePotentialDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	Velocity = FVector2D::ZeroVector;
}

void UScrollRectComponent::OnBeginDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (!IsScrollRectActive())
		return;

	UpdateScrollRectBounds();

	PointerStartLocalCursor = FVector2D::ZeroVector;
	FRectTransformUtility::ScreenPointToLocalPointInRectangle(GetViewRect(), OwnerCanvas, FVector2D(EventData->Position), PointerStartLocalCursor);
	ContentStartPosition = Content->GetAnchoredPosition();
	OnBeginDragEvent.Broadcast(FVector2D(EventData->Position));
	bDragging = true;
}

void UScrollRectComponent::OnEndDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	OnEndDragEvent.Broadcast(FVector2D(EventData->Position));
	bDragging = false;
}

void UScrollRectComponent::OnDrag(UPointerEventData* EventData)
{
	if (!bDragging)
		return;

	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (!IsScrollRectActive())
		return;

	FVector2D LocalCursor;
	if (!FRectTransformUtility::ScreenPointToLocalPointInRectangle(GetViewRect(), OwnerCanvas, FVector2D(EventData->Position), LocalCursor))
		return;

	UpdateScrollRectBounds();

	const FVector2D PointerDelta = LocalCursor - PointerStartLocalCursor;
	FVector2D Position = ContentStartPosition + PointerDelta;
	
	// Offset to get content into place in the view.
	const FVector2D Offset = CalculateOffset(Position - Content->GetAnchoredPosition());
	Position += Offset;
	if (MovementType == EScrollRectMovementType::MovementType_Elastic)
	{	
		if (Offset.X != 0)
			Position.X = Position.X - RubberDelta(Offset.X, ViewBounds.GetSize().X);
		if (Offset.Y != 0)
			Position.Y = Position.Y - RubberDelta(Offset.Y, ViewBounds.GetSize().Y);
	}

	SetContentAnchoredPosition(Position);
	OnDragEvent.Broadcast(FVector2D(EventData->Position));
}

void UScrollRectComponent::SetContentAnchoredPosition(FVector2D InPosition)
{
	if (!bHorizontal)
		InPosition.X = Content->GetAnchoredPosition().X;
	
	if (!bVertical)
		InPosition.Y = Content->GetAnchoredPosition().Y;

	if (InPosition != Content->GetAnchoredPosition())
	{
		Content->SetAnchoredPosition(InPosition);
		UpdateScrollRectBounds();

		SetContentPosDirtyMode(EContentPosDirtyMode::DirtyMode_Normal);
	}
}

void UScrollRectComponent::LateUpdate()
{
	if (!IsValid(Content))
		return;

	EnsureLayoutHasRebuilt();
	UpdateScrollRectBounds();

	FVector2D Position = Content->GetAnchoredPosition();
	
	const float DeltaTime = FApp::GetDeltaTime();
	FVector2D Offset = CalculateOffset(FVector2D::ZeroVector);
	if (!bDragging && (Offset != FVector2D::ZeroVector || Velocity != FVector2D::ZeroVector))
	{
		for (int32 Axis = 0; Axis < 2; ++Axis)
		{
			// Apply spring physics if movement is elastic and content has an offset from the view.
			if (MovementType == EScrollRectMovementType::MovementType_Elastic && Offset[Axis] != 0)
			{
				float Speed = Velocity[Axis];
				float SmoothTime = Elasticity;
				if (bScrolling)
					SmoothTime *= 3.0f;

				Position[Axis] = FMathUtility::SmoothDamp(Position[Axis], Position[Axis] + Offset[Axis],
					Speed, SmoothTime, MAX_flt, DeltaTime);
				if (FMath::Abs(Speed) < 1)
					Speed = 0;

				Velocity[Axis] = Speed;
			}
			// Else move content according to velocity with deceleration applied.
			else if (bInertia)
			{
				Velocity[Axis] *= FMath::Pow(DecelerationRate, DeltaTime);
				if (FMath::Abs(Velocity[Axis]) < 1)
					Velocity[Axis] = 0;
				Position[Axis] += Velocity[Axis] * DeltaTime;
			}
			// If we have neither elasticity or friction, there shouldn't be any velocity.
			else
			{
				Velocity[Axis] = 0;
			}
		}

		if (MovementType == EScrollRectMovementType::MovementType_Clamped)
		{
			Offset = CalculateOffset(Position - Content->GetAnchoredPosition());
			Position += Offset;
		}
		
		SetContentAnchoredPosition(Position);
	}

	if (bDragging && bInertia)
	{
		const FVector2D NewVelocity = (Content->GetAnchoredPosition() - PrevPosition) / DeltaTime;
		Velocity = FMath::Lerp(Velocity, NewVelocity, FMath::Clamp(DeltaTime * 10, 0.0f, 1.0f));
	}

	if (ViewBounds != PrevViewBounds || ContentBounds != PrevContentBounds || Content->GetAnchoredPosition() != PrevPosition)
	{
		UpdateScrollbars(Offset);
		OnValueChanged.Broadcast(GetNormalizedPosition());
		UpdatePrevData();
	}

	if (ContentPosDirtyMode != 0)
	{
		OnContentPosChangedEvent.Broadcast(ContentPosDirtyMode, OldContentPosition, Position);
		ContentPosDirtyMode = 0;
	}

	UpdateScrollbarVisibility();
	bScrolling = false;
	
	bLateUpdated = true;
	OldContentPosition = Position;
}

void UScrollRectComponent::UpdatePrevData()
{
	if (!IsValid(Content))
		PrevPosition = FVector2D::ZeroVector;
	else
		PrevPosition = Content->GetAnchoredPosition();

	PrevViewBounds = ViewBounds;
	PrevContentBounds = ContentBounds;
}

void UScrollRectComponent::UpdateScrollbars(const FVector2D& InOffset)
{
	if (IsValid(HorizontalScrollbar))
	{
		if (ContentBounds.GetSize().X > 0)
		{
			HorizontalScrollbar->SetSize(FMath::Clamp((ViewBounds.GetSize().X - FMath::Abs(InOffset.X)) / ContentBounds.GetSize().X, 0.0f, 1.0f));
		}
		else
		{
			HorizontalScrollbar->SetSize(1);
		}

		HorizontalScrollbar->SetValue(GetHorizontalNormalizedPosition());
	}

	if (IsValid(VerticalScrollbar))
	{
		if (ContentBounds.GetSize().Y > 0)
		{
			VerticalScrollbar->SetSize(FMath::Clamp((ViewBounds.GetSize().Y - FMath::Abs(InOffset.Y)) / ContentBounds.GetSize().Y, 0.0f, 1.0f));
		}
		else
		{
			VerticalScrollbar->SetSize(1);
		}

		VerticalScrollbar->SetValue(GetVerticalNormalizedPosition());
	}
}

FVector2D UScrollRectComponent::GetNormalizedPosition()
{
	return FVector2D(GetHorizontalNormalizedPosition(), GetVerticalNormalizedPosition());
}

void UScrollRectComponent::SetNormalizedPosition(FVector2D InNormalizedPosition)
{
	InternalSetNormalizedPosition(InNormalizedPosition.X, 0);
	InternalSetNormalizedPosition(InNormalizedPosition.Y, 1);
}

float UScrollRectComponent::GetHorizontalNormalizedPosition()
{
	UpdateScrollRectBounds();

	const float ContentBoundSizeX = ContentBounds.GetSize().X;
	const float ViewBoundSizeX = ViewBounds.GetSize().X;

	if ((ContentBoundSizeX <= ViewBoundSizeX) || FMathUtility::Approximately(ContentBoundSizeX, ViewBoundSizeX))
		return (ViewBounds.GetMin().X > ContentBounds.GetMin().X) ? 1 : 0;
	return (ViewBounds.GetMin().X - ContentBounds.GetMin().X) / (ContentBoundSizeX - ViewBoundSizeX);
}

void UScrollRectComponent::SetHorizontalNormalizedPosition(float InValue)
{
	InternalSetNormalizedPosition(InValue, 0);
}

float UScrollRectComponent::GetVerticalNormalizedPosition()
{
	UpdateScrollRectBounds();

	const float ContentBoundSizeY = ContentBounds.GetSize().Y;
	const float ViewBoundSizeY = ViewBounds.GetSize().Y;
	
	if ((ContentBoundSizeY <= ViewBoundSizeY) || FMathUtility::Approximately(ContentBoundSizeY, ViewBoundSizeY))
		return (ViewBounds.GetMin().Y > ContentBounds.GetMin().Y) ? 1 : 0;
	return (ViewBounds.GetMin().Y - ContentBounds.GetMin().Y) / (ContentBoundSizeY - ViewBoundSizeY);
}

void UScrollRectComponent::SetVerticalNormalizedPosition(float InValue)
{
	InternalSetNormalizedPosition(InValue, 1);
}

void UScrollRectComponent::InternalSetHorizontalNormalizedPosition(float InValue)
{
	InternalSetNormalizedPosition(InValue, 0);
}

void UScrollRectComponent::InternalSetVerticalNormalizedPosition(float InValue)
{
	InternalSetNormalizedPosition(InValue, 1);
}

void UScrollRectComponent::InternalSetNormalizedPosition(float InValue, int32 InAxis)
{
	EnsureLayoutHasRebuilt();
	UpdateScrollRectBounds();

	FVector LocalPosition = Content->GetLocalLocation();

	// How much the content is larger than the view.
	const float HiddenLength = ContentBounds.GetSize()[InAxis] - ViewBounds.GetSize()[InAxis];
	// Where the position of the lower left corner of the content bounds should be, in the space of the view.
	const float ContentBoundsMinPosition = ViewBounds.GetMin()[InAxis] - InValue * HiddenLength;
	// The new content localPosition, in the space of the view.
	const float NewLocalPosition = LocalPosition[InAxis] + ContentBoundsMinPosition - ContentBounds.GetMin()[InAxis];

	if (FMath::Abs(LocalPosition[InAxis] - NewLocalPosition) > 0.01f)
	{
		LocalPosition[InAxis] = NewLocalPosition;
		Content->SetLocalLocation(LocalPosition);
		Velocity[InAxis] = 0;
		UpdateScrollRectBounds();

		SetContentPosDirtyMode(EContentPosDirtyMode::DirtyMode_Normal);
	}
}

float UScrollRectComponent::RubberDelta(float OverStretching, float ViewSize)
{
	const float Sign = OverStretching < 0.0 ? -1 : 1;
	return (1 - (1 / ((FMath::Abs(OverStretching) * 0.55f / ViewSize) + 1))) * ViewSize * Sign;
}

bool UScrollRectComponent::IsHScrollingNeeded() const
{
	return ContentBounds.GetSize().X > ViewBounds.GetSize().X + 0.01f;
}

bool UScrollRectComponent::IsVScrollingNeeded() const
{
	return ContentBounds.GetSize().Y > ViewBounds.GetSize().Y + 0.01f;
}

void UScrollRectComponent::SetLayoutHorizontal()
{
	URectTransformComponent* TargetViewRect = GetViewRect();
	
	if (bHSliderExpand || bVSliderExpand)
	{
		// Make view full size to see if content fits.
		TargetViewRect->SetAnchorAndSizeAndPosition(FVector2D::ZeroVector, FVector2D(1,1), 
			FVector2D::ZeroVector,FVector2D::ZeroVector);
		
		// Recalculate content layout with this size to see if it fits when there are no scrollbars.
		FLayoutRebuilder::ForceRebuildLayoutImmediate(Content);

		const auto& TargetRect = TargetViewRect->GetRect();
		ViewBounds = FBounds(FVector(TargetRect.GetCenter(), 0), FVector(TargetRect.GetSize(), 0));
		ContentBounds = GetBounds();
	}

	// If it doesn't fit vertically, enable vertical scrollbar and shrink view horizontally to make room for it.
	if (bVSliderExpand && IsVScrollingNeeded())
	{
		TargetViewRect->SetSizeDelta(FVector2D(-(VSliderWidth + VerticalScrollbarSpacing), TargetViewRect->GetSizeDelta().Y));
		
		// Recalculate content layout with this size to see if it fits vertically
		// when there is a vertical scrollbar (which may reflowed the content to make it taller).
		FLayoutRebuilder::ForceRebuildLayoutImmediate(Content);

		const auto& TargetRect = TargetViewRect->GetRect();
		ViewBounds = FBounds(FVector(TargetRect.GetCenter(), 0), FVector(TargetRect.GetSize(), 0));
		ContentBounds = GetBounds();
	}

	// If it doesn't fit horizontally, enable horizontal scrollbar and shrink view vertically to make room for it.
	if (bHSliderExpand && IsHScrollingNeeded())
	{
		TargetViewRect->SetSizeDelta(FVector2D(TargetViewRect->GetSizeDelta().X, -(HSliderHeight + HorizontalScrollbarSpacing)));

		const auto& TargetRect = TargetViewRect->GetRect();
		ViewBounds = FBounds(FVector(TargetRect.GetCenter(), 0), FVector(TargetRect.GetSize(), 0));
		ContentBounds = GetBounds();
	}

	// If the vertical slider didn't kick in the first time, and the horizontal one did,
	// we need to check again if the vertical slider now needs to kick in.
	// If it doesn't fit vertically, enable vertical scrollbar and shrink view horizontally to make room for it.
	if (bVSliderExpand && IsVScrollingNeeded() && TargetViewRect->GetSizeDelta().X == 0 && TargetViewRect->GetSizeDelta().Y < 0)
	{
		TargetViewRect->SetSizeDelta(FVector2D(-(VSliderWidth + VerticalScrollbarSpacing), TargetViewRect->GetSizeDelta().Y));
	}
}

void UScrollRectComponent::SetLayoutVertical()
{
	UpdateScrollbarLayout();

	const URectTransformComponent* TargetViewRect = GetViewRect();
	const auto& TargetRect = TargetViewRect->GetRect();	
	ViewBounds = FBounds(FVector(TargetRect.GetCenter(), 0), FVector(TargetRect.GetSize(), 0));
	ContentBounds = GetBounds();
}

void UScrollRectComponent::UpdateScrollbarVisibility() const
{
	UpdateOneScrollbarVisibility(IsVScrollingNeeded(), bVertical, VerticalScrollbarVisibility, VerticalScrollbar);
	UpdateOneScrollbarVisibility(IsHScrollingNeeded(), bHorizontal, HorizontalScrollbarVisibility, HorizontalScrollbar);
}

void UScrollRectComponent::UpdateOneScrollbarVisibility(bool bXScrollingNeeded, bool bXAxisEnabled,
	EScrollRectScrollbarVisibility ScrollbarVisibility, UScrollbarComponent* Scrollbar)
{
	if (IsValid(Scrollbar))
	{
		if (ScrollbarVisibility == EScrollRectScrollbarVisibility::ScrollbarVisibility_Permanent)
		{
			if (Scrollbar->IsEnabled() != bXAxisEnabled)
				Scrollbar->SetEnabled(bXAxisEnabled);
		}
		else
		{
			if (Scrollbar->IsEnabled() != bXScrollingNeeded)
				Scrollbar->SetEnabled(bXScrollingNeeded);
		}
	}
}

void UScrollRectComponent::UpdateScrollbarLayout() const
{
	if (bVSliderExpand && IsValid(HorizontalScrollbar))
	{
		if (IsVScrollingNeeded())
		{
			HorizontalScrollbarRect->SetAnchorAndSizeAndPosition(FVector2D(0, HorizontalScrollbarRect->GetAnchorMin().Y), FVector2D(1, HorizontalScrollbarRect->GetAnchorMax().Y),
				FVector2D(-(VSliderWidth + VerticalScrollbarSpacing), HorizontalScrollbarRect->GetSizeDelta().Y), FVector2D(0, HorizontalScrollbarRect->GetAnchoredPosition().Y));
		}
		else
		{
			HorizontalScrollbarRect->SetAnchorAndSizeAndPosition(FVector2D(0, HorizontalScrollbarRect->GetAnchorMin().Y), FVector2D(1, HorizontalScrollbarRect->GetAnchorMax().Y),
				FVector2D(0, HorizontalScrollbarRect->GetSizeDelta().Y),FVector2D(0, HorizontalScrollbarRect->GetAnchoredPosition().Y));
		}
	}

	if (bHSliderExpand && IsValid(VerticalScrollbar))
	{
		if (IsHScrollingNeeded())
		{
			VerticalScrollbarRect->SetAnchorAndSizeAndPosition(FVector2D(VerticalScrollbarRect->GetAnchorMin().X, 0), FVector2D(VerticalScrollbarRect->GetAnchorMax().X, 1),
				FVector2D(VerticalScrollbarRect->GetSizeDelta().X, -(HSliderHeight + HorizontalScrollbarSpacing)), FVector2D(VerticalScrollbarRect->GetAnchoredPosition().X, 0));
		}
		else
		{
			VerticalScrollbarRect->SetAnchorAndSizeAndPosition(FVector2D(VerticalScrollbarRect->GetAnchorMin().X, 0), FVector2D(VerticalScrollbarRect->GetAnchorMax().X, 1),
				FVector2D(VerticalScrollbarRect->GetSizeDelta().X, 0), FVector2D(VerticalScrollbarRect->GetAnchoredPosition().X, 0));
		}
	}
}

void UScrollRectComponent::UpdateScrollRectBounds()
{
	const URectTransformComponent* TargetViewRect = GetViewRect();
	const auto& TargetRect = TargetViewRect->GetRect();
	
	ViewBounds = FBounds(FVector(TargetRect.GetCenter(), 0), FVector(TargetRect.GetSize(), 0));
	ContentBounds = GetBounds();

	if (!IsValid(Content))
		return;

	FVector ContentSize = ContentBounds.GetSize();
	FVector ContentPos = ContentBounds.Center;
	const FVector2D ContentPivot = Content->GetPivot();

	AdjustBounds(ViewBounds, ContentPivot, ContentSize, ContentPos);
	ContentBounds.SetSize(ContentSize);
	ContentBounds.Center = ContentPos;

	if (MovementType == EScrollRectMovementType::MovementType_Clamped)
	{
		// Adjust content so that content bounds bottom (right side) is never higher (to the left) than the view bounds bottom (right side).
		// top (left side) is never lower (to the right) than the view bounds top (left side).
		// All this can happen if content has shrunk.
		// This works because content size is at least as big as view size (because of the call to InternalUpdateBounds above).
		FVector2D Delta = FVector2D::ZeroVector;
		if (ViewBounds.GetMax().X > ContentBounds.GetMax().X)
		{
			Delta.X = FMath::Min(ViewBounds.GetMin().X - ContentBounds.GetMin().X, ViewBounds.GetMax().X - ContentBounds.GetMax().X);
		}
		else if (ViewBounds.GetMin().X < ContentBounds.GetMin().X)
		{
			Delta.X = FMath::Max(ViewBounds.GetMin().X - ContentBounds.GetMin().X, ViewBounds.GetMax().X - ContentBounds.GetMax().X);
		}

		if (ViewBounds.GetMin().Y < ContentBounds.GetMin().Y)
		{
			Delta.Y = FMath::Max(ViewBounds.GetMin().Y - ContentBounds.GetMin().Y, ViewBounds.GetMax().Y - ContentBounds.GetMax().Y);
		}
		else if (ViewBounds.GetMax().Y > ContentBounds.GetMax().Y)
		{
			Delta.Y = FMath::Min(ViewBounds.GetMin().Y - ContentBounds.GetMin().Y, ViewBounds.GetMax().Y - ContentBounds.GetMax().Y);
		}

		constexpr float Epsilon = 1.401298E-45f;
		if (Delta.SizeSquared() > Epsilon)
		{
			ContentPos = FVector(Content->GetAnchoredPosition() + Delta, 0);
			
			if (!bHorizontal)
				ContentPos.X = Content->GetAnchoredPosition().X;
			
			if (!bVertical)
				ContentPos.X = Content->GetAnchoredPosition().Y;
			
			AdjustBounds(ViewBounds, ContentPivot, ContentSize, ContentPos);
		}
	}
}

void UScrollRectComponent::AdjustBounds(const FBounds& InViewBounds, const FVector2D& InContentPivot, FVector& OutContentSize, FVector& OutContentPos)
{
	// Make sure content bounds are at least as large as view by adding padding if not.
	// One might think at first that if the content is smaller than the view, scrolling should be allowed.
	// However, that's not how scroll views normally work.
	// Scrolling is *only* possible when content is *larger* than view.
	// We use the pivot of the content rect to decide in which directions the content bounds should be expanded.
	// E.g. if pivot is at top, bounds are expanded downwards.
	// This also works nicely when ContentSizeFitter is used on the content.
	const FVector& Excess = InViewBounds.GetSize() - OutContentSize;
	
	if (Excess.X > 0)
	{
		OutContentPos.X -= Excess.X * (InContentPivot.X - 0.5f);
		OutContentSize.X = InViewBounds.GetSize().X;
	}
	
	if (Excess.Y > 0)
	{
		OutContentPos.Y -= Excess.Y * (InContentPivot.Y - 0.5f);
		OutContentSize.Y = InViewBounds.GetSize().Y;
	}
}

FBounds UScrollRectComponent::GetBounds()
{
	if (!IsValid(Content))
		return FBounds();

	FVector Corners[4];
	Content->GetWorldCorners(Corners);
	const FTransform& ViewWorldToLocalMatrix = GetViewRect()->GetComponentTransform().Inverse();
	return InternalGetBounds(Corners, ViewWorldToLocalMatrix);
}

FBounds UScrollRectComponent::InternalGetBounds(FVector(&Corners)[4], const FTransform& ViewWorldToLocalMatrix)
{	
	FVector VMin = FVector(MAX_flt, MAX_flt, MAX_flt);
	FVector VMax = FVector(-MAX_flt, -MAX_flt, -MAX_flt);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		const FVector V = ViewWorldToLocalMatrix.TransformPosition(Corners[Index]);
		VMin = FVector(FMath::Min(V.X, VMin.X), FMath::Min(V.Y, VMin.Y), FMath::Min(V.Z, VMin.Z));
		VMax = FVector(FMath::Max(V.X, VMax.X), FMath::Max(V.Y, VMax.Y), FMath::Max(V.Z, VMax.Z));
	}

	FBounds Bounds = FBounds(VMin, FVector::ZeroVector);
	Bounds.Encapsulate(VMax);
	return Bounds;
}

FVector2D UScrollRectComponent::CalculateOffset(const FVector2D& InDelta) const
{
	return InternalCalculateOffset(ViewBounds, ContentBounds, bHorizontal, bVertical, MovementType, InDelta);
}

FVector2D UScrollRectComponent::InternalCalculateOffset(const FBounds& InViewBounds, const FBounds& InContentBounds,
	bool bInHorizontal, bool bInVertical, EScrollRectMovementType InMovementType, const FVector2D& InDelta)
{
	FVector2D Offset = FVector2D::ZeroVector;
	if (InMovementType == EScrollRectMovementType::MovementType_Unrestricted)
		return Offset;

	FVector Min = InContentBounds.GetMin();
	FVector Max = InContentBounds.GetMax();

	// min/max offset extracted to check if approximately 0 and avoid recalculating layout every frame

	if (bInHorizontal)
	{
		Min.X += InDelta.X;
		Max.X += InDelta.X;

		const float MaxOffset = InViewBounds.GetMax().X - Max.X;
		const float MinOffset = InViewBounds.GetMin().X - Min.X;

		if (MinOffset < -0.001f)
		{
			Offset.X = MinOffset;
		}
		else if (MaxOffset > 0.001f)
		{
			Offset.X = MaxOffset;
		}
	}

	if (bInVertical)
	{
		Min.Y += InDelta.Y;
		Max.Y += InDelta.Y;

		const float MaxOffset = InViewBounds.GetMax().Y - Max.Y;
		const float MinOffset = InViewBounds.GetMin().Y - Min.Y;

		if (MaxOffset > 0.001f)
		{
			Offset.Y = MaxOffset;
		}
		else if (MinOffset < -0.001f)
		{
			Offset.Y = MinOffset;
		}
	}

	return Offset;
}

void UScrollRectComponent::StartAnimation(FVector2D TargetPos, bool bResetStartPos, bool bUseAnimation)
{
	if (!bUseAnimation)
	{
		SetContentAnchoredPosition(TargetPos);
		return;
	}

	if (bResetStartPos)
	{
		SetContentAnchoredPosition(FVector2D::ZeroVector);
	}

	// TODO
	/*isAnimating = true;
	m_StartPos = m_Content.anchoredPosition;
	m_EndPos = targetPos;
	m_Velocity = Vector2.zero;
	m_StartAnimationTime = 0;*/
}

void UScrollRectComponent::SetDirty()
{
	if (!IsScrollRectActive())
		return;

	if (!bLateUpdated.TryGetValue(false))
	{
		bHasRebuiltLayout = false;
	}
	SetContentPosDirtyMode(EContentPosDirtyMode::DirtyMode_LayoutDirty);
	
	FLayoutRebuilder::MarkLayoutForRebuild(this);
}

void UScrollRectComponent::SetDirtyCaching()
{
	if (!IsScrollRectActive())
		return;

	if (!bLateUpdated.TryGetValue(false))
	{
		bHasRebuiltLayout = false;
	}
	SetContentPosDirtyMode(EContentPosDirtyMode::DirtyMode_LayoutDirty);
	
	FCanvasUpdateRegistry::RegisterCanvasElementForLayoutRebuild(this);
	FLayoutRebuilder::MarkLayoutForRebuild(this);
}

/////////////////////////////////////////////////////
