#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

struct FSlateBrush;

class FSpriteItem : public FGCObject
{
public:
	/** Delegate for when the context menu requests a rename */
	DECLARE_DELEGATE(FOnRenameRequested);

	virtual ~FSpriteItem() override = default;
	
public:
	TWeakPtr<FSpriteItem> ParentSpriteItem;
	
	FString Name;

	FString Path;

	FString SuggestedImportPath;

	TArray<TSharedPtr<FSpriteItem>> SubSpriteItems;

	// Scope the creation of a node which ends when the initial 'name' is given/accepted by the user, which can be several frames after the node was actually created.
	TUniquePtr<FScopedTransaction> OngoingCreateTransaction;

	/** Handles rename requests */
	FOnRenameRequested RenameRequestedDelegate;

	TSharedPtr<FSlateDynamicImageBrush> ImageBrush;

	UTexture2D* Texture;

	TArray<uint8> RawData;
	
	int32 Width;
	int32 Height;

public:
	FSpriteItem(): Texture(nullptr), Width(0), Height(0)
	{
	}

	FSpriteItem(TSharedPtr<FSpriteItem> InParentSpriteItem, const FString& InName, const FString& InPath)
		: ParentSpriteItem(InParentSpriteItem)
		, Name(InName)
		, Path(InPath)
		, Texture(nullptr)
		, Width(0)
		, Height(0)
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
		Name = InNewName.ToString();
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
		Collector.AddReferencedObject(Texture);
	}
};

class SAtlasPackerSpriteItem : public SMultiColumnTableRow<TSharedPtr<FSpriteItem>>
{
private:
	SLATE_BEGIN_ARGS(SAtlasPackerSpriteItem)
		{
	}

	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<FSpriteItem>& InItem);

public:
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& InColumnName) override;
	
public:
	FText GetNameLabel() const;
	FSlateColor GetColorForNameLabel() const;
	void OnNameTextCommit(const FText& InNewName, ETextCommit::Type InTextCommit);
	bool OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage);
	
private:
	TSharedPtr<FSpriteItem> Item;
	TSharedPtr<SInlineEditableTextBlock> InlineWidget;
	
};

