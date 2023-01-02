#include "DetailCustomizations/SubComponentsDetailRow/SSubComponentOverlay.h"

#include "UIBlueprintEditorModule.h"
#include "Core/BehaviourComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

void SSubComponentOverlay::Construct( const FArguments& InArgs, const TSharedPtr< class IPropertyHandle > InPropertyHandle, 
	const TSharedPtr< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, int32 InIndex, const TAttribute<bool>& InIsParentEnabled)
{
	SetCanTick(false);

	OwnerObject = InOwnerObject;
	PropertyHandle = InPropertyHandle;
	SubComponentsPropertyHandle = InSubComponentsPropertyHandle;
	Index = InIndex;
	IsParentEnabled = InIsParentEnabled;
}

FReply SSubComponentOverlay::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FMenuBuilder MenuBuilder(true, nullptr, nullptr, true);

		bool bShouldOpenMenu = true;
		
		const FUIAction RemoveComponentAction(FExecuteAction::CreateSP(this, &SSubComponentOverlay::OnRemoveComponentClicked),
			FCanExecuteAction::CreateSP(this, &SSubComponentOverlay::CanRemoveComponent));

		const FUIAction MoveUpAction(FExecuteAction::CreateSP(this, &SSubComponentOverlay::OnMoveUpClicked),
			FCanExecuteAction::CreateSP(this, &SSubComponentOverlay::CanMoveUp));

		const FUIAction MoveDownAction(FExecuteAction::CreateSP(this, &SSubComponentOverlay::OnMoveDownClicked),
			FCanExecuteAction::CreateSP(this, &SSubComponentOverlay::CanMoveDown));
		
		MenuBuilder.BeginSection(NAME_None, NSLOCTEXT("SubComponentOverlay", "SubComponentHeading", ""));
		MenuBuilder.AddMenuEntry(NSLOCTEXT("SubComponentOverlay", "RemoveComponent", "Remove Component"), NSLOCTEXT("SubComponentOverlay", "RemoveComponent_ToolTip", "Remove Component"), FSlateIcon(), RemoveComponentAction);
		MenuBuilder.AddMenuEntry(NSLOCTEXT("SubComponentOverlay", "MoveUp", "Move Up"), NSLOCTEXT("SubComponentOverlay", "MoveUp_ToolTip", "Move Up"), FSlateIcon(), MoveUpAction);
		MenuBuilder.AddMenuEntry(NSLOCTEXT("SubComponentOverlay", "MoveDown", "Move Down"), NSLOCTEXT("SubComponentOverlay", "MoveDown_ToolTip", "Move Down"), FSlateIcon(), MoveDownAction);
		MenuBuilder.EndSection();

		if (bShouldOpenMenu)
		{
			FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
			FSlateApplication::Get().PushMenu(AsShared(), WidgetPath, MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
			return FReply::Handled();
		}
	}

	return FReply::Handled();
}

void SSubComponentOverlay::GetRequireSubClasses(UClass* InClass, TArray<UClass*>& RequireSubClasses) const
{
	if (!InClass)
		return;

	const FString& RequireClassesString = InClass->GetMetaData("RequireSubClasses");
	TArray<FString> RequireClassNames;
	RequireClassesString.ParseIntoArrayWS(RequireClassNames, TEXT(","), true);

	for (const FString& RequireClassName : RequireClassNames)
	{
		UClass* RequireClass = FindObject<UClass>(ANY_PACKAGE, *RequireClassName);
		if (RequireClass && RequireClass->IsChildOf(UBehaviourSubComponent::StaticClass()))
		{
			RequireSubClasses.AddUnique(RequireClass);
		}
	}
}

void SSubComponentOverlay::OnRemoveComponentClicked() const
{
	if (PropertyHandle.IsValid())
	{
		if (OwnerObject.IsValid())
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(OwnerObject.Get());
			if (BehaviourComp)
			{
				IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(BehaviourComp, NSLOCTEXT("SSubComponentOverlay", "RemoveSubComponent", "Remove SubComponent(s)"));
				
				const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
				if (AllSubComponents.IsValidIndex(Index))
				{
					if (AllSubComponents[Index])
					{
						TArray<UClass*> RequireSubClasses;
						int32 SomeSubCompNum = 0;
						for (const auto& SubComp : AllSubComponents)
						{
							if (SubComp && SubComp->GetClass() == AllSubComponents[Index]->GetClass())
							{
								++SomeSubCompNum;
							}
						}

						TArray<FText> DependClasses;
						for (const auto& SubComp : AllSubComponents)
						{
							if (SubComp && SubComp->GetClass() != AllSubComponents[Index]->GetClass())
							{
								GetRequireSubClasses(SubComp->GetClass(), RequireSubClasses);

								if (RequireSubClasses.Contains(AllSubComponents[Index]->GetClass()))
								{
									DependClasses.Add(SubComp->GetClass()->GetDisplayNameText());
								}
							}
						}

						if (SomeSubCompNum == 1 && DependClasses.Num() > 0)
						{
							FString DependString = DependClasses[0].ToString() + TEXT("(Script)");
							for (int32 DependIndex = 1, Count = DependClasses.Num(); DependIndex < Count; ++DependIndex)
							{
								DependString += TEXT(",") + DependClasses[DependIndex].ToString() + TEXT("(Script)");
							}
							
							const FText DialogText = FText::Format(
								LOCTEXT("RemoveSubComponentConfirm", "Can't remove '{0}' because {1} depends on it"),
								AllSubComponents[Index]->GetClass()->GetDisplayNameText(),
								FText::FromString(DependString)
							);

							FMessageDialog::Open(EAppMsgType::Ok, DialogText);
							return;
						}

						if (BehaviourComp->HasAnyFlags(RF_ArchetypeObject))
						{
							const auto ArrayHandle = SubComponentsPropertyHandle->AsArray();
							if (ArrayHandle.IsValid())
							{
								ArrayHandle->DeleteItem(Index);
							}
						}
						else
						{
							BehaviourComp->RemoveSubComponentForEditor(Index);
						}

						bool bValue = false;
						PropertyHandle->GetValue(bValue);
						PropertyHandle->SetValue(!bValue);
					}
				}

				IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(BehaviourComp);
			}
		}
	}
}

bool SSubComponentOverlay::CanRemoveComponent() const
{
	return IsParentEnabled.Get();
}

void SSubComponentOverlay::OnMoveUpClicked() const
{
	if (PropertyHandle.IsValid())
	{
		if (OwnerObject.IsValid())
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(OwnerObject.Get());
			if (BehaviourComp)
			{
				BehaviourComp->MoveSubComponentForEditor(Index, Index - 1);
			}
		}

		bool bValue = false;
		PropertyHandle->GetValue(bValue);
		PropertyHandle->SetValue(!bValue);
	}
}

bool SSubComponentOverlay::CanMoveUp() const
{
	return IsParentEnabled.Get() && Index > 0;
}

void SSubComponentOverlay::OnMoveDownClicked() const
{
	if (PropertyHandle.IsValid())
	{
		if (OwnerObject.IsValid())
		{
			const auto BehaviourComp = Cast<UBehaviourComponent>(OwnerObject.Get());
			if (BehaviourComp)
			{
				BehaviourComp->MoveSubComponentForEditor(Index, Index + 1);
			}
		}

		bool bValue = false;
		PropertyHandle->GetValue(bValue);
		PropertyHandle->SetValue(!bValue);
	}
}

bool SSubComponentOverlay::CanMoveDown() const
{
	if (OwnerObject.IsValid())
	{
		const auto BehaviourComp = Cast<UBehaviourComponent>(OwnerObject.Get());
		if (BehaviourComp)
		{
			return IsParentEnabled.Get() && Index < (BehaviourComp->GetAllSubComponents().Num() - 1);
		}
	}
	
	return false;
}

#undef LOCTEXT_NAMESPACE
