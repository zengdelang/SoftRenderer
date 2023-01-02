#include "Core/Widgets/DropdownComponent.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "Core/Widgets/DropdownItemSubComponent.h"
#include "Core/Widgets/FocusSubComponent.h"
#include "Core/Layout/LayoutRebuilder.h"
#include "Animation/FloatTween.h"
#include "Core/WidgetActor.h"
#include "UGUI.h"
#include "Core/Layout/RectTransformUtility.h"

/////////////////////////////////////////////////////
// UDropDownComponent

FOptionData UDropdownComponent::NoOptionData;

UDropdownComponent::UDropdownComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), ScrollContent(nullptr), Dropdown(nullptr)
{
	CaptionTextPath.Emplace(0);

	DropdownPath.Emplace(2);
	
	ScrollContentPath.Emplace(2);
	ScrollContentPath.Emplace(0);
	ScrollContentPath.Emplace(0);

	ExtraSpace = 0;
	Value = -1;
	AlphaFadeSpeed = 0.15f;
	
	bValidTemplate = false;
	bHasStarted = false;
	bUpdateDropdownHeight = true;
	bFlipAxisX = false;
	bFlipAxisY = false;
	bIsExpanded = false;
}

void UDropdownComponent::Awake()
{
	Super::Awake();
	
	if (UBehaviourComponent* CaptionTextComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(CaptionTextPath)))
	{
		CaptionText = CaptionTextComp->GetComponentByInterface(UTextElementInterface::StaticClass(), true);
	}

	if (UBehaviourComponent* CaptionImageComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(CaptionImagePath)))
	{
		CaptionImage = CaptionImageComp->GetComponentByInterface(UImageElementInterface::StaticClass(), true);
	}

	if (UBehaviourComponent* PlaceholderComp = Cast<UBehaviourComponent>(FindChildBehaviourComponent(PlaceholderPath)))
	{
		Placeholder = PlaceholderComp->GetComponentByInterface(UGraphicElementInterface::StaticClass(), true);
	}

	ScrollContent = Cast<URectTransformComponent>(FindChildBehaviourComponent(ScrollContentPath));
	Dropdown = Cast<URectTransformComponent>(FindChildBehaviourComponent(DropdownPath));
	
	bHasStarted = false;
	bUpdateDropdownHeight = true;
	bFlipAxisX = false;
	bFlipAxisY = false;
	
	if (CaptionImage)
	{
		if (UBehaviourComponent* CaptionImageComp = Cast<UBehaviourComponent>(CaptionImage.GetObject()))
		{
			CaptionImageComp->SetEnabled(IsValid(CaptionImage->GetSprite()));
		}
	}

#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsGameWorld() && IsValid(Dropdown))
	{
		Dropdown->SetEnabled(false);
	}
#else
	if (IsValid(Dropdown))
	{
		Dropdown->SetEnabled(false);
	}
#endif
}

void UDropdownComponent::OnEnable()
{
	Super::OnEnable();

	if (!bHasStarted)
	{
		bHasStarted = true;

#if WITH_EDITOR
		if (GetWorld() && GetWorld()->IsGameWorld() && IsValid(Dropdown))
		{
			Dropdown->SetEnabled(false);
		}
#else
		if (IsValid(Dropdown))
		{
			Dropdown->SetEnabled(false);
		}
#endif
		
		RefreshShownValue();
	}
}

void UDropdownComponent::OnDestroy()
{
	if (AlphaTweenRunner.IsValid())
	{
		AlphaTweenRunner->StopTween();
		AlphaTweenRunner.Reset();
	}

	DestroyItemWidgets();
	
	Super::OnDestroy();
}

#if WITH_EDITORONLY_DATA

void UDropdownComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RefreshShownValue();
}

void UDropdownComponent::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	RefreshShownValue();
}

#endif

bool UDropdownComponent::IsExpanded() const
{
	return bIsExpanded;
}

void UDropdownComponent::RefreshShownValue()
{
	FOptionData& Data = NoOptionData;
	if (Options.Num() > 0 && Value >= 0)
	{
		Data = Options[FMath::Clamp(Value, 0, Options.Num() - 1)];
	}

	if (CaptionText)
	{
		CaptionText->SetText(Data.Text);
	}

	if (CaptionImage)
	{
		CaptionImage->SetSprite(Data.Image);

		if (UBehaviourComponent* CaptionImageComp = Cast<UBehaviourComponent>(CaptionImage.GetObject()))
		{
			CaptionImageComp->SetEnabled(IsValid(CaptionImage->GetSprite()));
		}
	}

	if (Placeholder)
	{
		if (UBehaviourComponent* PlaceholderComp = Cast<UBehaviourComponent>(Placeholder.GetObject()))
		{
			PlaceholderComp->SetEnabled(Options.Num() == 0 || Value == -1);
		}
	}
}

void UDropdownComponent::OnPointerClick(UPointerEventData* EventData)
{
	Show();
}

void UDropdownComponent::OnSubmit(UBaseEventData* EventData)
{
	Show();
}

void UDropdownComponent::OnCancel(UBaseEventData* EventData)
{
	Hide();
}

void UDropdownComponent::Show()
{
	if (AlphaTweenRunner.IsValid())
	{
		AlphaTweenRunner->StopTween();
		AlphaTweenRunner.Reset();
	}

	if (!IsActiveAndEnabled() || !IsInteractableInHierarchy() || !IsValid(Dropdown) || !IsValid(ScrollContent))
		return;

	// Get root Canvas.
	const UCanvasSubComponent* RootCanvas = GetRootCanvas();
	if (!IsValid(RootCanvas))
		return;

	const URectTransformComponent* RootCanvasRectTransform = Cast<URectTransformComponent>(RootCanvas->GetOuter());
	if (!IsValid(RootCanvasRectTransform))
		return;
	
	if (!bValidTemplate)
	{
		SetupTemplate();
		if (!bValidTemplate)
			return;
	}

	Dropdown->SetEnabled(true);
	Dropdown->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	
	for (int32 Index = 0, Count = Items.Num(); Index < Count; ++Index)
	{
		const auto ItemComp = Items[Index];
		if (IsValid(ItemComp))
		{
			ItemComp->SetEnabled(false);
		}
		else
		{
			Items.RemoveAt(Index);
		}
	}

	if (Items.Num() > Options.Num())
	{
		for (int32 Index = Items.Num() - 1 ; Index >= Options.Num() ; --Index)
		{
			URectTransformComponent* ItemRootComp = Cast<URectTransformComponent>(Items[Index]->GetOuter());
			if (IsValid(ItemRootComp))
			{
				AActor* ItemWidget = nullptr;
				ItemWidget = ItemRootComp->GetOwner();
				if (IsValid(ItemWidget) && !GExitPurge)
				{
					// if still alive, destroy, otherwise just clear the pointer
					const bool bIsChildActorPendingKillOrUnreachable = ItemWidget->IsPendingKillOrUnreachable();

					UWorld* World = ItemWidget->GetWorld();

					// World may be nullptr during shutdown
					if (World != nullptr)
					{
						if (!bIsChildActorPendingKillOrUnreachable)
						{
							World->DestroyActor(ItemWidget);
						}
					}
				}
				Items.RemoveAt(Index);
			}
		}
	}

	FVector2D ItemSize = FVector2D::ZeroVector;
	bool bUpdateItemSize = true;
	for (int32 Index = 0, Count = Options.Num(); Index < Count; ++Index)
	{
		const FOptionData& Data = Options[Index];

		UDropdownItemSubComponent* ItemComp = nullptr;
		if (Items.IsValidIndex(Index))
		{
			ItemComp = Items[Index];
		}
		
		if (!IsValid(ItemComp))
		{
			const AWidgetActor* ItemWidget = CreateItemWidget();
			if (IsValid(ItemWidget) && ItemWidget->GetRootRectTransformComponent())
			{
				UDropdownItemSubComponent* DropdownItemComp = Cast<UDropdownItemSubComponent>(ItemWidget->GetRootRectTransformComponent()->GetComponent(UDropdownItemSubComponent::StaticClass(), true));
				if (IsValid(DropdownItemComp))
				{
					if (Items.IsValidIndex(Index))
					{
						Items[Index] = DropdownItemComp;
					}
					else
					{
						DropdownItemComp ->SetDropdown(this);
						Items.Add(DropdownItemComp);
					}

					ItemComp = DropdownItemComp;
				}
			}
		}
		
		if (!IsValid(ItemComp))
			continue;
		
		ItemComp->SetData(Index, GetValue() == Index, Data);

		URectTransformComponent* ItemRootComp = Cast<URectTransformComponent>(ItemComp->GetOuter());
		if (IsValid(ItemRootComp))
		{
			ItemRootComp->SetEnabled(true);
		}
		
		if (bUpdateItemSize)
		{
			bUpdateItemSize = false;
			
			if (IsValid(ItemRootComp))
			{
				FLayoutRebuilder::ForceRebuildLayoutImmediate(ItemRootComp);
			}
			
			const auto LayoutElement = Cast<ILayoutElementInterface>(ItemRootComp);
			if (LayoutElement)
			{
				ItemSize.Y = FMath::Max(0.f, LayoutElement->GetPreferredHeight());
			}
			else
			{
				const auto RectTransformComp = Cast<URectTransformComponent>(ItemRootComp);
				if (RectTransformComp)
				{
					ItemSize = FMath::Max(FVector2D::ZeroVector, RectTransformComp->GetRect().GetSize());
				}
			}
		}
		
		// Automatically set up explicit navigation   TODO
		// if (prev != null)
		// {
		// 	Navigation prevNav = prev.navigation;
		// 	Navigation toggleNav = item.toggle.navigation;
		// 	prevNav.mode = Navigation.Mode.Explicit;
		// 	toggleNav.mode = Navigation.Mode.Explicit;
		//
		// 	prevNav.selectOnDown = item.toggle;
		// 	prevNav.selectOnRight = item.toggle;
		// 	toggleNav.selectOnLeft = prev;
		// 	toggleNav.selectOnUp = prev;
		//
		// 	prev.navigation = prevNav;
		// 	item.toggle.navigation = toggleNav;
		// }
		// prev = item.toggle;
	}

	FVector2D ContentSizeDelta = ScrollContent->GetSizeDelta();
	ContentSizeDelta.Y = ItemSize.Y * Items.Num() + ExtraSpace;
	ScrollContent->SetSizeDelta(ContentSizeDelta);
	
	if (bUpdateDropdownHeight)
	{
		bUpdateDropdownHeight = false;
		DropdownSize.X = Dropdown->GetSizeDelta().X;
		DropdownSize.Y = Dropdown->GetRect().Height;
	}

	const float HeightExtraSpace = DropdownSize.Y - ScrollContent->GetRect().Height;
	if (HeightExtraSpace > 0)
	{
		Dropdown->SetSizeDelta(FVector2D(DropdownSize.X, DropdownSize.Y - HeightExtraSpace));
	}

	if (bFlipAxisX)
	{
		bFlipAxisX = false;
		FRectTransformUtility::FlipLayoutOnAxis(Dropdown, 0, false, false);
	}

	if (bFlipAxisY)
	{
		bFlipAxisY = false;
		FRectTransformUtility::FlipLayoutOnAxis(Dropdown, 1, false, false);
	}
	
	// Invert anchoring and position if dropdown is partially or fully outside of canvas rect.
	// Typically this will have the effect of placing the dropdown above the button instead of below,
	// but it works as inversion regardless of initial setup.
	FVector Corners[4];
	Dropdown->GetWorldCorners(Corners);

	const FRect RootCanvasRect = RootCanvasRectTransform->GetRect();
	const auto WorldToRootCanvasTransform = RootCanvasRectTransform->GetComponentTransform().Inverse();
	for (int32 Axis = 0; Axis < 2; ++Axis)
	{
		bool bOutside = false;
		for (int32 Index = 0; Index < 4; Index++)
		{
			FVector Corner = WorldToRootCanvasTransform.TransformPosition(Corners[Index]);
			if ((Corner[Axis] < RootCanvasRect.GetMin()[Axis] && !FMathUtility::Approximately(Corner[Axis], RootCanvasRect.GetMin()[Axis])) ||
				(Corner[Axis] > RootCanvasRect.GetMax()[Axis] && !FMathUtility::Approximately(Corner[Axis], RootCanvasRect.GetMax()[Axis])))
			{
				bOutside = true;
				break;
			}
		}
		
		if (bOutside)
		{
			if (Axis == 0)
			{
				bFlipAxisX = true;
			}
			else
			{
				bFlipAxisY = true;	
			}
			
			FRectTransformUtility::FlipLayoutOnAxis(Dropdown, Axis, false, false);
		}
	}
	
	for (int32 Index = 0, Count = Items.Num(); Index < Count; ++Index)
	{
		const UDropdownItemSubComponent* ItemComp = Items[Index];
		if (IsValid(ItemComp))
		{
			URectTransformComponent* ItemRootComp = Cast<URectTransformComponent>(ItemComp->GetOuter());
			if (IsValid(ItemRootComp) && ItemRootComp->IsEnabled())
			{
				ItemRootComp->SetLocalRotation(FRotator::ZeroRotator);
				ItemRootComp->SetLocalScale(FVector::OneVector);

				ItemRootComp->SetInsetAndSizeFromParentEdge(ERectTransformEdge::RectTransformEdge_Left, 0, ItemRootComp->GetSizeDelta()[0]);
				ItemRootComp->SetInsetAndSizeFromParentEdge(ERectTransformEdge::RectTransformEdge_Top, Index * ItemSize.Y + 0.5 * ExtraSpace, ItemRootComp->GetSizeDelta()[1]);
			}
		}
	}

	// Fade in the popup
	Dropdown->SetRenderOpacity(0);
	AlphaFadeList(AlphaFadeSpeed, 0.0f, 1.0f, true);

	UFocusSubComponent* FocusComp = Cast<UFocusSubComponent>(Dropdown->GetComponent(UFocusSubComponent::StaticClass(), true));
	if (!IsValid(FocusComp))
	{
		FocusComp = Cast<UFocusSubComponent>(Dropdown->AddSubComponentByClass(UFocusSubComponent::StaticClass()));
	}

	FocusComp->OnLostFocus.RemoveAll(this);
	FocusComp->OnLostFocus.AddUniqueDynamic(this, &UDropdownComponent::OnLostFocus);

	bIsExpanded = true;
}

void UDropdownComponent::Hide()
{
	bIsExpanded = false;
	
	if (AlphaTweenRunner.IsValid())
	{
		AlphaTweenRunner->StopTween();
		AlphaTweenRunner.Reset();
	}
	
	UFocusSubComponent* FocusComp = Cast<UFocusSubComponent>(Dropdown->GetComponent(UFocusSubComponent::StaticClass(), true));
	if (IsValid(FocusComp))
	{
		FocusComp->OnLostFocus.RemoveAll(this);
	}
	
	if (IsValid(Dropdown))
	{
		AlphaFadeList(AlphaFadeSpeed, GetRenderOpacity(), 0.0f, false);
	}
	
	Select();
}

UCanvasSubComponent* UDropdownComponent::GetRootCanvas() const
{
	if (IsValid(OwnerCanvas))
	{
		return OwnerCanvas->GetRootCanvas();
	}
	return nullptr;
}

void UDropdownComponent::SetupTemplate()
{
	bValidTemplate = false;

	const UClass* ItemWidgetClass = DropdownItemWidgetTemplateClass;
	if (ItemWidgetClass == nullptr)
	{
		UE_LOG(LogUGUI, Error, TEXT("UDropdownComponent---SetupTemplate, The dropdown template is not assigned. The template needs to be assigned."));
		return;
	}
	
	bValidTemplate = true;

	DestroyItemWidgets();

	UDropdownItemSubComponent* DropdownItemComp = nullptr;

	AWidgetActor* ItemWidget = CreateItemWidget();
	if (IsValid(ItemWidget))
	{
		if (const auto ItemRootComp = ItemWidget->GetRootRectTransformComponent())
		{
			DropdownItemComp = Cast<UDropdownItemSubComponent>(ItemRootComp->GetComponent(UDropdownItemSubComponent::StaticClass(), true));
			ItemRootComp->SetEnabled(false);	
		}
	}

	if (!IsValid(DropdownItemComp))
	{
		if (IsValid(ItemWidget))
		{
			bool bIsLevelBeingRemoved = false;
			if (const AActor* MyOwner = GetOwner())
			{
				if (const ULevel* MyLevel = MyOwner->GetLevel())
				{
					bIsLevelBeingRemoved = MyLevel->bIsBeingRemoved;
				}
			}

			if (!bIsLevelBeingRemoved && !GExitPurge)
			{
				// if still alive, destroy, otherwise just clear the pointer
				const bool bIsChildActorPendingKillOrUnreachable = ItemWidget->IsPendingKillOrUnreachable();

				UWorld* World = ItemWidget->GetWorld();

				// World may be nullptr during shutdown
				if (World != nullptr)
				{
					if (!bIsChildActorPendingKillOrUnreachable)
					{
						World->DestroyActor(ItemWidget);
					}
				}
			}
		}
		
		bValidTemplate = false;
		UE_LOG(LogUGUI, Error, TEXT("UDropdownComponent---SetupTemplate, The item template is not valid. The template must have a child component with a DropdownItemSubComponent serving as the item."));
		return;
	}

	DropdownItemComp ->SetDropdown(this);
	Items.Add(DropdownItemComp);

	UCanvasSubComponent* CanvasComp = Cast<UCanvasSubComponent>(Dropdown->GetComponent(UCanvasSubComponent::StaticClass(), true));
	if (!IsValid(CanvasComp))
	{
		CanvasComp = Cast<UCanvasSubComponent>(Dropdown->AddSubComponentByClass(UCanvasSubComponent::StaticClass()));
	}

	if (!CanvasComp->IsOverrideSorting())
	{
		CanvasComp->SetOverrideSorting(true);
	}

	if (CanvasComp->GetSortingOrder() < 30000)
	{
		CanvasComp->SetSortingOrder(30000);
	}
	
	bValidTemplate = true;
}

void UDropdownComponent::OnLostFocus()
{
	if (IsValid(Dropdown))
	{
		Dropdown->SetEnabled(true);
		Hide();
	}
}

AWidgetActor* UDropdownComponent::CreateItemWidget() const
{
	UClass* ItemWidgetClass = DropdownItemWidgetTemplateClass;
	if (ItemWidgetClass == nullptr)
	{
		return nullptr;
	}
	
    const auto World = GetWorld();
	if (IsValid(World))
	{
	    FActorSpawnParameters Params;
	    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	    Params.bDeferConstruction = true; // We defer construction so that we set ParentComponent prior to component registration so they appear selected
	    Params.bAllowDuringConstructionScript = true;
	    Params.ObjectFlags |= RF_TextExportTransient | RF_DuplicateTransient | RF_Transient;

        const auto ItemWidgetActor = World->SpawnActor<AWidgetActor>(ItemWidgetClass, Params);

	    // If spawn was successful,
	    if (ItemWidgetActor != nullptr)
	    {
            ItemWidgetActor->bDisableCanvasUpdateRectTransform = true;
            ItemWidgetActor->bStripCanvasComponent = true;

#if WITH_EDITOR
	    	ItemWidgetActor->bRecordForUndo = false;
#endif

	    	ItemWidgetActor->AttachSceneComponent = ScrollContent;
	    	
#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
			ItemWidgetActor->FinishSpawning(FTransform::Identity, false, nullptr, true);
#else
			ItemWidgetActor->FinishSpawning(GetComponentTransform(), false);
#endif

#if WITH_EDITORONLY_DATA
	    	if (ItemWidgetActor->GetRootRectTransformComponent())
	        {
                ItemWidgetActor->AttachParentComponent = ScrollContent;
	        }
#endif
	    	
	    	return ItemWidgetActor;
	    }
	}

    return nullptr;
}

void UDropdownComponent::DestroyItemWidgets()
{
	bool bIsLevelBeingRemoved = false;
	if (const AActor* MyOwner = GetOwner())
	{
		if (const ULevel* MyLevel = MyOwner->GetLevel())
		{
			bIsLevelBeingRemoved = MyLevel->bIsBeingRemoved;
		}
	}

	for (const auto& ItemComp : Items)
	{
		if (IsValid(ItemComp) && !bIsLevelBeingRemoved)
		{
			AActor* ItemWidget = nullptr;
			
			const URectTransformComponent* ItemRootComp = Cast<URectTransformComponent>(ItemComp->GetOuter());
			if (IsValid(ItemRootComp))
			{
				ItemWidget = ItemRootComp->GetOwner();
			}
			
			if (IsValid(ItemWidget) && !GExitPurge)
			{
				// if still alive, destroy, otherwise just clear the pointer
				const bool bIsChildActorPendingKillOrUnreachable = ItemWidget->IsPendingKillOrUnreachable();

				UWorld* World = ItemWidget->GetWorld();

				// World may be nullptr during shutdown
				if (World != nullptr)
				{
					if (!bIsChildActorPendingKillOrUnreachable)
					{
						World->DestroyActor(ItemWidget);
					}
				}
			}
		}
	}

	Items.Empty();
}

void UDropdownComponent::AlphaFadeList(float Duration, float Start, float End, bool bDropdownEnabled)
{
	if (End == Start)
		return;

	if (!AlphaTweenRunner.IsValid())
	{
		AlphaTweenRunner = MakeShareable(new FTweenRunner());
		AlphaTweenRunner->Init(MakeShareable(new FFloatTween()));
	}

	FFloatTween* AlphaTween = static_cast<FFloatTween*>(AlphaTweenRunner->GetTweenValue());
	AlphaTween->SetDuration(Duration);
	AlphaTween->StartValue = Start;
	AlphaTween->TargetValue = End;
	AlphaTween->bIgnoreTimeScale = true;

	AlphaTween->OnFloatTweenCallback.Clear();
	AlphaTween->OnFloatTweenCallback.AddUObject(this, &UDropdownComponent::SetAlpha);
	
	AlphaTweenRunner->OnTweenRunnerFinished.RemoveAll(this);
	AlphaTweenRunner->OnTweenRunnerFinished.AddWeakLambda(this, [this, bDropdownEnabled]
	{
		if (IsValid(Dropdown))
		{
			Dropdown->SetEnabled(bDropdownEnabled);
		}
	});
	AlphaTweenRunner->StartTween(GetWorld(), IsActiveAndEnabled());
}

void UDropdownComponent::SetAlpha(float Alpha) const
{
	if (!IsValid(Dropdown))
		return;

	Dropdown->SetRenderOpacity(Alpha);
}

void UDropdownComponent::OnSelectItem(int32 Index)
{
	if (Index < 0)
		return;

	SetValue(Index);
	Hide();
}

/////////////////////////////////////////////////////
