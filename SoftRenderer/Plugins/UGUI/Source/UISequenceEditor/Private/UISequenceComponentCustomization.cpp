#include "UISequenceComponentCustomization.h"
#include "UISequence.h"
#include "UISequenceComponent.h"
#include "EditorStyleSet.h"
#include "GameFramework/Actor.h"
#include "IDetailsView.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Docking/SDockTab.h"
#include "SSCSEditor.h"
#include "BlueprintEditorTabs.h"
#include "ScopedTransaction.h"
#include "ISequencerModule.h"
#include "Editor.h"
#include "UISequenceEditorTabSummoner.h"
#include "IPropertyUtilities.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "UISequenceComponentCustomization"

FName SequenceTabId("EmbeddedUISequenceID");

class SUISequenceEditorWidgetWrapper : public SUISequenceEditorWidget
{
public:
	~SUISequenceEditorWidgetWrapper()
	{
		GEditor->OnObjectsReplaced().Remove(OnObjectsReplacedHandle);
	}

	void Construct(const FArguments& InArgs, TWeakObjectPtr<UUISequenceComponent> InSequenceComponent)
	{
		SUISequenceEditorWidget::Construct(InArgs, nullptr);

		WeakSequenceComponent = InSequenceComponent;
		AssignSequence(GetUISequence());

		OnObjectsReplacedHandle = GEditor->OnObjectsReplaced().AddSP(this, &SUISequenceEditorWidgetWrapper::OnObjectsReplaced);
	}

protected:
	UUISequence* GetUISequence() const
	{
		UUISequenceComponent* SequenceComponent = WeakSequenceComponent.Get();
		return SequenceComponent ? SequenceComponent->Sequence : nullptr;
	}

	void OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
	{
		UUISequenceComponent* Component = WeakSequenceComponent.Get(true);

		UUISequenceComponent* NewSequenceComponent = Component ? Cast<UUISequenceComponent>(ReplacementMap.FindRef(Component)) : nullptr;
		if (NewSequenceComponent)
		{
			WeakSequenceComponent = NewSequenceComponent;
			AssignSequence(GetUISequence());
		}
	}

private:
	TWeakObjectPtr<UUISequenceComponent> WeakSequenceComponent;
	FDelegateHandle OnObjectsReplacedHandle;
	
};

TSharedRef<IDetailCustomization> FUISequenceComponentCustomization::MakeInstance()
{
	return MakeShared<FUISequenceComponentCustomization>();
}

void FUISequenceComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	PropertyUtilities = DetailBuilder.GetPropertyUtilities();

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() != 1)
	{
		return;
	}

	WeakSequenceComponent = Cast<UUISequenceComponent>(Objects[0].Get());
	if (!WeakSequenceComponent.Get())
	{
		return;
	}

	const IDetailsView* DetailsView = DetailBuilder.GetDetailsView();
	TSharedPtr<FTabManager> HostTabManager = DetailsView->GetHostTabManager();

	DetailBuilder.HideProperty("Sequence");
	
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Sequence", FText(), ECategoryPriority::Important);
	
	bool bIsExternalTabAlreadyOpened = false;

	if (HostTabManager.IsValid() && HostTabManager->HasTabSpawner(SequenceTabId))
	{
		WeakTabManager = HostTabManager;

		TSharedPtr<SDockTab> ExistingTab = HostTabManager->FindExistingLiveTab(SequenceTabId);
		if (ExistingTab.IsValid())
		{
			UUISequence* ThisSequence = GetUISequence();

			auto SequencerWidget = StaticCastSharedRef<SUISequenceEditorWidget>(ExistingTab->GetContent());
			bIsExternalTabAlreadyOpened = ThisSequence && SequencerWidget->GetSequence() == ThisSequence;
		}

		Category.AddCustomRow(FText())
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SequenceValueText", "Sequence"))
				.Font(DetailBuilder.GetDetailFont())
			]
			.ValueContent()
			[
				SNew(SButton)
				.OnClicked(this, &FUISequenceComponentCustomization::InvokeSequencer)
				[
					SNew(STextBlock)
					.Text(bIsExternalTabAlreadyOpened ? LOCTEXT("FocusSequenceTabButtonText", "Focus Tab") : LOCTEXT("OpenSequenceTabButtonText", "Open in Tab"))
					.Font(DetailBuilder.GetDetailFont())
				]
			];
	}

	// Only display an inline editor for non-blueprint sequences
	if (GetUISequence() && !GetUISequence()->GetParentBlueprint() && !bIsExternalTabAlreadyOpened)
	{
		Category.AddCustomRow(FText())
		.WholeRowContent()
		.MaxDesiredWidth(TOptional<float>())
		[
			SAssignNew(InlineSequencer, SBox)
			.HeightOverride(300)
			[
				SNew(SUISequenceEditorWidgetWrapper, WeakSequenceComponent)
			]
		];
	}

	DetailBuilder.HideProperty("AutoPlayLoopCount");
	IDetailCategoryBuilder& UISequenceCategory = DetailBuilder.EditCategory("UI Sequence", FText(), ECategoryPriority::Important);

	UISequenceCategory.AddProperty(TEXT("LoopType"));
	UISequenceCategory.AddProperty(TEXT("AutoPlayMode"));

	const auto AutoPlayModeProperty = DetailBuilder.GetProperty(TEXT("AutoPlayMode"));
	AutoPlayModeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	
	if (WeakSequenceComponent.IsValid() && WeakSequenceComponent->AutoPlayMode == EUISequenceAutoPlayMode::LoopExactly)
	{
		UISequenceCategory.AddProperty(TEXT("AutoPlayLoopCount"));
	}
	
	UISequenceCategory.AddProperty(TEXT("bAutoPlay"));
	UISequenceCategory.AddProperty(TEXT("ResumeAnimType"));
	UISequenceCategory.AddProperty(TEXT("bIgnoreTimeScale"));
	UISequenceCategory.AddProperty(TEXT("bPauseWhenOwnerDisabled"));
	UISequenceCategory.AddProperty(TEXT("PlayRate"));
	
	UISequenceCategory.AddProperty(TEXT("StartTimeOffset"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("Duration"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("DisplayRate"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("TickResolution"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("InputRate"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("Bindings"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("BoolCurves"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("FloatCurves"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("IntCurves"), nullptr, NAME_None, EPropertyLocation::Advanced);
	UISequenceCategory.AddProperty(TEXT("VectorCurves"), nullptr, NAME_None, EPropertyLocation::Advanced);
}

FReply FUISequenceComponentCustomization::InvokeSequencer()
{
	TSharedPtr<FTabManager> TabManager = WeakTabManager.Pin();
	if (TabManager.IsValid() && TabManager->HasTabSpawner(SequenceTabId))
	{
		if (TSharedPtr<SDockTab> Tab = TabManager->TryInvokeTab(SequenceTabId))
		{
			{
				// Set up a delegate that forces a refresh of this panel when the tab is closed to ensure we see the inline widget
				TWeakPtr<IPropertyUtilities> WeakUtilities = PropertyUtilities;
				auto OnClosed = [WeakUtilities](TSharedRef<SDockTab>)
				{
					TSharedPtr<IPropertyUtilities> PinnedPropertyUtilities = WeakUtilities.Pin();
					if (PinnedPropertyUtilities.IsValid())
					{
						PinnedPropertyUtilities->EnqueueDeferredAction(FSimpleDelegate::CreateSP(PinnedPropertyUtilities.ToSharedRef(), &IPropertyUtilities::ForceRefresh));
					}
				};

				Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(OnClosed));
			}

			// Move our inline widget content to the tab (so that we keep the existing sequencer state)
			if (InlineSequencer.IsValid())
			{
				Tab->SetContent(InlineSequencer->GetChildren()->GetChildAt(0));
				InlineSequencer->SetContent(SNullWidget::NullWidget);
				InlineSequencer->SetVisibility(EVisibility::Collapsed);
			}
			else
			{
				StaticCastSharedRef<SUISequenceEditorWidget>(Tab->GetContent())->AssignSequence(GetUISequence());
			}
		}
	}

	PropertyUtilities->ForceRefresh();

	return FReply::Handled();
}

UUISequence* FUISequenceComponentCustomization::GetUISequence() const
{
	UUISequenceComponent* SequenceComponent = WeakSequenceComponent.Get();
	return SequenceComponent ? SequenceComponent->Sequence : nullptr;
}

#undef LOCTEXT_NAMESPACE
