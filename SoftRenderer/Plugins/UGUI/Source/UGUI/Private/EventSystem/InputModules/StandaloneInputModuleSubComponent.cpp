#include "EventSystem/InputModules/StandaloneInputModuleSubComponent.h"
#include "Core/MathUtility.h"
#include "EventSystem/EventSystemComponent.h"
#include "EventSystem/ExecuteEvents.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UStandaloneInputModule

UStandaloneInputModuleSubComponent::UStandaloneInputModuleSubComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrevActionTime = 0;
    LastMoveVector = FVector2D::ZeroVector;
    ConsecutiveMoveCount = 0;

    LastMousePosition = FVector2D::ZeroVector;
    MousePosition = FVector2D::ZeroVector;

    CurrentFocusedGameObject = nullptr;
    InputPointerEvent = nullptr;

    HorizontalAxis = TEXT("Horizontal");
    VerticalAxis = TEXT("Vertical");
    SubmitButton = TEXT("Submit");
    CancelButton = TEXT("Cancel");

    InputActionsPerSecond = 10;
    RepeatDelay = 0.5f;

    bForceModuleActive = false;
}

bool UStandaloneInputModuleSubComponent::ShouldIgnoreEventsOnNoFocus()
{
#if PLATFORM_DESKTOP
    return true;
#else
    return false;
#endif
}

void UStandaloneInputModuleSubComponent::UpdateModule()
{
	if (ViewportClient && !ViewportClient->HasFocus() && ShouldIgnoreEventsOnNoFocus())
	{
        if (IsValid(InputPointerEvent) && IsValid(InputPointerEvent->PointerDrag) && InputPointerEvent->bDragging)
        {
            ReleaseMouse(InputPointerEvent, InputPointerEvent->PointerCurrentRaycast.GameObject);
        }
		
        InputPointerEvent = nullptr;
        return;
	}

    LastMousePosition = MousePosition;
    MousePosition = GetInput()->MousePosition();
}

void UStandaloneInputModuleSubComponent::ReleaseMouse(UPointerEventData* PointerEvent, USceneComponent* CurrentOverGo)
{
    FExecuteEvents::Execute<IPointerUpHandlerInterface>(PointerEvent->GetPointerPress(), PointerEvent);

    const auto PointerUpHandler = FExecuteEvents::GetEventHandler<IPointerClickHandlerInterface>(CurrentOverGo);

    // PointerClick and Drop events
    if (PointerEvent->GetPointerPress() == PointerUpHandler && PointerEvent->bEligibleForClick)
    {
        FExecuteEvents::Execute<IPointerClickHandlerInterface>(PointerEvent->GetPointerPress(), PointerEvent);
    }
    else if (IsValid(PointerEvent->PointerDrag) && PointerEvent->bDragging)
    {
        FExecuteEvents::ExecuteHierarchy<IDropHandlerInterface>(CurrentOverGo, PointerEvent);
    }

    PointerEvent->bEligibleForClick = false;
    PointerEvent->SetPointerPress(nullptr);
    PointerEvent->RawPointerPress = nullptr;

    if (IsValid(PointerEvent->PointerDrag) && PointerEvent->bDragging)
    {
        FExecuteEvents::Execute<IEndDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
    }

    PointerEvent->bDragging = false;
    PointerEvent->PointerDrag = nullptr;

    // redo pointer enter / exit to refresh state
    // so that if we moused over something that ignored it before
    // due to having pressed on something else
    // it now gets it.
    if (CurrentOverGo != PointerEvent->PointerEnter)
    {
        HandlePointerExitAndEnter(PointerEvent, nullptr);
        HandlePointerExitAndEnter(PointerEvent, CurrentOverGo);
    }

    InputPointerEvent = PointerEvent;
}

bool UStandaloneInputModuleSubComponent::IsModuleSupported()
{
    return bForceModuleActive || FSlateApplication::Get().IsMouseAttached();
}

bool UStandaloneInputModuleSubComponent::ShouldActivateModule()
{
    if (!Super::ShouldActivateModule())
    {
        return false;
    }

    const auto Input = GetInput();

    bool bShouldActivate = bForceModuleActive;
    bShouldActivate |= Input->GetButtonDown(SubmitButton);
    bShouldActivate |= Input->GetButtonDown(CancelButton);
    bShouldActivate |= !FMathUtility::Approximately(Input->GetAxis(HorizontalAxis), 0.0f);
    bShouldActivate |= !FMathUtility::Approximately(Input->GetAxis(VerticalAxis), 0.0f);
    bShouldActivate |= (MousePosition - LastMousePosition).SizeSquared() > 0.0f;
    bShouldActivate |= Input->GetMouseButtonDown(0);

    if (Input->TouchCount() > 0)
    {
        bShouldActivate = true;
    }

    return bShouldActivate;
}

void UStandaloneInputModuleSubComponent::ActivateModule()
{
    if (ViewportClient && !ViewportClient->HasFocus() && ShouldIgnoreEventsOnNoFocus())
    {
        return;
    }

    Super::ActivateModule();
	
    const auto Input = GetInput();

    MousePosition = Input->MousePosition();
    LastMousePosition = MousePosition;

    auto ToSelect = EventSystem->GetCurrentSelectedGameObject();
	if (!IsValid(ToSelect))
	{
        ToSelect = EventSystem->GetFirstSelectedGameObject();
	}
	
    EventSystem->SetSelectedGameObject(ToSelect, GetBaseEventData());
}

void UStandaloneInputModuleSubComponent::DeactivateModule()
{
    Super::DeactivateModule();
    ClearSelection();
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- StandaloneProcess"), STAT_UnrealGUI_StandaloneProcess, STATGROUP_UnrealGUI);
void UStandaloneInputModuleSubComponent::Process()
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_StandaloneProcess);
    
    if (ViewportClient && !ViewportClient->HasFocus() && ShouldIgnoreEventsOnNoFocus())
    {
        return;
    }

	bool bUsedEvent = SendUpdateEventToSelectedObject();

    // touch / mouse events should be processed before navigation events in case
    // they change the current selected gameobject and the submit button is a touch / mouse button.
             
    // touch needs to take precedence because of the mouse emulation layer
    if (!ProcessTouchEvents() && FSlateApplication::Get().IsMouseAttached())
    {
        ProcessMouseEvent();
    }

    if (EventSystem->bSendNavigationEvents)
    {
        if (!bUsedEvent)
        {
            bUsedEvent |= SendMoveEventToSelectedObject();
        }

        if (!bUsedEvent)
        {
            SendSubmitEventToSelectedObject();
        }
    }
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- ProcessTouchEvents"), STAT_UnrealGUI_ProcessTouchEvents, STATGROUP_UnrealGUI);
bool UStandaloneInputModuleSubComponent::ProcessTouchEvents()
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_ProcessTouchEvents);
    
    const auto Input = GetInput();
    
	for (int32 Index = 0, Count = Input->TouchCount(); Index < Count; ++Index)
	{
        const auto Touch = Input->GetTouch(Index);
        if (Touch == nullptr)
        {
            continue;
        }

        bool bPressed;
        bool bReleased;
	    bool bMoved;
        const auto Pointer = GetTouchPointerEventData(Touch, bPressed, bReleased, bMoved);

        ProcessTouchPress(Pointer, bPressed, bReleased, bMoved);

	    if (bReleased)
        {
            RemovePointerData(Pointer);
        }
	}
    
    return Input->TouchCount() > 0;
}

void UStandaloneInputModuleSubComponent::ProcessTouchPress(UPointerEventData* PointerEvent, bool bPressed, bool bReleased, bool bMoved)
{
    const auto CurrentOverGo = PointerEvent->PointerCurrentRaycast.GameObject;

    // PointerDown notification
    if (bPressed)
    {
        PointerEvent->bEligibleForClick = true;
        PointerEvent->bDragging = false;
        PointerEvent->bUseDragThreshold = true;
        PointerEvent->PointerPressRaycast = PointerEvent->PointerCurrentRaycast;

        const FVector TempPosition = PointerEvent->Position;
        PointerEvent->Position = PointerEvent->PressPosition;
        
        DeselectIfSelectionChanged(CurrentOverGo, PointerEvent);

        if (PointerEvent->PointerEnter != CurrentOverGo)
        {
            // send a pointer enter to the touched element if it isn't the one to select...
            HandlePointerExitAndEnter(PointerEvent, CurrentOverGo);
            PointerEvent->PointerEnter = CurrentOverGo;
        }

        // search for the control that will receive the press
        // if we can't find a press handler set the press
        // handler to be what would receive a click.
        auto NewPressed = FExecuteEvents::ExecuteHierarchy<IPointerDownHandlerInterface>(CurrentOverGo, PointerEvent);

        // didn't find a press handler... search for a click handler
        if (!IsValid(NewPressed))
        {
            NewPressed = FExecuteEvents::GetEventHandler<IPointerClickHandlerInterface>(CurrentOverGo);
        }

        const float Time = FApp::GetCurrentTime();

        if (NewPressed == PointerEvent->GetLastPointerPress())
        {
            const float DiffTime = Time - PointerEvent->ClickTime;
            if (DiffTime < 0.3f)
            {
                ++PointerEvent->ClickCount;
            }
            else
            {
                PointerEvent->ClickCount = 1;
            }

            PointerEvent->ClickTime = Time;
        }
        else
        {
            PointerEvent->ClickCount = 1;
        }

        PointerEvent->SetPointerPress(NewPressed);
        PointerEvent->RawPointerPress = CurrentOverGo;
        PointerEvent->ClickTime = Time;

        // Save the drag handler as well
        PointerEvent->PointerDrag = FExecuteEvents::GetEventHandler<IDragHandlerInterface>(CurrentOverGo);

        if (IsValid(PointerEvent->PointerDrag))
        {
            FExecuteEvents::Execute<IInitializePotentialDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
        }

        PointerEvent->Position = TempPosition;
        InputPointerEvent = PointerEvent;
    }
    
    if (!bReleased || bMoved)
    {
        ProcessMove(PointerEvent);
        ProcessDrag(PointerEvent);
    }

    // PointerUp notification
    if (bReleased)
    {
        FExecuteEvents::Execute<IPointerUpHandlerInterface>(PointerEvent->GetPointerPress(), PointerEvent);

        // see if we mouse up on the same element that we clicked on...
        const auto PointerUpHandler = FExecuteEvents::GetEventHandler<IPointerClickHandlerInterface>(CurrentOverGo);

        // PointerClick and Drop events
        if (PointerEvent->GetPointerPress() == PointerUpHandler && PointerEvent->bEligibleForClick)
        {
            FExecuteEvents::Execute<IPointerClickHandlerInterface>(PointerEvent->GetPointerPress(), PointerEvent);
        }
        else if (IsValid(PointerEvent->PointerDrag) && PointerEvent->bDragging)
        {
            FExecuteEvents::ExecuteHierarchy<IDropHandlerInterface>(CurrentOverGo, PointerEvent);
        }

        PointerEvent->bEligibleForClick = false;
        PointerEvent->SetPointerPress(nullptr);
        PointerEvent->RawPointerPress = nullptr;

        if (IsValid(PointerEvent->PointerDrag) && PointerEvent->bDragging)
        {
            FExecuteEvents::Execute<IEndDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
        }

        PointerEvent->bDragging = false;
        PointerEvent->PointerDrag = nullptr;

        // send exit events as we need to simulate this on touch up on touch device
        FExecuteEvents::ExecuteHierarchy<IPointerExitHandlerInterface>(PointerEvent->PointerEnter, PointerEvent);
        PointerEvent->PointerEnter = nullptr;

        InputPointerEvent = PointerEvent;
    }
}

bool UStandaloneInputModuleSubComponent::SendSubmitEventToSelectedObject()
{
    if (!IsValid(EventSystem->GetCurrentSelectedGameObject()))
    {
        return false;
    }

    const auto Input = GetInput();
    const auto Data = GetBaseEventData();
    
    if (Input->GetButtonDown(SubmitButton))
    {
        FExecuteEvents::Execute<ISubmitHandlerInterface>(EventSystem->GetCurrentSelectedGameObject(), Data);
    }

    if (Input->GetButtonDown(CancelButton))
    {
        FExecuteEvents::Execute<ICancelHandlerInterface>(EventSystem->GetCurrentSelectedGameObject(), Data);
    }
	
    return Data->IsUsed();
}

FVector2D UStandaloneInputModuleSubComponent::GetRawMoveVector()
{
    const auto Input = GetInput();
	
    FVector2D Move;
    Move.X = Input->GetAxis(HorizontalAxis);
    Move.Y = Input->GetAxis(VerticalAxis);

    if (Input->GetButtonDown(HorizontalAxis))
    {
        if (Move.X < 0)
        {
            Move.X = -1;
        }
        
        if (Move.X > 0)
        {
            Move.X = 1;
        }
    }
    
    if (Input->GetButtonDown(VerticalAxis))
    {
        if (Move.Y < 0)
        {
            Move.Y = -1;
        }
        
        if (Move.Y > 0)
        {
            Move.Y = 1;
        }
    }
    
    return Move;
}

bool UStandaloneInputModuleSubComponent::SendMoveEventToSelectedObject()
{
    const FVector2D Movement = GetRawMoveVector();
    if (FMathUtility::Approximately(Movement.X, 0) && FMathUtility::Approximately(Movement.Y, 0))
    {
        ConsecutiveMoveCount = 0;
        return false;
    }

    const float Time = FApp::GetCurrentTime();
    const bool bSimilarDir = (FVector2D::DotProduct(Movement, LastMoveVector) > 0);

    // If direction didn't change at least 90 degrees, wait for delay before allowing consequtive event.
    if (bSimilarDir && ConsecutiveMoveCount == 1)
    {
        if (Time <= PrevActionTime + RepeatDelay)
        {
            return false;
        }
    }
    // If direction changed at least 90 degree, or we already had the delay, repeat at repeat rate.
    else
    {
        if (Time <= PrevActionTime + 1.0f / InputActionsPerSecond)
        {
            return false;
        }
    }

    const auto AxisData = GetAxisEventData(Movement.X, Movement.Y, 0.6f);
    if (AxisData->MoveDir != EMoveDirection::MoveDirection_None)
    {
        FExecuteEvents::Execute<IMoveHandlerInterface>(EventSystem->GetCurrentSelectedGameObject(), AxisData);
        if (!bSimilarDir)
        {
            ConsecutiveMoveCount = 0;
        }
        ++ConsecutiveMoveCount;
        
        PrevActionTime = Time;
        LastMoveVector = Movement;
    }
    else
    {
        ConsecutiveMoveCount = 0;
    }

    return AxisData->IsUsed();
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- ProcessMouseEvent"), STAT_UnrealGUI_ProcessMouseEvent, STATGROUP_UnrealGUI);
void UStandaloneInputModuleSubComponent::ProcessMouseEvent(int32 Id)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_ProcessMouseEvent);
    
    auto& MouseData = GetMousePointerEventData(Id);
    const auto& LeftButtonData = MouseData.GetButtonState(EPointerInputButton::InputButton_Left).EventData;

    CurrentFocusedGameObject = LeftButtonData.ButtonData->PointerCurrentRaycast.GameObject;

    // Process the first mouse button fully
    ProcessMousePress(LeftButtonData);
    ProcessMove(LeftButtonData.ButtonData);
    ProcessDrag(LeftButtonData.ButtonData);

    // Now process right / middle clicks
    ProcessMousePress(MouseData.GetButtonState(EPointerInputButton::InputButton_Right).EventData);
    ProcessDrag(MouseData.GetButtonState(EPointerInputButton::InputButton_Right).EventData.ButtonData);
    ProcessMousePress(MouseData.GetButtonState(EPointerInputButton::InputButton_Middle).EventData);
    ProcessDrag(MouseData.GetButtonState(EPointerInputButton::InputButton_Middle).EventData.ButtonData);

    if (!FMathUtility::Approximately(LeftButtonData.ButtonData->ScrollDelta, 0.0f))
    {
        const auto ScrollHandler = FExecuteEvents::GetEventHandler<IScrollHandlerInterface>(LeftButtonData.ButtonData->PointerCurrentRaycast.GameObject);
        FExecuteEvents::ExecuteHierarchy<IScrollHandlerInterface>(ScrollHandler, LeftButtonData.ButtonData);
    }
}

bool UStandaloneInputModuleSubComponent::SendUpdateEventToSelectedObject()
{
    if (!IsValid(EventSystem->GetCurrentSelectedGameObject()))
    {
        return false;
    }

    const auto Data = GetBaseEventData();
    FExecuteEvents::Execute<IUpdateSelectedHandlerInterface>(EventSystem->GetCurrentSelectedGameObject(), Data);
    return Data->IsUsed();
}

void UStandaloneInputModuleSubComponent::ProcessMousePress(const FPIMMouseButtonEventData& Data)
{
    const auto PointerEvent = Data.ButtonData;
    const auto CurrentOverGo = PointerEvent->PointerCurrentRaycast.GameObject;

    // PointerDown notification
    if (Data.PressedThisFrame())
    {
        PointerEvent->bEligibleForClick = true;
        PointerEvent->Delta = FVector::ZeroVector;
        PointerEvent->bDragging = false;
        PointerEvent->bUseDragThreshold = true;
        PointerEvent->PressPosition = PointerEvent->Position;
        PointerEvent->PointerPressRaycast = PointerEvent->PointerCurrentRaycast;

        DeselectIfSelectionChanged(CurrentOverGo, PointerEvent);

        // search for the control that will receive the press
        // if we can't find a press handler set the press
        // handler to be what would receive a click.
        auto NewPressed = FExecuteEvents::ExecuteHierarchy<IPointerDownHandlerInterface>(CurrentOverGo, PointerEvent);

        // didn't find a press handler... search for a click handler
        if (!IsValid(NewPressed))
        {
            NewPressed = FExecuteEvents::GetEventHandler<IPointerClickHandlerInterface>(CurrentOverGo);
        }

        const float Time = FApp::GetCurrentTime();
        if (NewPressed == PointerEvent->GetLastPointerPress())
        {
            const float DiffTime = Time - PointerEvent->ClickTime;
            if (DiffTime < 0.3f)
            {
                ++PointerEvent->ClickCount;
            }
            else
            {
                PointerEvent->ClickCount = 1;
            }

            PointerEvent->ClickTime = Time;
        }
        else
        {
            PointerEvent->ClickCount = 1;
        }

        PointerEvent->SetPointerPress(NewPressed);
        PointerEvent->RawPointerPress = CurrentOverGo;
        PointerEvent->ClickTime = Time;
        
        // Save the drag handler as well
        PointerEvent->PointerDrag = FExecuteEvents::GetEventHandler<IDragHandlerInterface>(CurrentOverGo);

        if (IsValid(PointerEvent->PointerDrag))
        {
            FExecuteEvents::Execute<IInitializePotentialDragHandlerInterface>(PointerEvent->PointerDrag, PointerEvent);
        }

        InputPointerEvent = PointerEvent;
    }

    // PointerUp notification
    if (Data.ReleasedThisFrame())
    {
        ReleaseMouse(PointerEvent, CurrentOverGo);
    }
}

/////////////////////////////////////////////////////
