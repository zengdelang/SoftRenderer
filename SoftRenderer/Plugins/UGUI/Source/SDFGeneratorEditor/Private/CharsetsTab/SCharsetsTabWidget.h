#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FCharsetItem;

class SCharsetsTabWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCharsetsTabWidget) {}
	SLATE_END_ARGS()

public:
	virtual ~SCharsetsTabWidget() override;
	
	/**
	* Construct this widget
	*
	* @param InArgs The declaration data for this widget.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<class FSDFFontEditor> InSDFFontEditor);

	// SWidget interface
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual bool SupportsKeyboardFocus() const override { return true; }
	// End SWidget interface

public:
	void PostUndo(bool bSuccess);

private:
	TSharedRef<ITableRow> CharsetsTreeView_OnGenerateRow(TSharedPtr<FCharsetItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	void CharsetsTreeView_OnGetChildren(TSharedPtr<FCharsetItem> Item, TArray<TSharedPtr<FCharsetItem>>& OutChildren);
	
	void CharsetsTreeView_OnSelectionChanged(TSharedPtr<FCharsetItem> Item, ESelectInfo::Type SelectInfo);

	void OnItemScrolledIntoView(TSharedPtr<FCharsetItem> InItem, const TSharedPtr<ITableRow>& InWidget);

	void RebuildCharsetTree();

public:
	TArray<TSharedPtr<FCharsetItem>> GetSelectedItems();
	void ReparentItems(const TArray<TSharedPtr<FCharsetItem>>& Items, TSharedPtr<FCharsetItem> TargetItem, EItemDropZone DropZone);

private:
	/** Called to display context menu when right clicking on the widget */
	TSharedPtr< SWidget > CreateContextMenu();

	/** Registers context menu by name for later access */
	void RegisterContextMenu();

private:
	/** Cut selected node(s) */
	void CutSelectedNodes();
	bool CanCutNodes() const;

	/** Copy selected node(s) */
	void CopySelectedNodes();
	bool CanCopyNodes() const;

	/** Pastes previously copied node(s) */
	void PasteNodes();
	bool CanPasteNodes() const;

	/** Callbacks to duplicate the selected component */
	bool CanDuplicateComponent() const;
	void OnDuplicateComponent();
	
	void OnDeleteNodes();
	bool CanDeleteNodes() const;

	void OnRenameNode(TUniquePtr<FScopedTransaction> InComponentCreateTransaction);
	void OnRenameNode();
	bool CanRenameNode() const;
 
	/** Called at the end of each frame. */
	void OnPostTick(float);

	FReply HandleAddCharset();
	FReply HandleAddDefaultCharsets();

private:
	TSharedPtr<STreeView<TSharedPtr<FCharsetItem>>> TreeView;
	TSharedPtr<FUICommandList> CommandList;

	TWeakPtr<class FSDFFontEditor> SDFFontEditor;

	TArray<TSharedPtr<FCharsetItem>> CharsetItems;
	
	// Rename
	FString DeferredRenameRequest;
	TUniquePtr<FScopedTransaction> DeferredOngoingCreateTransaction;
	FDelegateHandle PostTickHandle;
	
};
