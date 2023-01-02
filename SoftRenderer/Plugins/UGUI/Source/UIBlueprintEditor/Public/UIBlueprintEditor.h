#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "BlueprintEditor.h"
#include "SSCSComponentEditor.h"
#include "Core/BehaviourComponent.h"

class UIBLUEPRINTEDITOR_API FUIBlueprintEditor : public FBlueprintEditor
{
public:
	FUIBlueprintEditor();

	virtual ~FUIBlueprintEditor() override;
	
	void InitUIBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode);
	
public:
	//~ Begin IBlueprintEditor Interface
	virtual void RefreshEditors(ERefreshBlueprintEditorReason::Type Reason = ERefreshBlueprintEditorReason::UnknownReason) override;
	virtual TSharedPtr<class FSCSEditorTreeNode> FindAndSelectSCSEditorTreeNode(const UActorComponent* InComponent, bool IsCntrlDown) override;
	//~ End IBlueprintEditor Interface

	void SelectedNodes(const TArray<URectTransformComponent*>& Components);
	
	// Begin FBlueprintEditor
	virtual void RegisterApplicationModes(const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode, bool bNewlyCreated = false) override;
	virtual FGraphAppearanceInfo GetGraphAppearance(class UEdGraph* InGraph) const override;
	virtual void CreateDefaultTabContents(const TArray<UBlueprint*>& InBlueprints) override;
	virtual void CreateDefaultCommands() override;
	// End FBlueprintEditor

	//~ Begin FNotifyHook Interface
	virtual void NotifyPostChange( const FPropertyChangedEvent& PropertyChangedEvent, FProperty* PropertyThatChanged) override;
	//~ End FNotifyHook Interface
	
	//~ Begin FTickableEditorObject Interface
	virtual void Tick(float DeltaTime) override;
	//~ End FTickableEditorObject Interface
	
public:
	void UpdatePreviewWidgetActor(UBlueprint* InBlueprint, bool bInForceFullUpdate = false);
	
	/** Getters for the various optional Kismet2 widgets */
	TSharedPtr<class SSCSComponentEditor> GetSCSComponentEditor() const { return SCSComponentEditor; }
	TSharedPtr<class SSCSUIEditorViewport> GetSCSUIViewport() const { return SCSUIViewport; }
	
	virtual void ClearSelectionStateFor(FName SelectionOwner) override;
	virtual void StartEditingDefaults(bool bAutoFocus = true, bool bForceRefresh = false) override;

	/** Called when Compile button is clicked */
	virtual void Compile() override;

	/** Refresh the preview viewport to reflect changes in the SCS */
	virtual void UpdateSCSUIPreview(bool bUpdateNow = false);

	virtual TArray<TSharedPtr<class FSCSComponentEditorTreeNode> > GetSelectedSCSComponentEditorTreeNodes() const;
	
protected:
	void CreateSCSEditorsForUIBlueprint();

	/** Delegate invoked when the selection is changed in the SCS component editor widget */
	void OnSelectionUpdatedForUIBlueprint(const TArray<TSharedPtr<class FSCSComponentEditorTreeNode>>& SelectedNodes, bool bUpdateDesigner);

	/** Delegate invoked when an item is double clicked in the SCS component editor widget */
	void OnComponentDoubleClickedForUIBlueprint(TSharedPtr<class FSCSComponentEditorTreeNode> Node);

	void UpdateSelectedNodes() const;
	
	void ConvertActorSequences();
	
protected:
	/** SCS component editor */
	TSharedPtr<class SSCSComponentEditor> SCSComponentEditor;

	/** Viewport widget */
	TSharedPtr<class SSCSUIEditorViewport> SCSUIViewport;

	TWeakObjectPtr<USimpleConstructionScript> OldSCS;

	TWeakObjectPtr<class AActor> OldWidgetActor;

	TArray<FSCSComponentEditorTreeNodePtrType> SelectedTreeNodes;
	
};
