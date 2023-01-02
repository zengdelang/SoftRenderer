#include "UIBlueprintEditor.h"
#include "SSCSEditor.h"
#include "BlueprintEditorModes.h"
#include "BlueprintEditorTabs.h"
#include "SUIKismetInspector.h"
#include "SSCSComponentEditor.h"
#include "SSCSUIEditorViewport.h"
#include "UGUISubsystem.h"
#include "UIBlueprintEditorCommands.h"
#include "UIBlueprintEditorModes.h"
#include "UISequenceComponent.h"
#include "UISequenceEditorSelection.h"
#include "Core/UIBlueprint.h"
#include "Core/WidgetActor.h"
#include "Core/Layout/RectTransformPreviewComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Designer/DesignerEditorEventViewportClient.h"
#include "UISequenceEditorSelection.h"

#define LOCTEXT_NAMESPACE "BlueprintEditor"

FUIBlueprintEditor::FUIBlueprintEditor()
{
	PreviewScene.GetWorld()->ExtraReferencedObjects.Add(NewObject<UDesignerEditorEventViewportClient>(GetTransientPackage()));
	PreviewScene.GetWorld()->bAllowAudioPlayback = true;
}

FUIBlueprintEditor::~FUIBlueprintEditor()
{

}

void FUIBlueprintEditor::InitUIBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode)
{
	TSharedPtr<FUIBlueprintEditor> ThisPtr(SharedThis(this));
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);

	const UBlueprint* BlueprintObj = GetBlueprintObj();
	if (IsValid(BlueprintObj) && BlueprintObj->SimpleConstructionScript != nullptr)
	{
		OldSCS = BlueprintObj->SimpleConstructionScript;
	}

	const auto CommandList = GetToolkitCommands();
	CommandList->MapAction(
		FUIBlueprintEditorCommands::Get().ConvertActorSequences,
		FExecuteAction::CreateSP(this, &FUIBlueprintEditor::ConvertActorSequences),
		FCanExecuteAction());
}

void FUIBlueprintEditor::RefreshEditors(ERefreshBlueprintEditorReason::Type Reason)
{
	if (CurrentUISelection == SelectionState_Components)
	{
		if (SCSComponentEditor.IsValid())
		{
			SCSComponentEditor->RefreshSelectionDetails();
		}
	}
	
	FBlueprintEditor::RefreshEditors(Reason);

	if (SCSComponentEditor.IsValid())
	{
		SCSComponentEditor->UpdateTree();
		
		// Note: Don't pass 'true' here because we don't want the preview actor to be reconstructed until after Blueprint modification is complete.
		UpdateSCSUIPreview();
	}
}

TSharedPtr<FSCSEditorTreeNode> FUIBlueprintEditor::FindAndSelectSCSEditorTreeNode(const UActorComponent* InComponent,
                                                                                  const bool bIsCntrlDown)
{
	if (SCSComponentEditor.IsValid())
	{
		const FSCSComponentEditorTreeNodePtrType NodePtr = SCSComponentEditor->GetNodeFromActorComponent(InComponent);
		if(NodePtr.IsValid())
		{
			SCSComponentEditor->SelectNode(NodePtr, bIsCntrlDown);
		}
	}
	
	return FBlueprintEditor::FindAndSelectSCSEditorTreeNode(InComponent, bIsCntrlDown);
}

void FUIBlueprintEditor::SelectedNodes(const TArray<URectTransformComponent*>& Components)
{
	if (SCSComponentEditor.IsValid())
	{
		SelectedTreeNodes.Reset();

		for (const auto& Comp : Components)
		{
			if (Comp)
			{
				const FSCSComponentEditorTreeNodePtrType NodePtr = SCSComponentEditor->GetNodeFromActorComponent(Comp);
				if(NodePtr.IsValid())
				{
					SelectedTreeNodes.Add(NodePtr);
				}
			}
		}

		SCSComponentEditor->SelectNodes(SelectedTreeNodes);
	}
}

void FUIBlueprintEditor::RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated/* = false*/)
{
	AddApplicationMode(
	FBlueprintEditorApplicationModes::StandardBlueprintEditorMode,
	MakeShareable(new FUIBlueprintEditorUnifiedMode(SharedThis(this), FBlueprintEditorApplicationModes::StandardBlueprintEditorMode, FBlueprintEditorApplicationModes::GetLocalizedMode, CanAccessComponentsMode())));

	SetCurrentMode(FBlueprintEditorApplicationModes::StandardBlueprintEditorMode);

	TabManager->TryInvokeTab(FBlueprintEditorTabs::SCSViewportID);
}

FGraphAppearanceInfo FUIBlueprintEditor::GetGraphAppearance(UEdGraph* InGraph) const
{
	FGraphAppearanceInfo AppearanceInfo = FBlueprintEditor::GetGraphAppearance(InGraph);

	if (GetBlueprintObj()->IsA(UUIBlueprint::StaticClass()))
	{
		AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "UI BLUEPRINT");
	}

	return AppearanceInfo;
}

void FUIBlueprintEditor::CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints)
{
	FBlueprintEditor::CreateDefaultTabContents(InBlueprints);

	UBlueprint* InBlueprint = InBlueprints.Num() == 1 ? InBlueprints[0] : nullptr;
	if (InBlueprint && 
		InBlueprint->ParentClass &&
		InBlueprint->ParentClass->IsChildOf(AActor::StaticClass()) && 
		InBlueprint->SimpleConstructionScript)
	{
		CreateSCSEditorsForUIBlueprint();
	}

	this->Inspector = 
		SNew(SUIKismetInspector, SharedThis(this))
			. HideNameArea(true)
			. ViewIdentifier(FName("BlueprintInspector"))
			. Kismet2(SharedThis(this))
			. OnFinishedChangingProperties( FOnFinishedChangingProperties::FDelegate::CreateSP(this, &FUIBlueprintEditor::OnFinishedChangingProperties) );
}

void FUIBlueprintEditor::CreateDefaultCommands()
{
	FUIBlueprintEditorCommands::Register();
	FBlueprintEditor::CreateDefaultCommands();
}

void FUIBlueprintEditor::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
	FProperty* PropertyThatChanged)
{
	FString PropertyName = PropertyThatChanged->GetName();
	if (FocusedGraphEdPtr.IsValid())
	{
		FocusedGraphEdPtr.Pin()->NotifyPostPropertyChange(PropertyChangedEvent, PropertyName);
	}
	
	if (IsEditingSingleBlueprint())
	{
		UBlueprint* Blueprint = GetBlueprintObj();
		UPackage* BlueprintPackage = Blueprint->GetOutermost();

		// if any of the objects being edited are in our package, mark us as dirty
		bool bPropertyInBlueprint = false;
		for (int32 ObjectIndex = 0; ObjectIndex < PropertyChangedEvent.GetNumObjectsBeingEdited(); ++ObjectIndex)
		{
			const UObject* Object = PropertyChangedEvent.GetObjectBeingEdited(ObjectIndex);
			if (Object && Object->GetOutermost() == BlueprintPackage)
			{
				bPropertyInBlueprint = true;
				break;
			}
		}

		if (bPropertyInBlueprint)
		{
			// Note: if change type is "interactive," hold off on applying the change (e.g. this will occur if the user is scrubbing a spinbox value; we don't want to apply the change until the mouse is released, for performance reasons)
			if (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
			{
				FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint, PropertyChangedEvent);

				// Call PostEditChange() on any Actors that might be based on this Blueprint
				FBlueprintEditorUtils::PostEditChangeBlueprintActors(Blueprint);
			}

			if (PropertyChangedEvent.GetPropertyName() == TEXT("SubComponents"))
			{
				// Force updates to occur immediately during interactive mode (otherwise the preview won't refresh because it won't be ticking)
				UpdateSCSPreview(PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive);
				UpdateSCSUIPreview(PropertyChangedEvent.ChangeType == EPropertyChangeType::Interactive);
			}
		}
	}
}

void FUIBlueprintEditor::Tick(float DeltaTime)
{
	FBlueprintEditor::Tick(DeltaTime);

	// Create or update the Blueprint actor instance in the preview scene
	if ( GetPreviewActor() == nullptr )
	{
		UpdatePreviewWidgetActor(GetBlueprintObj(), true);
	}

	if (OldWidgetActor != GetPreviewActor())
	{
		OldWidgetActor = GetPreviewActor();
		const auto WidgetActor = Cast<AWidgetActor>(OldWidgetActor.Get());
		if (IsValid(WidgetActor))
		{
#if WITH_EDITORONLY_DATA
			WidgetActor->OnBackgroundImageFilePathChanged.Broadcast(WidgetActor, WidgetActor->BackgroundImageFilePath);
#endif
		}
	}
	
	if (!OldSCS.IsValid())
	{
		const UBlueprint* BlueprintObj = GetBlueprintObj();
		if (IsValid(BlueprintObj) && BlueprintObj->SimpleConstructionScript != nullptr)
		{
			OldSCS = BlueprintObj->SimpleConstructionScript;
			CreateSCSEditorsForUIBlueprint();
		}
	}
}

void FUIBlueprintEditor::UpdatePreviewWidgetActor(UBlueprint* InBlueprint, bool bInForceFullUpdate)
{
	UpdatePreviewActor(InBlueprint, bInForceFullUpdate);
	UpdateSelectedNodes();

	if (SCSUIViewport.IsValid())
	{
		const auto ViewportClient = StaticCastSharedPtr<FSCSUIEditorViewportClient>(SCSUIViewport->GetViewportClient());
		if (ViewportClient.IsValid())
		{
			ViewportClient->bUpdateWidgetActorComponents = true;
		}
	}
}

void FUIBlueprintEditor::ClearSelectionStateFor(FName SelectionOwner)
{
	FBlueprintEditor::ClearSelectionStateFor(SelectionOwner);

	if ( SelectionOwner == SelectionState_Components )
	{
		if (SCSComponentEditor.IsValid())
		{
			SCSComponentEditor->ClearSelection();
		}
	}
}

void FUIBlueprintEditor::StartEditingDefaults(bool bAutoFocus, bool bForceRefresh)
{
	SetUISelectionState(FBlueprintEditor::SelectionState_ClassDefaults);

	if (IsEditingSingleBlueprint())
	{
		if (GetBlueprintObj()->GeneratedClass != nullptr)
		{
			if (SCSEditor.IsValid() && GetBlueprintObj()->GeneratedClass->IsChildOf<AActor>())
			{
				SCSEditor->SelectRoot();
			}
			else if (SCSComponentEditor.IsValid() && GetBlueprintObj()->GeneratedClass->IsChildOf<AActor>())
			{
				SCSComponentEditor->SelectRoot();
			}
			else
			{
				UObject* DefaultObject = GetBlueprintObj()->GeneratedClass->GetDefaultObject();

				// Update the details panel
				FString Title;
				DefaultObject->GetName(Title);
				SKismetInspector::FShowDetailsOptions Options(FText::FromString(Title), bForceRefresh);
				Options.bShowComponents = false;

				Inspector->ShowDetailsForSingleObject(DefaultObject, Options);

				if ( bAutoFocus )
				{
					TryInvokingDetailsTab();
				}
			}
		}
	}
	
	RefreshStandAloneDefaultsEditor();
}

void FUIBlueprintEditor::Compile()
{
	const auto Blueprint = GetBlueprintObj();

	if (!Blueprint)
		return;
	
	TArray<USCS_Node*> RootNodes = Blueprint->SimpleConstructionScript->GetRootNodes();
	for (const auto& RootNode : RootNodes)
	{
		const UUISequenceComponent* UISequenceComp = Cast<UUISequenceComponent>(RootNode->ComponentTemplate);
		if (IsValid(UISequenceComp) && UISequenceComp->Sequence)
		{
			UISequenceComp->Sequence->bChanged = true;
		}
	}
	
	FBlueprintEditor::Compile();
	
	// Do for ReparentBlueprint_NewParentChosen
	if (SCSComponentEditor.IsValid())
	{
		SCSComponentEditor->UpdateTree();
	}
}

void FUIBlueprintEditor::UpdateSCSUIPreview(bool bUpdateNow)
{
	// refresh widget
	if(SCSUIViewport.IsValid())
	{
		TSharedPtr<SDockTab> OwnerTab = Inspector->GetOwnerTab();
		if ( OwnerTab.IsValid() )
		{
			bUpdateNow &= OwnerTab->IsForeground();
		}

		// Only request a refresh immediately if the viewport tab is in the foreground.
		SCSUIViewport->RequestRefresh(false, bUpdateNow);
	}
}

TArray<TSharedPtr<FSCSComponentEditorTreeNode>> FUIBlueprintEditor::GetSelectedSCSComponentEditorTreeNodes() const
{
	TArray<TSharedPtr<FSCSComponentEditorTreeNode>>  Nodes;
	if (SCSComponentEditor.IsValid())
	{
		Nodes = SCSComponentEditor->GetSelectedNodes();
	}
	return Nodes;
}

void FUIBlueprintEditor::CreateSCSEditorsForUIBlueprint()
{
	SCSComponentEditor = SAssignNew(SCSComponentEditor, SSCSComponentEditor)
		.BlueprintEditor(SharedThis(this))
		.ActorContext(this, &FBlueprintEditor::GetSCSEditorActorContext)
		.PreviewActor(this, &FBlueprintEditor::GetPreviewActor)
		.AllowEditing(this, &FBlueprintEditor::InEditingMode)
		.OnSelectionUpdated(this, &FUIBlueprintEditor::OnSelectionUpdatedForUIBlueprint)
		.OnItemDoubleClicked(this, &FUIBlueprintEditor::OnComponentDoubleClickedForUIBlueprint);

	SCSUIViewport = SAssignNew(SCSUIViewport, SSCSUIEditorViewport)
		.BlueprintEditor(SharedThis(this));
}

void FUIBlueprintEditor::OnSelectionUpdatedForUIBlueprint(
	const TArray<TSharedPtr<FSCSComponentEditorTreeNode>>& SelectedNodes, bool bUpdateDesigner)
{
	if (SCSUIViewport.IsValid())
	{
		SCSUIViewport->OnComponentSelectionChanged();
	}

	UBlueprint* Blueprint = GetBlueprintObj();
	check(Blueprint != nullptr && Blueprint->SimpleConstructionScript != nullptr);

	// Update the selection visualization
	AActor* EditorActorInstance = Blueprint->SimpleConstructionScript->GetComponentEditorActorInstance();
	if (EditorActorInstance != nullptr)
	{
		for (UActorComponent* Component : EditorActorInstance->GetComponents())
		{
			if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
			{
				PrimitiveComponent->PushSelectionToProxy();
			}
		}
	}

	if (Inspector.IsValid())
	{
		// Clear the my blueprints selection
		if ( SelectedNodes.Num() > 0 )
		{
			SetUISelectionState(FBlueprintEditor::SelectionState_Components);
		}

		// Convert the selection set to an array of UObject* pointers
		FText InspectorTitle = FText::GetEmpty();
		TArray<UObject*> InspectorObjects;
		bool bShowComponents = true;
		InspectorObjects.Empty(SelectedNodes.Num());
		for (FSCSComponentEditorTreeNodePtrType NodePtr : SelectedNodes)
		{
			if (NodePtr.IsValid())
			{
				if (NodePtr->IsActorNode())
				{
					if (AActor* DefaultActor = NodePtr->GetEditableObjectForBlueprint<AActor>(GetBlueprintObj()))
					{
						InspectorObjects.Add(DefaultActor);

						FString Title;
						DefaultActor->GetName(Title);
						InspectorTitle = FText::FromString(Title);
						bShowComponents = false;

						TryInvokingDetailsTab();
					}
				}
				else
				{
					UActorComponent* EditableComponent = NodePtr->GetOrCreateEditableComponentTemplate(GetBlueprintObj());
					if (EditableComponent)
					{
						InspectorTitle = FText::FromString(NodePtr->GetDisplayString());
						InspectorObjects.Add(EditableComponent);
					}

					if ( SCSUIViewport.IsValid() )
					{
						TSharedPtr<SDockTab> OwnerTab = SCSUIViewport->GetOwnerTab();
						if ( OwnerTab.IsValid() )
						{
							OwnerTab->FlashTab();
						}
					}
				}
			}
		}

		// Update the details panel
		SKismetInspector::FShowDetailsOptions Options(InspectorTitle, true);
		Options.bShowComponents = bShowComponents;
		Inspector->ShowDetailsForObjects(InspectorObjects, Options);
	}

	UpdateSelectedNodes();

	if (bUpdateDesigner)
	{
		if (SCSUIViewport.IsValid())
		{
			const auto ViewportClient = StaticCastSharedPtr<FSCSUIEditorViewportClient>(SCSUIViewport->GetViewportClient());
			if (ViewportClient.IsValid())
			{
				ViewportClient->bUpdateWidgetActorComponents = true;
			}
		}
	}
}

void FUIBlueprintEditor::OnComponentDoubleClickedForUIBlueprint(TSharedPtr<FSCSComponentEditorTreeNode> Node)
{
	const TSharedPtr<SDockTab> OwnerTab = Inspector->GetOwnerTab();
	if ( OwnerTab.IsValid() )
	{
		GetTabManager()->TryInvokeTab(FBlueprintEditorTabs::SCSViewportID);
	}
}

void FUIBlueprintEditor::UpdateSelectedNodes() const
{
#if SUPPORT_UI_BLUEPRINT_EDITOR
	auto SelectedNodes = GetSelectedSCSComponentEditorTreeNodes();
	
	const bool bMultiSelected = SelectedNodes.Num() > 1;

	TArray<FSCSComponentEditorTreeNodePtrType> CurSelectedNodes;
	FSCSComponentEditorTreeNodePtrType ParentNode = nullptr;
	FSCSComponentEditorTreeNodePtrType SingleSelectedNode = nullptr;
	
	if (!bMultiSelected && SelectedNodes.Num() > 0)
	{
		SingleSelectedNode = SelectedNodes[0];
		
		ParentNode = SelectedNodes[0]->GetParent();
		if (ParentNode.IsValid())
		{
			CurSelectedNodes.Append(ParentNode->GetChildren());
			CurSelectedNodes.Remove(SingleSelectedNode);
		}
	}

	const auto PreviewWorld = PreviewScene.GetWorld();
	if (IsValid(PreviewWorld))
	{
		URectTransformPreviewComponent::ClearEditorUIDesignerInfo(PreviewWorld);
	}

	FEditorUIDesignerInfo* UIDesignerInfo = URectTransformPreviewComponent::GetEditorUIDesignerInfo(PreviewWorld);
	
	AActor* PreviewActor = GetPreviewActor();
	if (PreviewActor != nullptr)
	{
		TSet<TWeakObjectPtr<UActorComponent>> ActorSequenceSelection;
		for (const auto& SelectedNode : SelectedNodes)
		{
			FSCSComponentEditorTreeNodePtrType SelectedNodeParentNode = SelectedNode->GetParent();
			while (SelectedNodeParentNode.IsValid())
			{
				UActorComponent* ActorComponent = Cast<UActorComponent>(SelectedNodeParentNode->FindComponentInstanceInActor(PreviewActor));
				if(ActorComponent)
				{
					ActorSequenceSelection.Add(ActorComponent);
				}
				SelectedNodeParentNode = SelectedNodeParentNode->GetParent();
			}
			
			if (SelectedNode.IsValid())
			{
				UActorComponent* ActorComponent = Cast<UActorComponent>(SelectedNode->FindComponentInstanceInActor(PreviewActor));
				if(ActorComponent)
				{
					ActorSequenceSelection.Add(ActorComponent);
				}
			}
		}
		FUISequenceEditorSelection::SequenceActorSelection.Add(PreviewActor, ActorSequenceSelection);
		
		for (UActorComponent* Component : PreviewActor->GetComponents())
		{
			if (UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(Component))
			{
				BehaviourComp->UpdateRectTransformPreview();
			}
		}
	}

	const FSCSComponentEditorTreeNodePtrType RootNode = SCSComponentEditor->GetSceneRootNode();
	if (RootNode)
	{
		UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(RootNode->FindComponentInstanceInActor(PreviewActor));
		if(BehaviourComp)
		{
			BehaviourComp->UpdateRectTransformPreview();

			if (UIDesignerInfo)
			{
				UIDesignerInfo->RootRectComponent = RootNode->GetComponentTemplate();
			}
		}
	}

	if (UIDesignerInfo)
	{
		UIDesignerInfo->bMultiSelected = bMultiSelected;
	}
	
	if (ParentNode.IsValid())
	{
		UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(ParentNode->FindComponentInstanceInActor(PreviewActor));
		if(BehaviourComp)
		{
			BehaviourComp->UpdateRectTransformPreview();

			if (UIDesignerInfo)
			{
				UIDesignerInfo->ParentSelectedRectComponent = ParentNode->GetComponentTemplate();
			}
		}
	}

	if (SingleSelectedNode.IsValid())
	{
		UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(SingleSelectedNode->FindComponentInstanceInActor(PreviewActor));
		if(BehaviourComp)
		{
			BehaviourComp->UpdateRectTransformPreview();

			if (UIDesignerInfo)
			{
				UIDesignerInfo->CurSelectedRectComponent = SingleSelectedNode->GetComponentTemplate();
			}
		}
	}

	for (const auto& SelectedNode : CurSelectedNodes)
	{
		if (SelectedNode.IsValid())
		{
			UBehaviourComponent* BehaviourComp = Cast<UBehaviourComponent>(SelectedNode->FindComponentInstanceInActor(PreviewActor));
			if(BehaviourComp)
			{
				BehaviourComp->UpdateRectTransformPreview();

				if (UIDesignerInfo)
				{
					UIDesignerInfo->CurSelectedSiblingRectComponents.Emplace(SelectedNode->GetComponentTemplate());
				}
			}
		}
	}
#endif
}

void FUIBlueprintEditor::ConvertActorSequences()
{
	if (SCSComponentEditor.IsValid())
	{
		SCSComponentEditor->ConvertActorSequences();
	}
}

#undef LOCTEXT_NAMESPACE
