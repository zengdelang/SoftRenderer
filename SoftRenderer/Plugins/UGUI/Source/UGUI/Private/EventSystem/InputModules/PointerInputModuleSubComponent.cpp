#include "EventSystem/InputModules/PointerInputModuleSubComponent.h"
#include "Engine/Canvas.h"
#include "EventSystem/EventSystemComponent.h"
#include "EventSystem/ExecuteEvents.h"
#include "EventSystem/EventData/PointerEventData.h"

/////////////////////////////////////////////////////
// UPointerInputModule

int32 UPointerInputModuleSubComponent::KMouseLeftId = -1;
int32 UPointerInputModuleSubComponent::KMouseRightId = -2;
int32 UPointerInputModuleSubComponent::KMouseMiddleId = -3;
int32 UPointerInputModuleSubComponent::KFakeTouchesId = -4;

UPointerInputModuleSubComponent::UPointerInputModuleSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	
}

bool UPointerInputModuleSubComponent::GetPointerData(int32 Id, UPointerEventData*& Data, bool bCreate)
{
    const auto DataPtr = PointerData.Find(Id);

    if (!DataPtr && bCreate)
    {
        Data = NewObject<UPointerEventData>(this);
        Data->SetEventSystem(EventSystem);
        Data->PointerId = Id;
        PointerData.Add(Id, Data);
        return true;
    }
	
    Data = *DataPtr;
    return false;
}

void UPointerInputModuleSubComponent::RemovePointerData(const UPointerEventData* Data)
{
	if (!IsValid(Data))
		return;
	
	PointerData.Remove(Data->PointerId);
}

UPointerEventData* UPointerInputModuleSubComponent::GetTouchPointerEventData(FViewportTouchState* Input, bool& bPressed, bool& bReleased, bool& bMoved)
{
    UPointerEventData* PointerEventData = nullptr;
    GetPointerData(Input->GetFingerId(), PointerEventData, true);

    PointerEventData->Reset();

    bPressed = Input->IsPressed();
    bReleased = Input->IsEndedPhase();
    bMoved = Input->IsMovedPhase();
    
    PointerEventData->Delta = Input->GetDelta();

    PointerEventData->PressPosition = Input->GetPressedPosition();
    PointerEventData->Position = Input->GetPosition();
    PointerEventData->Button = EPointerInputButton::InputButton_Left;

    PointerEventData->PointerCurrentRaycast = FRaycastResult();
    EventSystem->RaycastAll(PointerEventData, PointerEventData->PointerCurrentRaycast, GetWorld());

    return PointerEventData;
}

void UPointerInputModuleSubComponent::CopyFromTo(const UPointerEventData* From, UPointerEventData* To)
{
    To->Position = From->Position;
    To->Delta = From->Delta;
    To->ScrollDelta = From->ScrollDelta;
    To->PointerCurrentRaycast = From->PointerCurrentRaycast;
    To->PointerEnter = From->PointerEnter;
}

EPointerFramePressState UPointerInputModuleSubComponent::StateForMouseButton(int32 ButtonId)
{
    const auto Input = GetInput();
    const bool bPressed = Input->GetMouseButtonDown(ButtonId);
    const bool bReleased = Input->GetMouseButtonUp(ButtonId);

	if (bPressed && bReleased)
        return EPointerFramePressState::FramePressState_PressedAndReleased;
	
    if (bPressed)
        return EPointerFramePressState::FramePressState_Pressed;
	
    if (bReleased)
        return EPointerFramePressState::FramePressState_Released;
	
    return EPointerFramePressState::FramePressState_NotChanged;
}

FPIMMouseState& UPointerInputModuleSubComponent::GetMousePointerEventData(int32 Id)
{
    const auto Input = GetInput();
	
	// Populate the left button...
    UPointerEventData* LeftData;
    const bool bCreated = GetPointerData(KMouseLeftId, LeftData, true);

    LeftData->Reset();

    const FVector Pos = FVector(Input->MousePosition(), 0);

    if (bCreated)
        LeftData->Position = Pos;

	if (ViewportClient && ViewportClient->IsCursorVisible())
	{
        LeftData->Delta = Pos - LeftData->Position;
        LeftData->Position = Pos;
	}
    else
    {
        // We don't want to do ANY cursor-based interaction when the mouse is locked
        LeftData->Position = FVector(-1, -1, 0);
        LeftData->Delta = FVector::ZeroVector;
    }
	
    LeftData->ScrollDelta = Input->MouseScrollDelta();
    LeftData->Button = EPointerInputButton::InputButton_Left;

    LeftData->PointerCurrentRaycast = FRaycastResult();
    EventSystem->RaycastAll(LeftData, LeftData->PointerCurrentRaycast, GetWorld());

    // copy the appropriate data into right and middle slots
    UPointerEventData* RightData;
    GetPointerData(KMouseRightId, RightData, true);
    CopyFromTo(LeftData, RightData);
    RightData->Button = EPointerInputButton::InputButton_Right;

    UPointerEventData* MiddleData;
    GetPointerData(KMouseMiddleId, MiddleData, true);
    CopyFromTo(LeftData, MiddleData);
    MiddleData->Button = EPointerInputButton::InputButton_Middle;

    MouseState.SetButtonState(EPointerInputButton::InputButton_Left, StateForMouseButton(0), LeftData);
    MouseState.SetButtonState(EPointerInputButton::InputButton_Right, StateForMouseButton(1), RightData);
    MouseState.SetButtonState(EPointerInputButton::InputButton_Middle, StateForMouseButton(2), MiddleData);

    return MouseState;
}

UPointerEventData* UPointerInputModuleSubComponent::GetLastPointerEventData(int32 Id)
{
    UPointerEventData* Data = nullptr;
    GetPointerData(Id, Data, false);
    return Data;
}

bool UPointerInputModuleSubComponent::ShouldStartDrag(const FVector& PressPos, const FVector& CurrentPos, float Threshold,
	bool bUseDragThreshold)
{
    if (!bUseDragThreshold)
        return true;

	const float X = PressPos.X - CurrentPos.X;
    const float Y = PressPos.Y - CurrentPos.Y;
    return (X * X + Y * Y) >= Threshold * Threshold;
}

void UPointerInputModuleSubComponent::ProcessMove(UPointerEventData* PointerEvent)
{
    USceneComponent* Target = (ViewportClient && ViewportClient->IsCursorVisible()) ? PointerEvent->PointerCurrentRaycast.GameObject : nullptr;
    HandlePointerExitAndEnter(PointerEvent, Target);
}

void UPointerInputModuleSubComponent::ProcessDrag(UPointerEventData* PointerEvent)
{
    if (!PointerEvent->IsPointerMoving() ||
        !ViewportClient->IsCursorVisible() ||
        !IsValid(PointerEvent->PointerDrag))
        return;

    if (!PointerEvent->bDragging
        && ShouldStartDrag(PointerEvent->PressPosition, PointerEvent->Position, EventSystem->GetPixelDragThreshold(), PointerEvent->bUseDragThreshold))
    {
        FExecuteEvents::Execute<IBeginDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
        PointerEvent->bDragging = true;
    }

    // Drag notification
    if (PointerEvent->bDragging)
    {
        // Before doing drag we should cancel any pointer down state
        // And clear selection!
        if (PointerEvent->GetPointerPress() != PointerEvent->PointerDrag)
        {
            FExecuteEvents::Execute<IPointerUpHandlerInterface>(PointerEvent->GetPointerPress(), PointerEvent);

            PointerEvent->bEligibleForClick = false;
            PointerEvent->SetPointerPress(nullptr);
            PointerEvent->RawPointerPress = nullptr;
        }
    	
        FExecuteEvents::Execute<IDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
    }
}

bool UPointerInputModuleSubComponent::IsPointerOverGameObject(int32 PointerId)
{
    const auto LastPointer = GetLastPointerEventData(PointerId);
    if (IsValid(LastPointer))
        return IsValid(LastPointer->PointerEnter);
    return false;
}

void UPointerInputModuleSubComponent::ClearSelection()
{
    for (const auto& Elem : PointerData)
    {
    	// clear all selection
        HandlePointerExitAndEnter(Elem.Value, nullptr);
    }

    PointerData.Reset();

	if (IsValid(EventSystem))
	{
        const auto EventData = GetBaseEventData();
        EventSystem->SetSelectedGameObject(nullptr, EventData);
	}
}

void UPointerInputModuleSubComponent::DeselectIfSelectionChanged(USceneComponent* CurrentOverGo, UBaseEventData* PointerEvent) const
{
    // Selection tracking
    const auto SelectHandlerGO = FExecuteEvents::GetEventHandler<ISelectHandlerInterface>(CurrentOverGo);
    // if we have clicked something new, deselect the old thing
    // leave 'selection handling' up to the press event though.
    if (SelectHandlerGO != EventSystem->GetCurrentSelectedGameObject())
    {
        EventSystem->SetSelectedGameObject(nullptr, PointerEvent);
    }
    
    UEventSystemComponent::OnSelectedGameObjectChangedEvent.Broadcast(GetWorld(), SelectHandlerGO);
}

FString UPointerInputModuleSubComponent::ToString() const
{
    FString FinalString = TEXT("");
    FinalString += FString::Printf(TEXT("Pointer Input Module of type: %s"), *StaticClass()->GetFName().ToString());
    FinalString += FString::Printf(TEXT("\n"));

    for (auto& Elem : PointerData)
    {
        if (!IsValid(Elem.Value))
            continue;
    	
        FinalString += FString::Printf(TEXT("Pointer: %d"), Elem.Key);
        FinalString += Elem.Value->ToString();
    }
	
    return FinalString;
}

void UPointerInputModuleSubComponent::ShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    Super::ShowDebugInfo(HUD, Canvas, DisplayInfo, YL, YPos);
    
    {
        const UFont* RenderFont = GEngine->GetMediumFont();
        Canvas->SetDrawColor(FColor::Yellow);

        YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Pointer Input Module of type: %s"), *StaticClass()->GetFName().ToString()), 32, YPos);
  
        for (auto& Elem : PointerData)
        {
            if (!IsValid(Elem.Value))
                continue;

            if (ViewportClient && !ViewportClient->HasMouseEvents())
            {
                if (Elem.Key == KMouseLeftId ||
                    Elem.Key == KMouseRightId ||
                    Elem.Key == KMouseMiddleId ||
                    Elem.Key == KFakeTouchesId)
                {
                    continue;
                }
            }

            YPos += Canvas->DrawText(RenderFont, TEXT("\n"), 64, YPos);
            YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Pointer: %d"), Elem.Key), 64, YPos);
            Elem.Value->ShowDebugInfo(HUD, Canvas, DisplayInfo, YL, YPos);
        }
    }
#endif
}

/////////////////////////////////////////////////////
