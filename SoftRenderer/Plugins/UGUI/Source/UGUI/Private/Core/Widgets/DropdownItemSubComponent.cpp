#include "Core/Widgets/DropdownItemSubComponent.h"
#include "Core/Widgets/DropdownComponent.h"
#include "Core/Widgets/ToggleComponent.h"

/////////////////////////////////////////////////////
// UDropdownItemSubComponent

UDropdownItemSubComponent::UDropdownItemSubComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), Toggle(nullptr), Index(0), Dropdown(nullptr)
{
	TogglePath.Emplace(0);
	
	ItemTextPath.Emplace(0);
	ItemTextPath.Emplace(2);
}

void UDropdownItemSubComponent::OnPointerEnter(UPointerEventData* EventData)
{
	const auto World = GetWorld();
	const auto EventSystem = UEventSystemComponent::GetCurrentEventSystem(World);
	if (IsValid(EventSystem))
	{
		EventSystem->SetSelectedGameObject(Cast<USceneComponent>(GetOuter()));
	}
}

void UDropdownItemSubComponent::OnCancel(UBaseEventData* EventData)
{
	if (IsValid(Dropdown))
	{
		Dropdown->Hide();
	}
}

void UDropdownItemSubComponent::OnToggleValueChanged(bool bIsOn)
{
	if (IsValid(Dropdown))
	{
		Dropdown->OnSelectItem(Index);
	}
}

void UDropdownItemSubComponent::SetDropdown(UDropdownComponent* InDropdown)
{
	Dropdown = InDropdown;

	if (!IsValid(Dropdown))
	{
		return;
	}
	
	if (UBehaviourComponent* ToggleComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(TogglePath)))
	{
		Toggle = Cast<UToggleComponent>(ToggleComp->GetComponent(UToggleComponent::StaticClass(), true));
	}

	if (IsValid(Toggle))
	{
		Toggle->OnValueChanged.AddUniqueDynamic(this, &UDropdownItemSubComponent::OnToggleValueChanged);
	}
	
	if (UBehaviourComponent* ItemTextComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(ItemTextPath)))
	{
		ItemText = ItemTextComp->GetComponentByInterface(UTextElementInterface::StaticClass(), true);
	}

	if (UBehaviourComponent* ItemImageComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(ItemImagePath)))
	{
		ItemImage = ItemImageComp->GetComponentByInterface(UImageElementInterface::StaticClass(), true);
	}
}

void UDropdownItemSubComponent::SetData(int32 InIndex, bool bIsOn, const FOptionData& Data)
{
	Index = InIndex;
	
	if (ItemText)
	{
		ItemText->SetText(Data.Text);
	}

	if (ItemImage)
	{
		ItemImage->SetSprite(Data.Image);

		if (UBehaviourComponent* CaptionImageComp = Cast<UBehaviourComponent>(ItemImage.GetObject()))
		{
			CaptionImageComp->SetEnabled(IsValid(ItemImage->GetSprite()));
		}
	}

	if (IsValid(Toggle))
	{
		Toggle->SetIsOnWithoutNotify(bIsOn);

		if (bIsOn)
		{
			Toggle->Select();
		}
	}
}

USceneComponent* UDropdownItemSubComponent::FindChildBehaviourComponent(const TArray<int32>& ChildPath) const
{
	const USceneComponent* CurComp = Cast<USceneComponent>(GetOuter());
	if (!IsValid(CurComp))
		return nullptr;
	
	USceneComponent* TargetComp = nullptr;
	
	for (const auto& ChildIndex : ChildPath)
	{
		auto ChildAttachChildren = CurComp->GetAttachChildren();

		if (ChildAttachChildren.IsValidIndex(ChildIndex))
		{
			if (IsValid(ChildAttachChildren[ChildIndex]))
			{
				TargetComp = ChildAttachChildren[ChildIndex];
				CurComp = TargetComp;
			}
			else
			{
				break;
			}
		}
	}
	
	return TargetComp;
}

/////////////////////////////////////////////////////

