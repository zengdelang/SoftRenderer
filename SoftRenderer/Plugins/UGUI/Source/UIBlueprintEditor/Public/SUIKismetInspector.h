#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SKismetInspector.h"

//////////////////////////////////////////////////////////////////////////
// SUIKismetInspector

class UIBLUEPRINTEDITOR_API SUIKismetInspector : public SKismetInspector
{
public:
	void Construct(const FArguments& InArgs, TSharedPtr<class FUIBlueprintEditor> InUIBlueprintEditorPtr);

	// SWidget interface
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	// End of SWidget interface
	
protected:
    UBlueprint* GetBlueprintObj() const { return Blueprint.Get(); }

private:
	const FSlateBrush* GetComponentIcon() const;
	
private:
	EVisibility GetBorderAreaVisibility() const;

private:
	FText OnGetVariableText() const;
	void OnVariableTextChanged(const FText& InNewText);
	void OnVariableTextCommitted(const FText& InNewName, ETextCommit::Type InTextCommit);
	bool OnVariableCanRename() const;

private:
	ECheckBoxState GetIsEnabled() const;
	void HandleEnabledChanged(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsVariable() const;
	void HandleIsVariableChanged(ECheckBoxState CheckState) const;

private:
	FText OnGetTooltipText() const;
	void OnTooltipTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit) const;

private:
	bool OnVariableCategoryChangeEnabled() const;
	FText OnGetVariableCategoryText() const;
	void OnVariableCategoryTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);
	void OnVariableCategorySelectionChanged(TSharedPtr<FText> ProposedSelection, ESelectInfo::Type /*SelectInfo*/) const;
	TSharedRef<ITableRow> MakeVariableCategoryViewWidget(TSharedPtr<FText> Item, const TSharedRef< STableViewBase >& OwnerTable) const;

	void PopulateVariableCategories();

private:
	ECheckBoxState GetIsEditableWhenInherited() const;
	void HandleIsEditableWhenInherited(ECheckBoxState CheckState);

private:
	ECheckBoxState GetIsGraying() const;
	void HandleIsGraying(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsInvertColor() const;
	void HandleIsInvertColor(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsInteractable() const;
	void HandleIsInteractable(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsBlockRaycasts() const;
	void HandleIsBlockRaycasts(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsIgnoreReversedGraphics() const;
	void HandleIsIgnoreReversedGraphics(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsIgnoreParentRenderOpacity() const;
	void HandleIsIgnoreParentRenderOpacity(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsIgnoreParentInteractable() const;
	void HandleIsIgnoreParentInteractable(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsIgnoreParentBlockRaycasts() const;
	void HandleIsIgnoreParentBlockRaycasts(ECheckBoxState CheckState) const;

private:
	ECheckBoxState GetIsIgnoreParentReversedGraphics() const;
	void HandleIsIgnoreParentReversedGraphics(ECheckBoxState CheckState) const;
	
private:
	float ZOrderValue = 0;
	
	TOptional<float> GetZOrderValue() const;
	void OnZOrderValueChanged(float NewZOrder);
	void OnZOrderValueCommitted(float NewZOrder, ETextCommit::Type CommitType) const;

private:
	float RenderOpacityValue = 0;
	
	TOptional<float> GetRenderOpacityValue() const;
	void OnRenderOpacityValueChanged(float NewRenderOpacity);
	void OnRenderOpacityValueCommitted(float NewRenderOpacity, ETextCommit::Type CommitType) const;
	
private:
	/** Pointer back to my parent tab */
	TWeakObjectPtr<UBlueprint> Blueprint;
	
	/** Weak reference to the Blueprint editor */
	TWeakPtr<class FUIBlueprintEditor> UIBlueprintEditorPtr;

	/** The cached tree Node we're editing */
	TSharedPtr<class FSCSComponentEditorTreeNode> CachedNodePtr;
	
	/** The widget used when in variable name editing mode */ 
	TSharedPtr<SEditableTextBox> VariableNameEditableTextBox;

	/** The container widget for the class link users can click to open another asset */
	TSharedPtr<SBox> ClassLinkArea;
	
	/** Flag to indicate whether or not the variable name is invalid */
	bool bIsVariableNameInvalid = false;

	/** A list of all category names to choose from */
	TArray<TSharedPtr<FText>> VariableCategorySource;
	
	/** Widgets for the categories */
	TSharedPtr<SComboButton> VariableCategoryComboButton;
	TSharedPtr<SListView<TSharedPtr<FText>>> VariableCategoryListView;
	
};
