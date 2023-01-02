#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerPrivate.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class FSpriteItem;

class SSpriteAtlasPackerSpriteList : public SSpriteAtlasPackerBaseWidget
{
public:
	SLATE_BEGIN_ARGS(SSpriteAtlasPackerSpriteList) { }
	SLATE_END_ARGS()

public:
	virtual ~SSpriteAtlasPackerSpriteList() override;
	
	/**
	* Construct this widget
	*
	* @param InArgs The declaration data for this widget.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList);

	// SWidget interface
	virtual FReply OnDragOver( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	virtual FReply OnDrop( const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent ) override;
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	// End SWidget interface

private:
	TSharedRef<ITableRow> SpritesTreeView_OnGenerateRow(TSharedPtr<FSpriteItem> Item, const TSharedRef<STableViewBase>& OwnerTable);
	
	void SpritesTreeView_OnGetChildren(TSharedPtr<FSpriteItem> Item, TArray<TSharedPtr<FSpriteItem>>& OutChildren);
	
	void SpritesTreeView_OnSelectionChanged(TSharedPtr<FSpriteItem> Item, ESelectInfo::Type SelectInfo);

	void OnItemScrolledIntoView(TSharedPtr<FSpriteItem> InItem, const TSharedPtr<ITableRow>& InWidget);

	void RebuildSpriteTree();

private:
	/** Called to display context menu when right clicking on the widget */
	TSharedPtr< SWidget > CreateContextMenu();

	/** Registers context menu by name for later access */
	void RegisterContextMenu();

	void OnFindInExplorer() const;

private:
	void OnSpritesListChanged(TArray<TSharedPtr<FSpriteItem>>& NewSprites);

	void OnDeleteNodes();
	bool CanDeleteNodes() const;

	void OnRenameNode(TUniquePtr<FScopedTransaction> InComponentCreateTransaction);
	void OnRenameNode();
	bool CanRenameNode() const;

	EVisibility OnEmptyTipsVisibility() const;
	
	/** Called at the end of each frame. */
	void OnPostTick(float);
	
private:
	TSharedPtr<STreeView<TSharedPtr<FSpriteItem>>> TreeView;
	TSharedPtr<FUICommandList> CommandList;

	// Rename
	FString DeferredRenameRequest;
	TUniquePtr<FScopedTransaction> DeferredOngoingCreateTransaction;
	FDelegateHandle PostTickHandle;
	
};
