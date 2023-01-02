#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/Text/SDFFontCharset.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FSlateBrush;

class FCharsetItem : public FGCObject
{
public:
	/** Delegate for when the context menu requests a rename */
	DECLARE_DELEGATE(FOnRenameRequested);

	virtual ~FCharsetItem() override = default;
	
public:
	TWeakPtr<FCharsetItem> ParentSpriteItem;
	
	// Scope the creation of a node which ends when the initial 'name' is given/accepted by the user, which can be several frames after the node was actually created.
	TUniquePtr<FScopedTransaction> OngoingCreateTransaction;

	/** Handles rename requests */
	FOnRenameRequested RenameRequestedDelegate;

	USDFFontCharset* FontCharset;

public:
	FCharsetItem() : FontCharset(nullptr)
	{
	}

	FCharsetItem(TSharedPtr<FCharsetItem> InParentSpriteItem)
		: ParentSpriteItem(InParentSpriteItem)
		, FontCharset(nullptr)
	{
		
	}

	FCharsetItem(USDFFontCharset* InFontCharset)
		: FontCharset(InFontCharset)
	{
		
	}

	/**
	 * Requests a rename on the node.
	 * @param OngoingCreateTransaction The transaction scoping the node creation which will end once the node is named by the user or null if the rename is not part of a the creation process.
	 */
	void OnRequestRename(TUniquePtr<FScopedTransaction> InOngoingCreateTransaction)
	{
		OngoingCreateTransaction = MoveTemp(InOngoingCreateTransaction); // Take responsibility to end the 'create + give initial name' transaction.
		RenameRequestedDelegate.ExecuteIfBound();
	}

	/** Renames the object or variable represented by this node */
	virtual void OnCompleteRename(const FText& InNewName)
	{
		if (FontCharset)
		{
			FScopedTransaction TransactionContext(NSLOCTEXT("SCharsetsTabWidget", "SDFFontEditor_RenameCharset", "Rename charset"));
			
			FontCharset->Modify();
			FontCharset->Name = InNewName.ToString();
		}
		
		// If a 'create + give initial name' transaction exists, end it, the object is expected to have its initial name.
		CloseOngoingCreateTransaction();
	}

	/** Sets up the delegate for a rename operation */
	void SetRenameRequestedDelegate(FOnRenameRequested InRenameRequested) { RenameRequestedDelegate = InRenameRequested; }

	void CloseOngoingCreateTransaction()
	{
		OngoingCreateTransaction.Reset();
	}

public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(FontCharset);
	}

public:
	FString GetCharsetName() const
	{
		if (FontCharset)
		{
			return FontCharset->Name;
		}
		
		return TEXT("");
	}
	
};

class SCharsetItemWidget : public SMultiColumnTableRow<TSharedPtr<FCharsetItem>>
{
private:
	SLATE_BEGIN_ARGS(SCharsetItemWidget)
	{
	}

	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<FCharsetItem>& InItem, TWeakPtr<class SCharsetsTabWidget> InCharsetTabWidget);

public:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;
	
public:
	FText GetNameLabel() const;
	FSlateColor GetColorForNameLabel() const;
	void OnNameTextCommit(const FText& InNewName, ETextCommit::Type InTextCommit);
	bool OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage);
	
private:
	/** Drag-drop handlers */
	void HandleOnDragEnter(const FDragDropEvent& DragDropEvent);
	void HandleOnDragLeave(const FDragDropEvent& DragDropEvent);
	FReply HandleOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	TOptional<EItemDropZone> HandleOnCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TSharedPtr<FCharsetItem> TargetItem);
	FReply HandleOnAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TSharedPtr<FCharsetItem> TargetItem);
	
private:
	TSharedPtr<FCharsetItem> Item;
	TSharedPtr<SInlineEditableTextBlock> InlineWidget;

	TWeakPtr<class SCharsetsTabWidget> CharsetTabWidget;
	
};
