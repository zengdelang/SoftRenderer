#include "Core/Widgets/ToggleGroupComponent.h"
#include "Core/Widgets/ToggleComponent.h"
#include "UGUI.h"

/////////////////////////////////////////////////////
// UToggleGroupComponent

UToggleGroupComponent::UToggleGroupComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAllowSwitchOff = false;
}

void UToggleGroupComponent::NotifyToggleOn(UToggleComponent* Toggle, bool bSendCallback)
{
    if (!ValidateToggleIsInGroup(Toggle))
    {
        return;
    }
	
    // disable all toggles in the group
    for (int32 Index = 0, Count = Toggles.Num(); Index < Count; ++Index)
    {
        const auto ToggleObj = Toggles[Index];
        if (!IsValid(ToggleObj) || ToggleObj == Toggle)
        {
            continue;
        }

        if (bSendCallback)
        {
            ToggleObj->SetIsOn(false);
        }
        else
        {
            ToggleObj->SetIsOnWithoutNotify(false);
        }
    }
}

void UToggleGroupComponent::UnregisterToggle(UToggleComponent* Toggle)
{
    if (Toggles.Contains(Toggle))
    {
        Toggles.Remove(Toggle);
    }

    if (!IsAllowSwitchOff() && !AnyTogglesOn() && Toggles.Num() != 0)
    {
        if (IsValid(Toggles[0]))
        {
            Toggles[0]->SetIsOn(true);
            NotifyToggleOn(Toggles[0]);
        }
    }
}

void UToggleGroupComponent::RegisterToggle(UToggleComponent* Toggle)
{
    if (!Toggles.Contains(Toggle))
    {
        Toggles.Add(Toggle);
    }

    if (!IsAllowSwitchOff() && !AnyTogglesOn() && IsValid(Toggle))
    {
        Toggle->SetIsOn(true);
        NotifyToggleOn(Toggle);
    }
}

bool UToggleGroupComponent::AnyTogglesOn()
{
    for (int32 Index = 0, Count = Toggles.Num(); Index < Count; ++Index)
    {
        const auto ToggleObj = Toggles[Index];
        if (!IsValid(ToggleObj))
        {
            continue;
        }

        if (ToggleObj->IsOn())
        {
            return true;
        }
    }
    
	return false;
}

void UToggleGroupComponent::GetActiveToggles(TArray<UToggleComponent*>& OutToggles)
{
    OutToggles.Empty();
    OutToggles.Reserve(Toggles.Num());
	
    for (int32 Index = 0, Count = Toggles.Num(); Index < Count; ++Index)
    {
        const auto ToggleObj = Toggles[Index];
        if (!IsValid(ToggleObj))
        {
            continue;
        }

        if (ToggleObj->IsOn())
        {
            OutToggles.Add(ToggleObj);
        }
    }
}

void UToggleGroupComponent::SetAllTogglesOff(bool bSendCallback)
{
    const bool bOldAllowSwitchOff = bAllowSwitchOff;
    bAllowSwitchOff = true;

    if (bSendCallback)
    {
        for (int32 Index = 0, Count = Toggles.Num(); Index < Count; ++Index)
        {
            const auto ToggleObj = Toggles[Index];
            if (!IsValid(ToggleObj))
            {
                continue;
            }

            ToggleObj->SetIsOn(false);
        }
    }
    else
    {
        for (int32 Index = 0, Count = Toggles.Num(); Index < Count; ++Index)
        {
            const auto ToggleObj = Toggles[Index];
            if (!IsValid(ToggleObj))
            {
                continue;
            }

            ToggleObj->SetIsOnWithoutNotify(false);
        }
    }

    bAllowSwitchOff = bOldAllowSwitchOff;
}

bool UToggleGroupComponent::ValidateToggleIsInGroup(UToggleComponent* Toggle) const
{
    if (!IsValid(Toggle) || !Toggles.Contains(Toggle))
    {
    	if (IsValid(Toggle))
    	{
            UE_LOG(LogUGUI, Error, TEXT("UToggleGroupComponent --- Toggle is not part of ToggleGroup %s"), *GetFName().ToString());
    	}
        else
        {
            UE_LOG(LogUGUI, Error, TEXT("UToggleGroupComponent --- Toggle %s is not part of ToggleGroup %s"), *Toggle->GetFName().ToString(), *GetFName().ToString());
        }
        return false;
    }
    
    return true;
}

/////////////////////////////////////////////////////
