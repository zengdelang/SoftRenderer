#include "SSpriteAtlasPackerSpriteList.h"
#include "SSCSComponentEditor.h"
#include "SSpriteAtlasPackerSpriteListItem.h"
#include "ToolMenus.h"
#include "Framework/Commands/GenericCommands.h"

#define LOCTEXT_NAMESPACE "SSpriteAtlasPackerViewport"

static const FName ColumnName_SpriteRoot( "SpriteRoot" );
static const FName Sprite_ContextMenuName( "SpriteItemContextMenu" );

SSpriteAtlasPackerSpriteList::~SSpriteAtlasPackerSpriteList()
{
	FSpriteAtlasPacker::Get().GetEvents().OnSpriteListChangedEvent.RemoveAll(this);
}

void SSpriteAtlasPackerSpriteList::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList)
{
	CommandList = InCommandList;

	CommandList->MapAction( FGenericCommands::Get().Delete,
		FUIAction( FExecuteAction::CreateSP( this, &SSpriteAtlasPackerSpriteList::OnDeleteNodes ), 
		FCanExecuteAction::CreateSP( this, &SSpriteAtlasPackerSpriteList::CanDeleteNodes ) ) 
		);

	CommandList->MapAction( FGenericCommands::Get().Rename,
		FUIAction( FExecuteAction::CreateSP( this, &SSpriteAtlasPackerSpriteList::OnRenameNode),
		FCanExecuteAction::CreateSP( this, &SSpriteAtlasPackerSpriteList::CanRenameNode ) ) 
		);

	TSharedPtr<SHeaderRow> HeaderRow = SNew(SHeaderRow)
	+ SHeaderRow::Column(ColumnName_SpriteRoot)
	.DefaultLabel(LOCTEXT("SpriteRoot", "SpriteRoot"))
	.FillWidth(4);
	
	// Create the tree view control
	TreeView =
		SNew( STreeView<TSharedPtr<FSpriteItem>> )
		.TreeItemsSource( &FSpriteAtlasPacker::Get().GetRootSprites() )
		.SelectionMode(ESelectionMode::Multi)
		.ClearSelectionOnClick(false)
		.OnItemScrolledIntoView(this, &SSpriteAtlasPackerSpriteList::OnItemScrolledIntoView)
		.OnGenerateRow( this, &SSpriteAtlasPackerSpriteList::SpritesTreeView_OnGenerateRow ) 
		.OnGetChildren( this, &SSpriteAtlasPackerSpriteList::SpritesTreeView_OnGetChildren )
		.OnContextMenuOpening(this, &SSpriteAtlasPackerSpriteList::CreateContextMenu)
		.OnSelectionChanged( this, &SSpriteAtlasPackerSpriteList::SpritesTreeView_OnSelectionChanged )
		.ItemHeight(24)
		.HeaderRow
		(
			HeaderRow
		);

	TreeView->GetHeaderRow()->SetVisibility(EVisibility::Collapsed);

	RebuildSpriteTree();

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.Padding(0)
		[
			TreeView.ToSharedRef()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(0)
		[
			SNew(STextBlock)
			.Visibility(this, &SSpriteAtlasPackerSpriteList::OnEmptyTipsVisibility)
			.Text(LOCTEXT("EmptyTips", "Drop sprites or folders here"))
		]
	];

	FSpriteAtlasPacker::Get().GetEvents().OnSpriteListChangedEvent.AddSP(this, &SSpriteAtlasPackerSpriteList::OnSpritesListChanged);
}

FReply SSpriteAtlasPackerSpriteList::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return FSpriteAtlasPacker::Get().CanHandleDrag(DragDropEvent);
}

FReply SSpriteAtlasPackerSpriteList::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	return FSpriteAtlasPacker::Get().TryHandleDragDrop(DragDropEvent);
}

FReply SSpriteAtlasPackerSpriteList::OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent )
{
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

TSharedRef<ITableRow> SSpriteAtlasPackerSpriteList::SpritesTreeView_OnGenerateRow(TSharedPtr<FSpriteItem> Item,
                                                                                  const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew( SAtlasPackerSpriteItem, OwnerTable, Item.ToSharedRef());
}

void SSpriteAtlasPackerSpriteList::SpritesTreeView_OnGetChildren(TSharedPtr<FSpriteItem> Item,
	TArray<TSharedPtr<FSpriteItem>>& OutChildren)
{
	OutChildren.Append(Item->SubSpriteItems);
}

void SSpriteAtlasPackerSpriteList::SpritesTreeView_OnSelectionChanged(TSharedPtr<FSpriteItem> Item,
	ESelectInfo::Type SelectInfo)
{
	const auto& SelectedItems = TreeView->GetSelectedItems();
	if (SelectedItems.Num() > 0)
	{
		const auto& TargetItem = SelectedItems[0];
		FSpriteAtlasPacker::Get().SelectedSprite = TargetItem;
		FSpriteAtlasPacker::Get().GetEvents().OnSpriteListSelectionChangedEvent.Broadcast(TargetItem->Width, TargetItem->Height, TargetItem->Texture);
	}
	else
	{
		FSpriteAtlasPacker::Get().SelectedSprite = nullptr;
		FSpriteAtlasPacker::Get().GetEvents().OnSpriteListSelectionChangedEvent.Broadcast(2048, 2048, nullptr);
	}
}

void SSpriteAtlasPackerSpriteList::RebuildSpriteTree()
{
	const auto& SelectedItems = FSpriteAtlasPacker::Get().GetRootSprites();
	if (SelectedItems.Num() > 0)
	{
		const auto& TargetItem = SelectedItems[0];
		FSpriteAtlasPacker::Get().SelectedSprite = TargetItem;
		FSpriteAtlasPacker::Get().GetEvents().OnSpriteListSelectionChangedEvent.Broadcast(TargetItem->Width, TargetItem->Height, TargetItem->Texture);
	}
	
	// Refresh the view
	TreeView->RequestTreeRefresh();
}

TSharedPtr<SWidget> SSpriteAtlasPackerSpriteList::CreateContextMenu()
{
	const auto& SelectedItems = TreeView->GetSelectedItems();

	if (SelectedItems.Num() > 0)
	{
		RegisterContextMenu();

		FToolMenuContext ToolMenuContext(CommandList, TSharedPtr<FExtender>());
		return UToolMenus::Get()->GenerateWidget(Sprite_ContextMenuName, ToolMenuContext);
	}
	return TSharedPtr<SWidget>();
}

void SSpriteAtlasPackerSpriteList::RegisterContextMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus->IsMenuRegistered(Sprite_ContextMenuName))
	{
		UToolMenu* Menu = ToolMenus->RegisterMenu(Sprite_ContextMenuName);
		Menu->AddDynamicSection("SCSEditorDynamic", FNewToolMenuDelegate::CreateLambda([&](UToolMenu* InMenu)
		{
			FToolMenuSection& Section = InMenu->AddSection("SpriteListItem", LOCTEXT("SpriteListItemHeading", "Edit"));
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Rename);
			
			FToolMenuSection& MenuSection = InMenu->AddSection("SpriteFile", LOCTEXT("SpriteFileHeading", "File"));

			const auto& SelectedItems = TreeView->GetSelectedItems();
			if (SelectedItems.Num() == 1)
			{
				MenuSection.AddMenuEntry(
					"FindInExplorer",
					LOCTEXT("FindInExplorer", "Find in Explorer"),
					LOCTEXT("FindInExplorer_ToolTip", ""),
					FSlateIcon(FEditorStyle::GetStyleSetName(), ""),
					FUIAction(
						FExecuteAction::CreateSP(this, &SSpriteAtlasPackerSpriteList::OnFindInExplorer),
						FCanExecuteAction()));
			}
		}));
	}
}

void SSpriteAtlasPackerSpriteList::OnFindInExplorer() const
{
	const auto& SelectedItems = TreeView->GetSelectedItems();
	if (SelectedItems.Num() == 1)
	{
		FPlatformProcess::ExploreFolder(*IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*SelectedItems[0]->Path));
	}
}

void SSpriteAtlasPackerSpriteList::OnSpritesListChanged(TArray<TSharedPtr<FSpriteItem>>& NewSprites)
{
	TreeView->RequestTreeRefresh();

	TreeView->ClearSelection();
	TreeView->SetItemSelection(NewSprites, true);

	if (DeferredOngoingCreateTransaction.IsValid())
	{
		// Close the ongoing 'add' sub-transaction before staring another one. The user will not be able to edit the name of that component because the
		// new component is going to still focus.
		DeferredOngoingCreateTransaction.Reset();
	}
}

void SSpriteAtlasPackerSpriteList::OnDeleteNodes()
{
	const auto& SelectedItems = TreeView->GetSelectedItems();
	for (auto& Elem : SelectedItems)
	{
		FSpriteAtlasPacker::Get().DeleteSelectedItem(Elem);
	}
	
	TreeView->RequestTreeRefresh();
}

bool SSpriteAtlasPackerSpriteList::CanDeleteNodes() const
{
	return true;
}

void SSpriteAtlasPackerSpriteList::OnRenameNode(TUniquePtr<FScopedTransaction> InComponentCreateTransaction)
{
	auto SelectedItems = TreeView->GetSelectedItems();

	// Should already be prevented from making it here.
	check(SelectedItems.Num() == 1);

	DeferredRenameRequest = SelectedItems[0]->Name;

	check(!DeferredOngoingCreateTransaction.IsValid()); // If this fails, something in the chain of responsibility failed to end the previous transaction.
	DeferredOngoingCreateTransaction = MoveTemp(InComponentCreateTransaction); // If a 'create + give initial name' transaction is ongoing, take responsibility of ending it until the selected item is scrolled into view.

	TreeView->RequestScrollIntoView(SelectedItems[0]);

	if (DeferredOngoingCreateTransaction.IsValid() && !PostTickHandle.IsValid())
	{
		// Ensure the item will be scrolled into view during the frame (See explanation in OnPostTick()).
		PostTickHandle = FSlateApplication::Get().OnPostTick().AddSP(this, &SSpriteAtlasPackerSpriteList::OnPostTick);
	}
}

void SSpriteAtlasPackerSpriteList::OnRenameNode()
{
	OnRenameNode(nullptr); // null means that the rename is not part of the creation process (create + give initial name).
}

void SSpriteAtlasPackerSpriteList::OnPostTick(float)
{
	// If a 'create + give initial name' is ongoing and the transaction ownership was not transferred during the frame it was requested, it is most likely because the newly
	// created item could not be scrolled into view (should say 'teleported', the scrolling is not animated). The tree view will not put the item in view if the there is
	// no space left to display the item. (ex a splitter where all the display space is used by the other component). End the transaction before starting a new frame. The user
	// will not be able to rename on creation, the widget is likely not in view and cannot be edited anyway.
	DeferredOngoingCreateTransaction.Reset();

	// The post tick event handler is not required anymore.
	FSlateApplication::Get().OnPostTick().Remove(PostTickHandle);
	PostTickHandle.Reset();
}

bool SSpriteAtlasPackerSpriteList::CanRenameNode() const
{
	const auto& SelectedItems = TreeView->GetSelectedItems();
	return SelectedItems.Num() == 1;
}

EVisibility SSpriteAtlasPackerSpriteList::OnEmptyTipsVisibility() const
{
	if (FSpriteAtlasPacker::Get().GetRootSprites().Num() == 0)
		return EVisibility::HitTestInvisible;
	return EVisibility::Collapsed;
}

void SSpriteAtlasPackerSpriteList::OnItemScrolledIntoView(TSharedPtr<FSpriteItem> InItem, const TSharedPtr<ITableRow>& InWidget)
{
	if(!DeferredRenameRequest.IsEmpty())
	{
		const FString ItemName = InItem->Name;
		if(DeferredRenameRequest.Equals(ItemName))
		{
			DeferredRenameRequest.Empty();
			InItem->OnRequestRename(MoveTemp(DeferredOngoingCreateTransaction)); // Transfer responsibility to end the 'create + give initial name' transaction to the tree item if such transaction is ongoing.
		}
	}
}

#undef LOCTEXT_NAMESPACE
