#include "EventSystem/EventSystemComponent.h"
#include "EventSystem/ExecuteEvents.h"
#include "EventSystem/EventData/BaseEventData.h"
#include "EventSystem/InputModules/BaseInputModuleSubComponent.h"
#include "EventSystem/InputModules/PointerInputModuleSubComponent.h"
#include "EventSystem/InputModules/StandaloneInputModuleSubComponent.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "UGUI.h"
#include "UGUIWorldSubsystem.h"

#if WITH_EDITORONLY_DATA
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Logging/TokenizedMessage.h"
#endif

/////////////////////////////////////////////////////
// FEventSystemInputProcessor

FEventSystemInputProcessor::FEventSystemInputProcessor(UEventSystemComponent* InEventSystemComponent)
{
    EventSystemComponent = InEventSystemComponent;
}

void FEventSystemInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
    if (EventSystemComponent.IsValid())
    {
        EventSystemComponent->TickEventSystem(DeltaTime);
    }
}

/////////////////////////////////////////////////////
// UEventSystem

TAutoConsoleVariable<int32> CVarShowEventSystemDebugInfo(
    TEXT("UGUI.ShowEventSystemDebugInfo"),
    1,
    TEXT("0 - Do not print.")
    TEXT("1 - Print."));

TMap<TWeakObjectPtr<UWorld>, TArray<TWeakObjectPtr<UEventSystemComponent>>> UEventSystemComponent::EventSystems;
FOnSelectedGameObjectChangedEvent UEventSystemComponent::OnSelectedGameObjectChangedEvent;

UEventSystemComponent::UEventSystemComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SizeDelta = FVector2D::ZeroVector;
	
    CurrentInputModule = nullptr;
    FirstSelected = nullptr;
    CurrentSelected = nullptr;
    
    DragThreshold = 10;
    
    DummyData = nullptr;

    bSendNavigationEvents = true;
    bSelectionGuard = false;

#if WITH_EDITORONLY_DATA
    bCheckMultipleComponents = true;
#endif
    
#if WITH_EDITOR
    bRegisterInEditor = false;
#endif
    
    const auto CanvasRenderSubComp = CreateDefaultSubobject<UStandaloneInputModuleSubComponent>(TEXT("StandaloneInputModuleSubComponent0"));
    if (CanvasRenderSubComp)
    {
        SubComponents.Emplace(CanvasRenderSubComp);
    }
}

UEventSystemComponent* UEventSystemComponent::GetCurrentEventSystem(UWorld* InWorld)
{
    const auto& EventSystemListPtr = EventSystems.Find(InWorld);
    if (EventSystemListPtr)
    {
        auto& EventSystemList = *EventSystemListPtr;
        for (constexpr int32 Index = 0; Index < EventSystemList.Num();)
        {
            if (EventSystemList[Index].IsValid())
            {
                return EventSystemList[Index].Get();
            }

            EventSystemList.RemoveAt(Index, 1, false);
        }
    }

    return nullptr;
}

void UEventSystemComponent::SetCurrentEventSystem(UEventSystemComponent* InEventSystem)
{
    if (!IsValid(InEventSystem))
    {
        return;
    }

    const auto World = InEventSystem->GetWorld();
    const auto& EventSystemListPtr = EventSystems.Find(World);
    if (EventSystemListPtr)
    {
        auto& EventSystemList = *EventSystemListPtr;
        const int32 Index = EventSystemList.IndexOfByKey(InEventSystem);
        if (Index >= 0)
        {
            EventSystemList.RemoveAt(Index, 1, false);
            EventSystemList.Insert(InEventSystem, 0);
        }
    }
}

void UEventSystemComponent::Awake()
{
    Super::Awake();

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    AHUD::OnShowDebugInfo.RemoveAll(this);
    AHUD::OnShowDebugInfo.AddUObject(this, &UEventSystemComponent::ShowDebugInfo);
#endif
}

void UEventSystemComponent::OnEnable()
{
    Super::OnEnable();
    
    const auto World = GetWorld();
#if WITH_EDITOR
    if (bRegisterInEditor || (World && World->IsGameWorld()))
    {
        EventSystems.Find(World);
        const auto& EventSystemListPtr = EventSystems.Find(World);
        if (EventSystemListPtr)
        {
            EventSystemListPtr->Add(this);
        }
        else
        {
            TArray<TWeakObjectPtr<UEventSystemComponent>> EventSystemList;
            EventSystemList.Add(this);
            EventSystems.Emplace(World, EventSystemList);
        }
    }
#else
    const auto& EventSystemListPtr = EventSystems.Find(World);
    if (EventSystemListPtr)
    {
        EventSystemListPtr->Add(this);
    }
    else
    {
        TArray<TWeakObjectPtr<UEventSystemComponent>> EventSystemList;
        EventSystemList.Add(this);
        EventSystems.Emplace(World, EventSystemList);
    }
#endif

#if WITH_EDITOR
    if (World && World->IsGameWorld())
#endif
    {
        const auto& EventSystemArrayPtr = EventSystems.Find(World);
        if (EventSystemArrayPtr && EventSystemArrayPtr->Num() > 1 && GetOwner())
        {
            UE_LOG(LogUGUI, Warning, TEXT("UEventSystemComponent --- There are multiple EventSystemComponent components, but only one can work. WidgetAcotr : %s"), *GetOwner()->GetFullName());

#if WITH_EDITORONLY_DATA
            if (bCheckMultipleComponents)
            {
                FNotificationInfo Info(FText::FromString(TEXT("There are multiple EventSystemComponent components, Open the messagelog tab to view the details.")));
                Info.Image = FEditorStyle::GetBrush(FTokenizedMessage::GetSeverityIconName(EMessageSeverity::Warning));
                Info.bFireAndForget = true;
                Info.bUseThrobber = false;
                Info.ExpireDuration = 4.f;
                FSlateNotificationManager::Get().AddNotification(Info);
            }
#endif
        }
    }
    
#if WITH_EDITOR
    if (bRegisterInEditor || (World && World->IsGameWorld()))
#else
    if (World && World->IsGameWorld())
#endif
    {
        EventSystemInputProcessor = MakeShareable(new FEventSystemInputProcessor(this));
        if (FSlateApplication::IsInitialized())
        {
            FSlateApplication::Get().RegisterInputPreProcessor(EventSystemInputProcessor);
        }
    }
}

void UEventSystemComponent::OnDisable()
{
    const auto World = GetWorld();
#if WITH_EDITOR
    if (bRegisterInEditor || (World && World->IsGameWorld()))
    {
        if (IsValid(CurrentInputModule))
        {
            CurrentInputModule->DeactivateModule();
            CurrentInputModule = nullptr;
        }
        
        const auto& EventSystemListPtr = EventSystems.Find(World);
        if (EventSystemListPtr)
        {
            EventSystemListPtr->RemoveSwap(this);
        }
    }
#else
    if (IsValid(CurrentInputModule))
    {
        CurrentInputModule->DeactivateModule();
        CurrentInputModule = nullptr;
    }
        
    const auto& EventSystemListPtr = EventSystems.Find(World);
    if (EventSystemListPtr)
    {
        EventSystemListPtr->RemoveSwap(this);
    }
#endif

    if (EventSystemInputProcessor.IsValid())
    {
        if (FSlateApplication::IsInitialized())
        {
            FSlateApplication::Get().UnregisterInputPreProcessor(EventSystemInputProcessor);
        }
        EventSystemInputProcessor.Reset();
    }

    Super::OnDisable();
}

void UEventSystemComponent::OnDestroy()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    AHUD::OnShowDebugInfo.RemoveAll(this);
#endif
    
    Super::OnDestroy();
}

DECLARE_CYCLE_STAT(TEXT("UIEventSystem --- EventSystemTick"), STAT_UnrealGUI_EventSystemTick, STATGROUP_UnrealGUI);
void UEventSystemComponent::TickEventSystem(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_UnrealGUI_EventSystemTick);
    
    if (GetCurrentEventSystem(GetWorld()) != this)
        return;

    if (!FApp::CanEverRender())
    {
        DestroyComponent();
        return;
    }
    
    TickModules();

    bool bChangedModule = false;
    for (const auto& Module : SystemInputModules)
    {
        if (IsValid(Module) && Module->IsModuleSupported() && Module->ShouldActivateModule())
        {
            if (CurrentInputModule != Module)
            {
                ChangeEventModule(Module);
                bChangedModule = true;
            }
            break;
        }
    }

    // no event module set... set the first valid one...
    if (CurrentInputModule == nullptr)
    {
        for (const auto& Module : SystemInputModules)
        {
            if (IsValid(Module) && Module->IsModuleSupported())
            {
                ChangeEventModule(Module);
                bChangedModule = true;
                break;
            }
        }
    }

    if (!bChangedModule && IsValid(CurrentInputModule))
    {
        CurrentInputModule->Process();
    }
}

UBaseEventData* UEventSystemComponent::GetBaseEventDataCache()
{
    if (DummyData == nullptr)
    {
        DummyData = NewObject<UBaseEventData>(this);
        DummyData->SetEventSystem(this);
    }
    return DummyData;
}

void UEventSystemComponent::SetSelectedGameObject(USceneComponent* InSelectedObject, UBaseEventData* Pointer)
{
    if (bSelectionGuard)
    {
    	if (IsValid(InSelectedObject))
    	{
            UE_LOG(LogUGUI, Error, TEXT("Attempting to select %s while already selecting an object."), *InSelectedObject->GetFName().ToString());
    	}
        return;
    }

    bSelectionGuard = true;
    if (InSelectedObject == CurrentSelected)
    {
        bSelectionGuard = false;
        return;
    }

    FExecuteEvents::Execute<IDeselectHandlerInterface>(CurrentSelected, Pointer);
    CurrentSelected = InSelectedObject;
    FExecuteEvents::Execute<ISelectHandlerInterface>(CurrentSelected, Pointer);

    bSelectionGuard = false;
}

bool UEventSystemComponent::IsPointerOverGameObject() const
{
    return IsPointerOverGameObject(UPointerInputModuleSubComponent::KMouseLeftId);
}

bool UEventSystemComponent::IsPointerOverGameObject(int32 PointerId) const
{
    if (!IsValid(CurrentInputModule))
        return false;

    return CurrentInputModule->IsPointerOverGameObject(PointerId);
}

void UEventSystemComponent::UpdateModules()
{
    GetSubComponents(SystemInputModules);

    for (int32 Index = SystemInputModules.Num() - 1; Index >= 0; --Index)
    {
        if (IsValid(SystemInputModules[Index]) && SystemInputModules[Index]->IsActiveAndEnabled())
            continue;

        SystemInputModules.RemoveAt(Index, 1, false);
    }
}

void UEventSystemComponent::TickModules()
{
    for (const auto& Module : SystemInputModules)
    {
        if (IsValid(Module))
        {
            Module->UpdateModule();
        }
    }
}

void UEventSystemComponent::ChangeEventModule(UBaseInputModuleSubComponent* Module)
{
    if (CurrentInputModule == Module)
        return;

    if (IsValid(CurrentInputModule))
        CurrentInputModule->DeactivateModule();

    if (IsValid(Module))
        Module->ActivateModule();

    CurrentInputModule = Module;
}

void UEventSystemComponent::RaycastAll(const UPointerEventData* EventData, FRaycastResult& RaycastResult, const UWorld* InWorld)
{
    if (IsValid(InWorld))
    {
        const auto UIWorldSubsystem = InWorld->GetSubsystem<UUGUIWorldSubsystem>();
        if (IsValid(UIWorldSubsystem))
        {
            UIWorldSubsystem->RaycastAll(EventData, RaycastResult);
        }
    }
}

FString UEventSystemComponent::ToString() const
{
    FString FinalString = TEXT("");
    FinalString += FString::Printf(TEXT("Selected: %s\n"), IsValid(CurrentSelected) ? *CurrentSelected->GetFName().ToString() : TEXT("None"));
    FinalString += FString::Printf(TEXT("\n"));
    FinalString += FString::Printf(TEXT("\n"));
    FinalString += FString::Printf(TEXT("%s"), IsValid(CurrentInputModule) ? *CurrentInputModule->ToString() : TEXT("No Module"));
    return FinalString;
}

void UEventSystemComponent::ShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const
{
 #if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
    const bool bPrintDebugInfo = CVarShowEventSystemDebugInfo.GetValueOnGameThread() != 0;
    if (!bPrintDebugInfo)
    {
        return;
    }
    
    if (GetWorld() == nullptr || !GetWorld()->IsGameWorld())
        return;

    if (!IsEnabledInHierarchy())
        return;
    
	{
        const UFont* RenderFont = GEngine->GetMediumFont();
        Canvas->SetDrawColor(FColor::Yellow);
		YPos += 16;

        YPos += Canvas->DrawText(RenderFont, TEXT("Event System :"), 4, YPos);
        YPos += Canvas->DrawText(RenderFont, TEXT("\n"), 34, YPos);
	    YPos += Canvas->DrawText(RenderFont, FString::Printf(TEXT("Selected: %s\n"), IsValid(CurrentSelected) ? *CurrentSelected->GetFName().ToString() : TEXT("None")), 34, YPos);
        YPos += Canvas->DrawText(RenderFont, TEXT("\n"), 34, YPos);
        
        if (IsValid(CurrentInputModule))
        {
            CurrentInputModule->ShowDebugInfo(HUD, Canvas, DisplayInfo, YL, YPos);
        }
        else
        {
            YPos += Canvas->DrawText(RenderFont, TEXT("No Module"), 34, YPos);
        }
	}
#endif
}

/////////////////////////////////////////////////////
