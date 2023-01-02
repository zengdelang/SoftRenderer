#include "SCharsetsTabWidget.h"

#include "Factories.h"
#include "SCharsetItemWidget.h"
#include "ToolMenus.h"
#include "Framework/Commands/GenericCommands.h"
#include "SDFFontEditor.h"
#include "UnrealExporter.h"
#include "Core/Widgets/Text/SDFFontCharset.h"
#include "Exporters/Exporter.h"
#include "HAL/PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "SCharsetsTabWidget"

static const FName ColumnName_CharsetRoot( "CharsetRoot" );
static const FName Charset_ContextMenuName( "CharsetItemContextMenu" );

// Text object factory for pasting components
struct FCharsetObjectTextFactory : public FCustomizableTextObjectFactory
{
	TArray<USDFFontCharset*> NewObjectList;

	virtual ~FCharsetObjectTextFactory() {}

	// Constructs a new object factory from the given text buffer
	static TSharedRef<FCharsetObjectTextFactory> Get(const FString& InTextBuffer, bool bPasteAsArchetypes = true)
	{
		// Construct a new instance
		TSharedPtr<FCharsetObjectTextFactory> FactoryPtr = MakeShareable(new FCharsetObjectTextFactory());
		check(FactoryPtr.IsValid());

		// Create new objects if we're allowed to
		if (FactoryPtr->CanCreateObjectsFromText(InTextBuffer))
		{
			EObjectFlags ObjectFlags = RF_Transactional;
			if (bPasteAsArchetypes)
			{
				ObjectFlags |= RF_ArchetypeObject | RF_Public;
			}

			// Use the transient package initially for creating the objects, since the variable name is used when copying
			FactoryPtr->ProcessBuffer(GetTransientPackage(), ObjectFlags, InTextBuffer);
		}

		return FactoryPtr.ToSharedRef();
	}

protected:
	// Constructor; protected to only allow this class to instance itself
	FCharsetObjectTextFactory()
		: FCustomizableTextObjectFactory(GWarn)
	{
	}

	// FCustomizableTextObjectFactory implementation

	virtual bool CanCreateClass(UClass* ObjectClass, bool& bOmitSubObjs) const override
	{
		// Allow actor component types to be created
		bool bCanCreate = ObjectClass->IsChildOf(USDFFontCharset::StaticClass());

		return bCanCreate;
	}

	virtual void ProcessConstructedObject(UObject* NewObject) override
	{
		check(NewObject);
		
		if (USDFFontCharset* NewFontCharset = Cast<USDFFontCharset>(NewObject))
		{
			NewObjectList.Add(NewFontCharset);
		}
	}

	// FCustomizableTextObjectFactory (end)
};


SCharsetsTabWidget::~SCharsetsTabWidget()
{
		
}

void SCharsetsTabWidget::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<FSDFFontEditor> InSDFFontEditor)
{
	SDFFontEditor = InSDFFontEditor;
	CommandList = InCommandList;

	CommandList->MapAction( FGenericCommands::Get().Cut,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::CutSelectedNodes ), 
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanCutNodes ) ) 
		);
	
	CommandList->MapAction( FGenericCommands::Get().Copy,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::CopySelectedNodes ), 
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanCopyNodes ) ) 
		);
	
	CommandList->MapAction( FGenericCommands::Get().Paste,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::PasteNodes ), 
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanPasteNodes ) ) 
		);
	
	CommandList->MapAction( FGenericCommands::Get().Duplicate,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::OnDuplicateComponent ), 
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanDuplicateComponent ) ) 
		);

	CommandList->MapAction( FGenericCommands::Get().Delete,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::OnDeleteNodes ), 
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanDeleteNodes ) ) 
		);

	CommandList->MapAction( FGenericCommands::Get().Rename,
		FUIAction( FExecuteAction::CreateSP( this, &SCharsetsTabWidget::OnRenameNode),
		FCanExecuteAction::CreateSP( this, &SCharsetsTabWidget::CanRenameNode ) ) 
		);
	
	TSharedPtr<SHeaderRow> HeaderRow = SNew(SHeaderRow)
	+ SHeaderRow::Column(ColumnName_CharsetRoot)
	.DefaultLabel(LOCTEXT("CharsetRoot", "CharsetRoot"))
	.FillWidth(4);
	
	// Create the tree view control
	TreeView =
		SNew( STreeView<TSharedPtr<FCharsetItem>> )
		.TreeItemsSource(&CharsetItems)
		.SelectionMode(ESelectionMode::Multi)
		.ClearSelectionOnClick(true)
		.OnItemScrolledIntoView(this, &SCharsetsTabWidget::OnItemScrolledIntoView)
		.OnGenerateRow( this, &SCharsetsTabWidget::CharsetsTreeView_OnGenerateRow ) 
		.OnGetChildren( this, &SCharsetsTabWidget::CharsetsTreeView_OnGetChildren )
		.OnContextMenuOpening(this, &SCharsetsTabWidget::CreateContextMenu)
		.OnSelectionChanged( this, &SCharsetsTabWidget::CharsetsTreeView_OnSelectionChanged )
		.ItemHeight(24)
		.HeaderRow
		(
			HeaderRow
		);

	TreeView->GetHeaderRow()->SetVisibility(EVisibility::Collapsed);

	RebuildCharsetTree();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			[				
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.HAlign(HAlign_Center)
				.OnClicked(this, &SCharsetsTabWidget::HandleAddCharset)
				.ForegroundColor(FSlateColor::UseForeground())
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.AutoWidth()
					.Padding(1.f,1.f)
					[
						SNew(STextBlock)
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
						.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.10"))
						.Text(FText::FromString(FString(TEXT("\xf067"))) /*fa-plus*/)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(1.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AddCharsetButtonLabel", "Add Charset"))
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					]
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.Padding(2, 0, 0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
				.OnClicked(this, &SCharsetsTabWidget::HandleAddDefaultCharsets)
				.ForegroundColor(FSlateColor::UseForeground())
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.Padding(1.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("DefaultCharsetsButtonLabel", "Default Charsets"))
						.TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(2, 0)
		[
			SNew(SBox)
			.Padding(1.f)
			[
				SNew(SBorder)
				.Padding(FEditorStyle::GetMargin(TEXT("Menu.Separator.Padding")))
				.BorderImage(FEditorStyle::GetBrush(TEXT("Menu.Separator")))
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1)
		.Padding(2)
		[
			TreeView.ToSharedRef()
		]
	];
}

FReply SCharsetsTabWidget::OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent )
{
	if (CommandList->ProcessCommandBindings(InKeyEvent))
	{
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SCharsetsTabWidget::PostUndo(bool bSuccess)
{
	RebuildCharsetTree();
}

TSharedRef<ITableRow> SCharsetsTabWidget::CharsetsTreeView_OnGenerateRow(TSharedPtr<FCharsetItem> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SCharsetItemWidget, OwnerTable, Item.ToSharedRef(), SharedThis(this));
}

void SCharsetsTabWidget::CharsetsTreeView_OnGetChildren(TSharedPtr<FCharsetItem> Item, TArray<TSharedPtr<FCharsetItem>>& OutChildren)
{
	
}

void SCharsetsTabWidget::CharsetsTreeView_OnSelectionChanged(TSharedPtr<FCharsetItem> Item, ESelectInfo::Type SelectInfo)
{
	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			const auto& SelectedItems = TreeView->GetSelectedItems();
			if (SelectedItems.Num() > 0)
			{
				const auto& TargetItem = SelectedItems[0];
				SDFFontEditor.Pin()->GetDetailsView()->SetObject(TargetItem->FontCharset);
			}
			else
			{
				SDFFontEditor.Pin()->GetDetailsView()->SetObject(Font);
			}
		}
	}
}

void SCharsetsTabWidget::RebuildCharsetTree()
{
	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			CharsetItems.Empty();
			for (const auto& FontCharsetObj : Font->Charsets)
			{
				USDFFontCharset* FontCharset = Cast<USDFFontCharset>(FontCharsetObj);
				if (FontCharset)
				{
					CharsetItems.Emplace(MakeShareable(new FCharsetItem(FontCharset)));
				}
			}
		}
	}
	
	// Refresh the view
	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();
	}
}

TArray<TSharedPtr<FCharsetItem>> SCharsetsTabWidget::GetSelectedItems()
{
	return TreeView->GetSelectedItems();
}

void SCharsetsTabWidget::ReparentItems(const TArray<TSharedPtr<FCharsetItem>>& Items,
	TSharedPtr<FCharsetItem> TargetItem, EItemDropZone DropZone)
{
	const FScopedTransaction Transaction(LOCTEXT("ReparentItems", "Reparent Item(s)"));
	
	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			Font->Modify();

			USDFFontCharset* TargetItemObj = nullptr;

			TArray<TSharedPtr<FCharsetItem>> NewItems;
			for (int32 Index = 0, Count = CharsetItems.Num(); Index < Count; ++Index)
			{
				if (CharsetItems[Index] == TargetItem)
				{
					break;
				}

				if (!Items.Contains(CharsetItems[Index]))
				{
					NewItems.Add(CharsetItems[Index]);
				}
			}

			if (!Items.Contains(TargetItem) && DropZone != EItemDropZone::AboveItem)
			{
				NewItems.Add(TargetItem);
			}

			NewItems.Append(Items);

			for (int32 Index = 0, Count = CharsetItems.Num(); Index < Count; ++Index)
			{
				if (!NewItems.Contains(CharsetItems[Index]))
				{
					NewItems.Add(CharsetItems[Index]);
				}
			}

			Font->Charsets.Empty();

			for (const auto& Item : NewItems)
			{
				Font->Charsets.Add(Item->FontCharset);
			}
		}

		RebuildCharsetTree();
		
		TreeView->ClearSelection();
		TreeView->SetItemSelection(Items, true);
	}
}

TSharedPtr<SWidget> SCharsetsTabWidget::CreateContextMenu()
{
	const auto& SelectedItems = TreeView->GetSelectedItems();

	if (SelectedItems.Num() > 0)
	{
		RegisterContextMenu();

		FToolMenuContext ToolMenuContext(CommandList, TSharedPtr<FExtender>());
		return UToolMenus::Get()->GenerateWidget(Charset_ContextMenuName, ToolMenuContext);
	}
	return TSharedPtr<SWidget>();
}

void SCharsetsTabWidget::RegisterContextMenu()
{
	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus->IsMenuRegistered(Charset_ContextMenuName))
	{
		UToolMenu* Menu = ToolMenus->RegisterMenu(Charset_ContextMenuName);
		Menu->AddDynamicSection("SCSEditorDynamic", FNewToolMenuDelegate::CreateLambda([&](UToolMenu* InMenu)
		{
			FToolMenuSection& Section = InMenu->AddSection("CharsetListItem", LOCTEXT("CharsetListItemHeading", "Edit"));
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Paste);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Rename);
		}));
	}
}

void SCharsetsTabWidget::CutSelectedNodes()
{
	const FScopedTransaction Transaction( TreeView->GetSelectedItems().Num() > 1 ? LOCTEXT("CutCharsets", "Cut Charsets") : LOCTEXT("CutCharset", "Cut Charset") );
	
	CopySelectedNodes();

	//OnDeleteNodes();
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			Font->Modify();
		
			const auto& SelectedItems = TreeView->GetSelectedItems();
			for (auto& Elem : SelectedItems)
			{
				Font->Charsets.Remove(Elem->FontCharset);
			}
		}

		RebuildCharsetTree();
	}
}

bool SCharsetsTabWidget::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteNodes();
}

void SCharsetsTabWidget::CopySelectedNodes()
{
	TArray<USDFFontCharset*> Charsets;
	for (const auto& Elem : TreeView->GetSelectedItems())
	{
		if (Elem.IsValid() && Elem->FontCharset)
		{
			Charsets.Add(Elem->FontCharset);
		}
	}

	FStringOutputDevice Archive;

	// Clear the mark state for saving.
	UnMarkAllObjects(EObjectMark(OBJECTMARK_TagExp | OBJECTMARK_TagImp));
	
	const FExportObjectInnerContext Context;
	
	for (const auto& Charset : Charsets)
	{
		check(Charset);
		
		UExporter::ExportToOutputDevice(&Context, Charset, nullptr, Archive, TEXT("copy"), 0, PPF_ExportsNotFullyQualified | PPF_Copy | PPF_Delimited, false, SDFFontEditor.IsValid() ? SDFFontEditor.Pin()->GetSDFFont() : nullptr);
	}

	// Copy text to clipboard
	FString ExportedText = Archive;
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool SCharsetsTabWidget::CanCopyNodes() const
{
	return true;
}

void SCharsetsTabWidget::PasteNodes()
{
	const FScopedTransaction Transaction(LOCTEXT("PasteItems", "Paste Item(s)"));

	// Get the text from the clipboard
	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);
	
	TSharedRef<FCharsetObjectTextFactory> Factory = FCharsetObjectTextFactory::Get(TextToImport);

	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			Font->Modify();

			for (const auto& Elem : Factory->NewObjectList)
			{
				Font->Charsets.Add(Elem);
			}
		}

		RebuildCharsetTree();
		
		TreeView->ClearSelection();

		TArray<TSharedPtr<FCharsetItem>> SelectedItems;
		for (int32 Index = Factory->NewObjectList.Num() - 1; Index >= 0; --Index)
		{
			if (CharsetItems.IsValidIndex(Index))
			{
				SelectedItems.Add(CharsetItems[CharsetItems.Num() - (Factory->NewObjectList.Num() - Index)]);
			}
		}
		
		TreeView->SetItemSelection(SelectedItems, true);

		if (SelectedItems.Num() == 1)
		{
			OnRenameNode(nullptr); 
		}
	}
}

bool SCharsetsTabWidget::CanPasteNodes() const
{
	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);
	
	TSharedRef<FCharsetObjectTextFactory> Factory = FCharsetObjectTextFactory::Get(ClipboardContent);
	return Factory->NewObjectList.Num() > 0;
}

bool SCharsetsTabWidget::CanDuplicateComponent() const
{
	return CanCopyNodes();
}

void SCharsetsTabWidget::OnDuplicateComponent()
{
	CopySelectedNodes();
	PasteNodes();
}

void SCharsetsTabWidget::OnDeleteNodes()
{
	USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
	if (Font)
	{
		FScopedTransaction TransactionContext(LOCTEXT("SDFFontEditor_DeleteCharset", "Delete charset(s)"));
			
		Font->Modify();
		
		const auto& SelectedItems = TreeView->GetSelectedItems();
		for (auto& Elem : SelectedItems)
		{
			Font->Charsets.Remove(Elem->FontCharset);
		}
	}

	RebuildCharsetTree();
}

bool SCharsetsTabWidget::CanDeleteNodes() const
{
	return true;
}

void SCharsetsTabWidget::OnRenameNode(TUniquePtr<FScopedTransaction> InComponentCreateTransaction)
{
	auto SelectedItems = TreeView->GetSelectedItems();

	// Should already be prevented from making it here.
	check(SelectedItems.Num() == 1);

	DeferredRenameRequest = SelectedItems[0]->GetCharsetName();

	check(!DeferredOngoingCreateTransaction.IsValid()); // If this fails, something in the chain of responsibility failed to end the previous transaction.
	DeferredOngoingCreateTransaction = MoveTemp(InComponentCreateTransaction); // If a 'create + give initial name' transaction is ongoing, take responsibility of ending it until the selected item is scrolled into view.

	TreeView->RequestScrollIntoView(SelectedItems[0]);

	if (DeferredOngoingCreateTransaction.IsValid() && !PostTickHandle.IsValid())
	{
		// Ensure the item will be scrolled into view during the frame (See explanation in OnPostTick()).
		PostTickHandle = FSlateApplication::Get().OnPostTick().AddSP(this, &SCharsetsTabWidget::OnPostTick);
	}
}

void SCharsetsTabWidget::OnRenameNode()
{
	OnRenameNode(nullptr); // null means that the rename is not part of the creation process (create + give initial name).
}

void SCharsetsTabWidget::OnPostTick(float)
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

bool SCharsetsTabWidget::CanRenameNode() const
{
	const auto& SelectedItems = TreeView->GetSelectedItems();
	return SelectedItems.Num() == 1;
}

void SCharsetsTabWidget::OnItemScrolledIntoView(TSharedPtr<FCharsetItem> InItem, const TSharedPtr<ITableRow>& InWidget)
{
	if (!DeferredRenameRequest.IsEmpty())
	{
		const FString ItemName = InItem->GetCharsetName();
		if(DeferredRenameRequest.Equals(ItemName))
		{
			DeferredRenameRequest.Empty();
			InItem->OnRequestRename(MoveTemp(DeferredOngoingCreateTransaction)); // Transfer responsibility to end the 'create + give initial name' transaction to the tree item if such transaction is ongoing.
		}
	}
}

FReply SCharsetsTabWidget::HandleAddCharset()
{
	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			FScopedTransaction TransactionContext(LOCTEXT("SDFFontEditor_AddCharset", "Add new charset"));
			
			Font->Modify();
			
			UObject* UseOuter = Font;
			EObjectFlags	MaskedOuterFlags = UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : RF_NoFlags;
			if (UseOuter && UseOuter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				MaskedOuterFlags |= RF_ArchetypeObject;
			}

			MaskedOuterFlags |= RF_Public | RF_Transactional | RF_ArchetypeObject;
	
			USDFFontCharset* NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			Font->Charsets.Add(NewObj);
		}

		RebuildCharsetTree();
		
		TreeView->ClearSelection();
		TreeView->SetItemSelection(CharsetItems[CharsetItems.Num() - 1], true);
		OnRenameNode(nullptr); 
	}
	return FReply::Handled();
}

FReply SCharsetsTabWidget::HandleAddDefaultCharsets()
{
	if (SDFFontEditor.IsValid())
	{
		USDFFont* Font = SDFFontEditor.Pin()->GetSDFFont();
		if (Font)
		{
			FScopedTransaction TransactionContext(LOCTEXT("SDFFontEditor_AddDefaultCharsets", "Add Default Charsets"));
			
			Font->Modify();
			
			UObject* UseOuter = Font;
			EObjectFlags	MaskedOuterFlags = UseOuter ? UseOuter->GetMaskedFlags(RF_PropagateToSubObjects) : RF_NoFlags;
			if (UseOuter && UseOuter->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
			{
				MaskedOuterFlags |= RF_ArchetypeObject;
			}

			MaskedOuterFlags |= RF_Public | RF_Transactional | RF_ArchetypeObject;
			
			USDFFontCharset* NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Numbers");
			NewObj->Charset = TEXT("\"0123456789\"");
			NewObj->FontScale = 129;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("ASCII");
			NewObj->Charset = TEXT("[0x00, 0x100]");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Chinese phonetic alphabet");
			NewObj->Charset = TEXT("\"﻿āáǎàōóǒòêēéěèīíǐìūúǔùǖǘǚǜü\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Currency Sign");
			NewObj->Charset = TEXT("\"¤฿฿BsBr₵¢ ₡ ₫€ƒFtRs.₲₭kr£₤Lm₥₦ ₱PQRSkRp৲৳R$S/.$ 〒₮₩¥NT$￥zł₴₪៛﷼рубRM\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Greek alphabet");
			NewObj->Charset = TEXT("\"αβγδεζηθικλμνξοπρστυφχψωΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Math symbols");
			NewObj->Charset = TEXT("\"﻿≈≡≠＝≤≥＜＞≮≯∷±＋－×÷／∫∮∝∞∧∨∑∏∪∩∈∵∴⊥∥∠⌒⊙≌∽√\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Roman numbers");
			NewObj->Charset = TEXT("\"ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩⅪⅫLCDM\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Sequence Number");
			NewObj->Charset = TEXT("\"①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳⒈⒉⒊⒋⒌⒍⒎⒏⒐⒑⒒⒓⒔⒕⒖⒗⒘⒙⒚⒛⑴⑵⑶⑷⑸⑹⑺⑻⑼⑽⑾⑿⒀⒁⒂⒃⒄⒅⒆⒇❶❷❸❹❺❻❼❽❾❿⓫⓬⓭⓮⓯⓰⓱⓲⓳⓴ⒶⒷⒸⒹⒺⒻⒼⒽⒾⒿⓀⓁⓂⓃⓄⓅⓆⓇⓈⓉⓊⓋⓌⓍⓎⓏⓐⓑⓒⓓⓔⓕⓖⓗⓘⓙⓚⓛⓜⓝⓞⓟⓠⓡⓢⓣⓤⓥⓦⓧⓨⓩ⒜⒝⒞⒟⒠⒡⒢⒣⒤⒥⒦⒧⒨⒩⒪⒫⒬⒭⒮⒯⒰⒱⒲⒳⒴⒵№㈠㈡㈢㈣㈤㈥㈦㈧㈨㈩零壹贰叁肆伍陆柒捌玖拾佰仟\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Punctuation");
			NewObj->Charset = TEXT("\"。，、；：？！…—·ˉ¨‘’“”々～‖∶＂＇｀｜〃〔〕〈〉《》「」『』．〖〗【】（）［］｛｝,./?<>;:'\\\"\\|\\[\\]{}·﹒`~!@#$%^&*()_1234567890℃－γ¿¡„£。；，：‘’“”（）、？《》！——……–．·【】■o□ë \"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("ar");
			NewObj->Charset = TEXT("\"ا‎ﺎ‎ﺎ‎ا‎ﺏ‎ـب‎ـبـ‎بـ‎ﺕ‎ـت‎ـتـ‎تـ‎ﺙ‎ـث‎ـثـ‎ثـ‎ﺝ‎ـج‎ـجـ‎جـ‎ﺡ‎ـح‎ـحـ‎حـ‎ﺥ‎ـخ‎ـخـ‎خـ‎ﺩ‎ـد‎ـد‎د‎ﺫ‎ـذ‎ـذ‎ذ‎ﺭ‎ـر‎ـر‎ر‎ﺯ‎ـز‎ـز‎ز‎ﺱ‎ـس‎ـسـ‎سـ‎ﺵ‎ـش‎ـشـ‎شـ‎ﺹ‎ـص‎ـصـ‎صـ‎ﺽ‎ـض‎ـضـ‎ضـ‎ﻁ‎ـط‎ـطـ‎طـ‎ﻅ‎ـظ‎ـظـ‎ظـ‎ﻉ‎ـع‎ـعـ‎عـ‎ﻍ‎ـغ‎ـغـ‎غـ‎ف‎ـف‎ـفـ‎فـ‎ﻕ‎ـق‎ـقـ‎قـ‎ﻙ‎ـك‎ـكـ‎كـ‎ﻝ‎ـل‎ـلـ‎لـ‎ﻡ‎ـم‎ـمـ‎مـ‎ن‎ـن‎ـنـ‎نـ‎ﻩ‎ـه‎ـهـ‎هـ‎ﻭ‎ـو‎ـو‎و‎ﻱ‎ـي‎ـيـ‎يـ‎\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("de");
			NewObj->Charset = TEXT("\"ABCDEFGHIJKLMNOPQRSTUVWXYZäöüßabcdefghijklmnopqrstuvwxyz\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("en");
			NewObj->Charset = TEXT("\"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("es");
			NewObj->Charset = TEXT("\"AaBbCcDdEeFfGgHhIiJjKkLlMmNnÑñOoPpQqRrSsTtUuVvWwXxYyZz\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("fr");
			NewObj->Charset = TEXT("\"aàâbcçdeèéêëfghî ïjklmnoôöpqrstuùûüvwxyzœæAÀÂBCÇDEÈÉÊËFGHÎÏJKLMNOÔÖPQRSTUÙÛVWXYZŒÆ\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("ja");
			NewObj->Charset = TEXT("\"ぁぃぅぇぉかきくけこんさしすせそたちつってとゐなにぬねのはひふへほゑまみむめもゃゅょゎをあいうえおがぎぐげござじずぜぞだぢづでどぱぴぷぺぽばびぶべぼらりるれろやゆよわァィゥヴェォカヵキクケコサシスセソタチツッテトヰンナニヌネノハヒフヘホヱマミムメモャュョヮヲアイウエオガギグゲゴザジズゼゾダヂヅデドパピプペポバビブベボラリルレロヤユヨワ\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
			
			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("ko");
			NewObj->Charset = TEXT("\"ㅏㅑㅓㅕㅗㅛㅜㅠㅡㅣㄱ가갸거겨고교구규그기ㄴ나냐너녀노뇨누뉴느니ㄷ다댜더뎌도됴두듀드디ㄹ라랴러려로료루류르리ㅁ마먀머며모묘무뮤므미ㅂ바뱌버벼보뵤부뷰브비ㅅ사샤서셔소쇼수슈스시ㅇ아야어여오요우유으이ㅈ자쟈저져조죠주쥬즈지ㅊ차챠처쳐초쵸추츄츠치ㅋ카캬커켜코쿄쿠큐크키ㅌ타탸터텨토툐투튜트티ㅍ파퍄퍼펴포표푸퓨프피ㅎ하햐허혀호효후휴흐히ㄲ까꺄꺼껴꼬꾜꾸뀨끄끼ㄸ따땨떠뗘또뚀뚜뜌뜨띠ㅃ빠뺘뻐뼈뽀뾰뿌쀼쁘삐ㅆ싸쌰써쎠쏘쑈쑤쓔쓰씨ㅉ짜쨔쩌쪄쪼쬬쭈쮸쯔찌ㅐㅒㅔㅖㅘㅙㅚㅝㅞㅟㅢ개걔게계과괘괴궈궤귀긔내냬네녜놔놰뇌눠눼뉘늬대댸데뎨돠돼되둬뒈뒤듸래럐레례롸뢔뢰뤄뤠뤼릐매먜메몌뫄뫠뫼뭐뭬뮈믜배뱨베볘봐봬뵈붜붸뷔븨새섀세셰솨쇄쇠숴쉐쉬싀애얘에예와왜외워웨위의재쟤제졔좌좨죄줘줴쥐즤채챼체쳬촤쵀최춰췌취츼캐컈케켸콰쾌쾨쿼퀘퀴킈태턔테톄톼퇘퇴퉈퉤튀틔패퍠페폐퐈퐤푀풔풰퓌픠해햬헤혜화홰회훠훼휘희깨꺠께꼐꽈꽤꾀꿔꿰뀌끠때떄떼뗴똬뙈뙤뚸뛔뛰띄빼뺴뻬뼤뽜뽸뾔뿨쀄쀠쁴쌔썌쎄쎼쏴쐐쐬쒀쒜쒸씌째쨰쩨쪠쫘쫴쬐쭤쮀쮜쯰각간갇갈감갑강낙난낟날남납낭닥단닫달담답당락란랃랄람랍랑막만맏말맘맙망박반받발밤밥방삭산삳살삼삽상악안앋알암압앙작잔잗잘잠잡장착찬찯찰참찹창칵칸칻칼캄캅캉탁탄탇탈탐탑탕팍판팓팔팜팝팡학한핟할함합항\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("ru");
			NewObj->Charset = TEXT("\"АаБбВвГ гД дЕ еЁ ёЖжЗзИиЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЪъЫыЬьЭэЮюЯядзджчжсчтшчшзж\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Chinese - GB first-level character database");
			NewObj->Charset = TEXT("\"啊阿埃挨哎唉哀皑癌蔼矮艾碍爱隘鞍氨安俺按暗岸胺案肮昂盎凹敖熬翱袄傲奥懊澳芭捌扒叭吧笆八疤巴拔跋靶把耙坝霸罢爸白柏百摆佰败拜稗斑班搬扳般颁板版扮拌伴瓣半办绊邦帮梆榜膀绑棒磅蚌镑傍谤苞胞包褒剥薄雹保堡饱宝抱报暴豹鲍爆杯碑悲卑北辈背贝钡倍狈备惫焙被奔苯本笨崩绷甭泵蹦迸逼鼻比鄙笔彼碧蓖蔽毕毙毖币庇痹闭敝弊必辟壁臂避陛鞭边编贬扁便变卞辨辩辫遍标彪膘表鳖憋别瘪彬斌濒滨宾摈兵冰柄丙秉饼炳病并玻菠播拨钵波博勃搏铂箔伯帛舶脖膊渤泊驳捕卜哺补埠不布步簿部怖擦猜裁材才财睬踩采彩菜蔡餐参蚕残惭惨灿苍舱仓沧藏操糙槽曹草厕策侧册测层蹭插叉茬茶查碴搽察岔差诧拆柴豺搀掺蝉馋谗缠铲产阐颤昌猖场尝常长偿肠厂敞畅唱倡超抄钞朝嘲潮巢吵炒车扯撤掣彻澈郴臣辰尘晨忱沉陈趁衬撑称城橙成呈乘程惩澄诚承逞骋秤吃痴持匙池迟弛驰耻齿侈尺赤翅斥炽充冲虫崇宠抽酬畴踌稠愁筹仇绸瞅丑臭初出橱厨躇锄雏滁除楚础储矗搐触处揣川穿椽传船喘串疮窗幢床闯创吹炊捶锤垂春椿醇唇淳纯蠢戳绰疵茨磁雌辞慈瓷词此刺赐次聪葱囱匆从丛凑粗醋簇促蹿篡窜摧崔催脆瘁粹淬翠村存寸磋撮搓措挫错搭达答瘩打大呆歹傣戴带殆代贷袋待逮怠耽担丹单郸掸胆旦氮但惮淡诞弹蛋当挡党荡档刀捣蹈倒岛祷导到稻悼道盗德得的蹬灯登等瞪凳邓堤低滴迪敌笛狄涤翟嫡抵底地蒂第帝弟递缔颠掂滇碘点典靛垫电佃甸店惦奠淀殿碉叼雕凋刁掉吊钓调跌爹碟蝶迭谍叠丁盯叮钉顶鼎锭定订丢东冬董懂动栋侗恫冻洞兜抖斗陡豆逗痘都督毒犊独读堵睹赌杜镀肚度渡妒端短锻段断缎堆兑队对墩吨蹲敦顿囤钝盾遁掇哆多夺垛躲朵跺舵剁惰堕蛾峨鹅俄额讹娥恶厄扼遏鄂饿恩而儿耳尔饵洱二贰发罚筏伐乏阀法珐藩帆番翻樊矾钒繁凡烦反返范贩犯饭泛坊芳方肪房防妨仿访纺放菲非啡飞肥匪诽吠肺废沸费芬酚吩氛分纷坟焚汾粉奋份忿愤粪丰封枫蜂峰锋风疯烽逢冯缝讽奉凤佛否夫敷肤孵扶拂辐幅氟符伏俘服浮涪福袱弗甫抚辅俯釜斧脯腑府腐赴副覆赋复傅付阜父腹负富讣附妇缚咐噶嘎该改概钙盖溉干甘杆柑竿肝赶感秆敢赣冈刚钢缸肛纲岗港杠篙皋高膏羔糕搞镐稿告哥歌搁戈鸽胳疙割革葛格蛤阁隔铬个各给根跟耕更庚羹埂耿梗工攻功恭龚供躬公宫弓巩汞拱贡共钩勾沟苟狗垢构购够辜菇咕箍估沽孤姑鼓古蛊骨谷股故顾固雇刮瓜剐寡挂褂乖拐怪棺关官冠观管馆罐惯灌贯光广逛瑰规圭硅归龟闺轨鬼诡癸桂柜跪贵刽辊滚棍锅郭国果裹过哈骸孩海氦亥害骇酣憨邯韩含涵寒函喊罕翰撼捍旱憾悍焊汗汉夯杭航壕嚎豪毫郝好耗号浩呵喝荷菏核禾和何合盒貉阂河涸赫褐鹤贺嘿黑痕很狠恨哼亨横衡恒轰哄烘虹鸿洪宏弘红喉侯猴吼厚候后呼乎忽瑚壶葫胡蝴狐糊湖弧虎唬护互沪户花哗华猾滑画划化话槐徊怀淮坏欢环桓还缓换患唤痪豢焕涣宦幻荒慌黄磺蝗簧皇凰惶煌晃幌恍谎灰挥辉徽恢蛔回毁悔慧卉惠晦贿秽会烩汇讳诲绘荤昏婚魂浑混豁活伙火获或惑霍货祸击圾基机畸稽积箕肌饥迹激讥鸡姬绩缉吉极棘辑籍集及急疾汲即嫉级挤几脊己蓟技冀季伎祭剂悸济寄寂计记既忌际妓继纪嘉枷夹佳家加荚颊贾甲钾假稼价架驾嫁歼监坚尖笺间煎兼肩艰奸缄茧检柬碱硷拣捡简俭剪减荐槛鉴践贱见键箭件健舰剑饯渐溅涧建僵姜将浆江疆蒋桨奖讲匠酱降蕉椒礁焦胶交郊浇骄娇嚼搅铰矫侥脚狡角饺缴绞剿教酵轿较叫窖揭接皆秸街阶截劫节桔杰捷睫竭洁结解姐戒藉芥界借介疥诫届巾筋斤金今津襟紧锦仅谨进靳晋禁近烬浸尽劲荆兢茎睛晶鲸京惊精粳经井警景颈静境敬镜径痉靖竟竞净炯窘揪究纠玖韭久灸九酒厩救旧臼舅咎就疚鞠拘狙疽居驹菊局咀矩举沮聚拒据巨具距踞锯俱句惧炬剧捐鹃娟倦眷卷绢撅攫抉掘倔爵觉决诀绝均菌钧军君峻俊竣浚郡骏喀咖卡咯开揩楷凯慨刊堪勘坎砍看康慷糠扛抗亢炕考拷烤靠坷苛柯棵磕颗科壳咳可渴克刻客课肯啃垦恳坑吭空恐孔控抠口扣寇枯哭窟苦酷库裤夸垮挎跨胯块筷侩快宽款匡筐狂框矿眶旷况亏盔岿窥葵奎魁傀馈愧溃坤昆捆困括扩廓阔垃拉喇蜡腊辣啦莱来赖蓝婪栏拦篮阑兰澜谰揽览懒缆烂滥琅榔狼廊郎朗浪捞劳牢老佬姥酪烙涝勒乐雷镭蕾磊累儡垒擂肋类泪棱楞冷厘梨犁黎篱狸离漓理李里鲤礼莉荔吏栗丽厉励砾历利傈例俐痢立粒沥隶力璃哩俩联莲连镰廉怜涟帘敛脸链恋炼练粮凉梁粱良两辆量晾亮谅撩聊僚疗燎寥辽潦了撂镣廖料列裂烈劣猎琳林磷霖临邻鳞淋凛赁吝拎玲菱零龄铃伶羚凌灵陵岭领另令溜琉榴硫馏留刘瘤流柳六龙聋咙笼窿隆垄拢陇楼娄搂篓漏陋芦卢颅庐炉掳卤虏鲁麓碌露路赂鹿潞禄录陆戮驴吕铝侣旅履屡缕虑氯律率滤绿峦挛孪滦卵乱掠略抡轮伦仑沦纶论萝螺罗逻锣箩骡裸落洛骆络妈麻玛码蚂马骂嘛吗埋买麦卖迈脉瞒馒蛮满蔓曼慢漫谩芒茫盲氓忙莽猫茅锚毛矛铆卯茂冒帽貌贸么玫枚梅酶霉煤没眉媒镁每美昧寐妹媚门闷们萌蒙檬盟锰猛梦孟眯醚靡糜迷谜弥米秘觅泌蜜密幂棉眠绵冕免勉娩缅面苗描瞄藐秒渺庙妙蔑灭民抿皿敏悯闽明螟鸣铭名命谬摸摹蘑模膜磨摩魔抹末莫墨默沫漠寞陌谋牟某拇牡亩姆母墓暮幕募慕木目睦牧穆拿哪呐钠那娜纳氖乃奶耐奈南男难囊挠脑恼闹淖呢馁内嫩能妮霓倪泥尼拟你匿腻逆溺蔫拈年碾撵捻念娘酿鸟尿捏聂孽啮镊镍涅您柠狞凝宁拧泞牛扭钮纽脓浓农弄奴努怒女暖虐疟挪懦糯诺哦欧鸥殴藕呕偶沤啪趴爬帕怕琶拍排牌徘湃派攀潘盘磐盼畔判叛乓庞旁耪胖抛咆刨炮袍跑泡呸胚培裴赔陪配佩沛喷盆砰抨烹澎彭蓬棚硼篷膨朋鹏捧碰坯砒霹批披劈琵毗啤脾疲皮匹痞僻屁譬篇偏片骗飘漂瓢票撇瞥拼频贫品聘乒坪苹萍平凭瓶评屏坡泼颇婆破魄迫粕剖扑铺仆莆葡菩蒲埔朴圃普浦谱曝瀑期欺栖戚妻七凄漆柒沏其棋奇歧畦崎脐齐旗祈祁骑起岂乞企启契砌器气迄弃汽泣讫掐恰洽牵扦钎铅千迁签仟谦乾黔钱钳前潜遣浅谴堑嵌欠歉枪呛腔羌墙蔷强抢橇锹敲悄桥瞧乔侨巧鞘撬翘峭俏窍切茄且怯窃钦侵亲秦琴勤芹擒禽寝沁青轻氢倾卿清擎晴氰情顷请庆琼穷秋丘邱球求囚酋泅趋区蛆曲躯屈驱渠取娶龋趣去圈颧权醛泉全痊拳犬券劝缺炔瘸却鹊榷确雀裙群然燃冉染瓤壤攘嚷让饶扰绕惹热壬仁人忍韧任认刃妊纫扔仍日戎茸蓉荣融熔溶容绒冗揉柔肉茹蠕儒孺如辱乳汝入褥软阮蕊瑞锐闰润若弱撒洒萨腮鳃塞赛三叁伞散桑嗓丧搔骚扫嫂瑟色涩森僧莎砂杀刹沙纱傻啥煞筛晒珊苫杉山删煽衫闪陕擅赡膳善汕扇缮墒伤商赏晌上尚裳梢捎稍烧芍勺韶少哨邵绍奢赊蛇舌舍赦摄射慑涉社设砷申呻伸身深娠绅神沈审婶甚肾慎渗声生甥牲升绳省盛剩胜圣师失狮施湿诗尸虱十石拾时什食蚀实识史矢使屎驶始式示士世柿事拭誓逝势是嗜噬适仕侍释饰氏市恃室视试收手首守寿授售受瘦兽蔬枢梳殊抒输叔舒淑疏书赎孰熟薯暑曙署蜀黍鼠属术述树束戍竖墅庶数漱恕刷耍摔衰甩帅栓拴霜双爽谁水睡税吮瞬顺舜说硕朔烁斯撕嘶思私司丝死肆寺嗣四伺似饲巳松耸怂颂送宋讼诵搜艘擞嗽苏酥俗素速粟僳塑溯宿诉肃酸蒜算虽隋随绥髓碎岁穗遂隧祟孙损笋蓑梭唆缩琐索锁所塌他它她塔獭挞蹋踏胎苔抬台泰酞太态汰坍摊贪瘫滩坛檀痰潭谭谈坦毯袒碳探叹炭汤塘搪堂棠膛唐糖倘躺淌趟烫掏涛滔绦萄桃逃淘陶讨套特藤腾疼誊梯剔踢锑提题蹄啼体替嚏惕涕剃屉天添填田甜恬舔腆挑条迢眺跳贴铁帖厅听烃汀廷停亭庭挺艇通桐酮瞳同铜彤童桶捅筒统痛偷投头透凸秃突图徒途涂屠土吐兔湍团推颓腿蜕褪退吞屯臀拖托脱鸵陀驮驼椭妥拓唾挖哇蛙洼娃瓦袜歪外豌弯湾玩顽丸烷完碗挽晚皖惋宛婉万腕汪王亡枉网往旺望忘妄威巍微危韦违桅围唯惟为潍维苇萎委伟伪尾纬未蔚味畏胃喂魏位渭谓尉慰卫瘟温蚊文闻纹吻稳紊问嗡翁瓮挝蜗涡窝我斡卧握沃巫呜钨乌污诬屋无芜梧吾吴毋武五捂午舞伍侮坞戊雾晤物勿务悟误昔熙析西硒矽晰嘻吸锡牺稀息希悉膝夕惜熄烯溪汐犀檄袭席习媳喜铣洗系隙戏细瞎虾匣霞辖暇峡侠狭下厦夏吓掀锨先仙鲜纤咸贤衔舷闲涎弦嫌显险现献县腺馅羡宪陷限线相厢镶香箱襄湘乡翔祥详想响享项巷橡像向象萧硝霄削哮嚣销消宵淆晓小孝校肖啸笑效楔些歇蝎鞋协挟携邪斜胁谐写械卸蟹懈泄泻谢屑薪芯锌欣辛新忻心信衅星腥猩惺兴刑型形邢行醒幸杏性姓兄凶胸匈汹雄熊休修羞朽嗅锈秀袖绣墟戌需虚嘘须徐许蓄酗叙旭序畜恤絮婿绪续轩喧宣悬旋玄选癣眩绚靴薛学穴雪血勋熏循旬询寻驯巡殉汛训讯逊迅压押鸦鸭呀丫芽牙蚜崖衙涯雅哑亚讶焉咽阉烟淹盐严研蜒岩延言颜阎炎沿奄掩眼衍演艳堰燕厌砚雁唁彦焰宴谚验殃央鸯秧杨扬佯疡羊洋阳氧仰痒养样漾邀腰妖瑶摇尧遥窑谣姚咬舀药要耀椰噎耶爷野冶也页掖业叶曳腋夜液一壹医揖铱依伊衣颐夷遗移仪胰疑沂宜姨彝椅蚁倚已乙矣以艺抑易邑屹亿役臆逸肄疫亦裔意毅忆义益溢诣议谊译异翼翌绎茵荫因殷音阴姻吟银淫寅饮尹引隐印英樱婴鹰应缨莹萤营荧蝇迎赢盈影颖硬映哟拥佣臃痈庸雍踊蛹咏泳涌永恿勇用幽优悠忧尤由邮铀犹油游酉有友右佑釉诱又幼迂淤于盂榆虞愚舆余俞逾鱼愉渝渔隅予娱雨与屿禹宇语羽玉域芋郁吁遇喻峪御愈欲狱育誉浴寓裕预豫驭鸳渊冤元垣袁原援辕园员圆猿源缘远苑愿怨院曰约越跃钥岳粤月悦阅耘云郧匀陨允运蕴酝晕韵孕匝砸杂栽哉灾宰载再在咱攒暂赞赃脏葬遭糟凿藻枣早澡蚤躁噪造皂灶燥责择则泽贼怎增憎曾赠扎喳渣札轧铡闸眨栅榨咋乍炸诈摘斋宅窄债寨瞻毡詹粘沾盏斩辗崭展蘸栈占战站湛绽樟章彰漳张掌涨杖丈帐账仗胀瘴障招昭找沼赵照罩兆肇召遮折哲蛰辙者锗蔗这浙珍斟真甄砧臻贞针侦枕疹诊震振镇阵蒸挣睁征狰争怔整拯正政帧症郑证芝枝支吱蜘知肢脂汁之织职直植殖执值侄址指止趾只旨纸志挚掷至致置帜峙制智秩稚质炙痔滞治窒中盅忠钟衷终种肿重仲众舟周州洲诌粥轴肘帚咒皱宙昼骤珠株蛛朱猪诸诛逐竹烛煮拄瞩嘱主著柱助蛀贮铸筑住注祝驻抓爪拽专砖转撰赚篆桩庄装妆撞壮状椎锥追赘坠缀谆准捉拙卓桌琢茁酌啄着灼浊兹咨资姿滋淄孜紫仔籽滓子自渍字鬃棕踪宗综总纵邹走奏揍租足卒族祖诅阻组钻纂嘴醉最罪尊遵昨左佐柞做作坐座\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Chinese - GB second-level character database");
			NewObj->Charset = TEXT("\"亍丌兀丐廿卅丕亘丞鬲孬噩丨禺丿匕乇夭爻卮氐囟胤馗毓睾鼗丶亟鼐乜乩亓芈孛啬嘏仄厍厝厣厥厮靥赝匚叵匦匮匾赜卦卣刂刈刎刭刳刿剀剌剞剡剜蒯剽劂劁劐劓冂罔亻仃仉仂仨仡仫仞伛仳伢佤仵伥伧伉伫佞佧攸佚佝佟佗伲伽佶佴侑侉侃侏佾佻侪佼侬侔俦俨俪俅俚俣俜俑俟俸倩偌俳倬倏倮倭俾倜倌倥倨偾偃偕偈偎偬偻傥傧傩傺僖儆僭僬僦僮儇儋仝氽佘佥俎龠汆籴兮巽黉馘冁夔勹匍訇匐凫夙兕亠兖亳衮袤亵脔裒禀嬴蠃羸冫冱冽冼凇冖冢冥讠讦讧讪讴讵讷诂诃诋诏诎诒诓诔诖诘诙诜诟诠诤诨诩诮诰诳诶诹诼诿谀谂谄谇谌谏谑谒谔谕谖谙谛谘谝谟谠谡谥谧谪谫谮谯谲谳谵谶卩卺阝阢阡阱阪阽阼陂陉陔陟陧陬陲陴隈隍隗隰邗邛邝邙邬邡邴邳邶邺邸邰郏郅邾郐郄郇郓郦郢郜郗郛郫郯郾鄄鄢鄞鄣鄱鄯鄹酃酆刍奂劢劬劭劾哿勐勖勰叟燮矍廴凵凼鬯厶弁畚巯坌垩垡塾墼壅壑圩圬圪圳圹圮圯坜圻坂坩垅坫垆坼坻坨坭坶坳垭垤垌垲埏垧垴垓垠埕埘埚埙埒垸埴埯埸埤埝堋堍埽埭堀堞堙塄堠塥塬墁墉墚墀馨鼙懿艹艽艿芏芊芨芄芎芑芗芙芫芸芾芰苈苊苣芘芷芮苋苌苁芩芴芡芪芟苄苎芤苡茉苷苤茏茇苜苴苒苘茌苻苓茑茚茆茔茕苠苕茜荑荛荜茈莒茼茴茱莛荞茯荏荇荃荟荀茗荠茭茺茳荦荥荨茛荩荬荪荭荮莰荸莳莴莠莪莓莜莅荼莶莩荽莸荻莘莞莨莺莼菁萁菥菘堇萘萋菝菽菖萜萸萑萆菔菟萏萃菸菹菪菅菀萦菰菡葜葑葚葙葳蒇蒈葺蒉葸萼葆葩葶蒌蒎萱葭蓁蓍蓐蓦蒽蓓蓊蒿蒺蓠蒡蒹蒴蒗蓥蓣蔌甍蔸蓰蔹蔟蔺蕖蔻蓿蓼蕙蕈蕨蕤蕞蕺瞢蕃蕲蕻薤薨薇薏蕹薮薜薅薹薷薰藓藁藜藿蘧蘅蘩蘖蘼廾弈夼奁耷奕奚奘匏尢尥尬尴扌扪抟抻拊拚拗拮挢拶挹捋捃掭揶捱捺掎掴捭掬掊捩掮掼揲揸揠揿揄揞揎摒揆掾摅摁搋搛搠搌搦搡摞撄摭撖摺撷撸撙撺擀擐擗擤擢攉攥攮弋忒甙弑卟叱叽叩叨叻吒吖吆呋呒呓呔呖呃吡呗呙吣吲咂咔呷呱呤咚咛咄呶呦咝哐咭哂咴哒咧咦哓哔呲咣哕咻咿哌哙哚哜咩咪咤哝哏哞唛哧唠哽唔哳唢唣唏唑唧唪啧喏喵啉啭啁啕唿啐唼唷啖啵啶啷唳唰啜喋嗒喃喱喹喈喁喟啾嗖喑啻嗟喽喾喔喙嗪嗷嗉嘟嗑嗫嗬嗔嗦嗝嗄嗯嗥嗲嗳嗌嗍嗨嗵嗤辔嘞嘈嘌嘁嘤嘣嗾嘀嘧嘭噘嘹噗嘬噍噢噙噜噌噔嚆噤噱噫噻噼嚅嚓嚯囔囗囝囡囵囫囹囿圄圊圉圜帏帙帔帑帱帻帼帷幄幔幛幞幡岌屺岍岐岖岈岘岙岑岚岜岵岢岽岬岫岱岣峁岷峄峒峤峋峥崂崃崧崦崮崤崞崆崛嵘崾崴崽嵬嵛嵯嵝嵫嵋嵊嵩嵴嶂嶙嶝豳嶷巅彳彷徂徇徉後徕徙徜徨徭徵徼衢彡犭犰犴犷犸狃狁狎狍狒狨狯狩狲狴狷猁狳猃狺狻猗猓猡猊猞猝猕猢猹猥猬猸猱獐獍獗獠獬獯獾舛夥飧夤夂饣饧饨饩饪饫饬饴饷饽馀馄馇馊馍馐馑馓馔馕庀庑庋庖庥庠庹庵庾庳赓廒廑廛廨廪膺忄忉忖忏怃忮怄忡忤忾怅怆忪忭忸怙怵怦怛怏怍怩怫怊怿怡恸恹恻恺恂恪恽悖悚悭悝悃悒悌悛惬悻悱惝惘惆惚悴愠愦愕愣惴愀愎愫慊慵憬憔憧憷懔懵忝隳闩闫闱闳闵闶闼闾阃阄阆阈阊阋阌阍阏阒阕阖阗阙阚丬爿戕氵汔汜汊沣沅沐沔沌汨汩汴汶沆沩泐泔沭泷泸泱泗沲泠泖泺泫泮沱泓泯泾洹洧洌浃浈洇洄洙洎洫浍洮洵洚浏浒浔洳涑浯涞涠浞涓涔浜浠浼浣渚淇淅淞渎涿淠渑淦淝淙渖涫渌涮渫湮湎湫溲湟溆湓湔渲渥湄滟溱溘滠漭滢溥溧溽溻溷滗溴滏溏滂溟潢潆潇漤漕滹漯漶潋潴漪漉漩澉澍澌潸潲潼潺濑濉澧澹澶濂濡濮濞濠濯瀚瀣瀛瀹瀵灏灞宀宄宕宓宥宸甯骞搴寤寮褰寰蹇謇辶迓迕迥迮迤迩迦迳迨逅逄逋逦逑逍逖逡逵逶逭逯遄遑遒遐遨遘遢遛暹遴遽邂邈邃邋彐彗彖彘尻咫屐屙孱屣屦羼弪弩弭艴弼鬻屮妁妃妍妩妪妣妗姊妫妞妤姒妲妯姗妾娅娆姝娈姣姘姹娌娉娲娴娑娣娓婀婧婊婕娼婢婵胬媪媛婷婺媾嫫媲嫒嫔媸嫠嫣嫱嫖嫦嫘嫜嬉嬗嬖嬲嬷孀尕尜孚孥孳孑孓孢驵驷驸驺驿驽骀骁骅骈骊骐骒骓骖骘骛骜骝骟骠骢骣骥骧纟纡纣纥纨纩纭纰纾绀绁绂绉绋绌绐绔绗绛绠绡绨绫绮绯绱绲缍绶绺绻绾缁缂缃缇缈缋缌缏缑缒缗缙缜缛缟缡缢缣缤缥缦缧缪缫缬缭缯缰缱缲缳缵幺畿巛甾邕玎玑玮玢玟珏珂珑玷玳珀珉珈珥珙顼琊珩珧珞玺珲琏琪瑛琦琥琨琰琮琬琛琚瑁瑜瑗瑕瑙瑷瑭瑾璜璎璀璁璇璋璞璨璩璐璧瓒璺韪韫韬杌杓杞杈杩枥枇杪杳枘枧杵枨枞枭枋杷杼柰栉柘栊柩枰栌柙枵柚枳柝栀柃枸柢栎柁柽栲栳桠桡桎桢桄桤梃栝桕桦桁桧桀栾桊桉栩梵梏桴桷梓桫棂楮棼椟椠棹椤棰椋椁楗棣椐楱椹楠楂楝榄楫榀榘楸椴槌榇榈槎榉楦楣楹榛榧榻榫榭槔榱槁槊槟榕槠榍槿樯槭樗樘橥槲橄樾檠橐橛樵檎橹樽樨橘橼檑檐檩檗檫猷獒殁殂殇殄殒殓殍殚殛殡殪轫轭轱轲轳轵轶轸轷轹轺轼轾辁辂辄辇辋辍辎辏辘辚軎戋戗戛戟戢戡戥戤戬臧瓯瓴瓿甏甑甓攴旮旯旰昊昙杲昃昕昀炅曷昝昴昱昶昵耆晟晔晁晏晖晡晗晷暄暌暧暝暾曛曜曦曩贲贳贶贻贽赀赅赆赈赉赇赍赕赙觇觊觋觌觎觏觐觑牮犟牝牦牯牾牿犄犋犍犏犒挈挲掰搿擘耄毪毳毽毵毹氅氇氆氍氕氘氙氚氡氩氤氪氲攵敕敫牍牒牖爰虢刖肟肜肓肼朊肽肱肫肭肴肷胧胨胩胪胛胂胄胙胍胗朐胝胫胱胴胭脍脎胲胼朕脒豚脶脞脬脘脲腈腌腓腴腙腚腱腠腩腼腽腭腧塍媵膈膂膑滕膣膪臌朦臊膻臁膦欤欷欹歃歆歙飑飒飓飕飙飚殳彀毂觳斐齑斓於旆旄旃旌旎旒旖炀炜炖炝炻烀炷炫炱烨烊焐焓焖焯焱煳煜煨煅煲煊煸煺熘熳熵熨熠燠燔燧燹爝爨灬焘煦熹戾戽扃扈扉礻祀祆祉祛祜祓祚祢祗祠祯祧祺禅禊禚禧禳忑忐怼恝恚恧恁恙恣悫愆愍慝憩憝懋懑戆肀聿沓泶淼矶矸砀砉砗砘砑斫砭砜砝砹砺砻砟砼砥砬砣砩硎硭硖硗砦硐硇硌硪碛碓碚碇碜碡碣碲碹碥磔磙磉磬磲礅磴礓礤礞礴龛黹黻黼盱眄眍盹眇眈眚眢眙眭眦眵眸睐睑睇睃睚睨睢睥睿瞍睽瞀瞌瞑瞟瞠瞰瞵瞽町畀畎畋畈畛畲畹疃罘罡罟詈罨罴罱罹羁罾盍盥蠲钅钆钇钋钊钌钍钏钐钔钗钕钚钛钜钣钤钫钪钭钬钯钰钲钴钶钷钸钹钺钼钽钿铄铈铉铊铋铌铍铎铐铑铒铕铖铗铙铘铛铞铟铠铢铤铥铧铨铪铩铫铮铯铳铴铵铷铹铼铽铿锃锂锆锇锉锊锍锎锏锒锓锔锕锖锘锛锝锞锟锢锪锫锩锬锱锲锴锶锷锸锼锾锿镂锵镄镅镆镉镌镎镏镒镓镔镖镗镘镙镛镞镟镝镡镢镤镥镦镧镨镩镪镫镬镯镱镲镳锺矧矬雉秕秭秣秫稆嵇稃稂稞稔稹稷穑黏馥穰皈皎皓皙皤瓞瓠甬鸠鸢鸨鸩鸪鸫鸬鸲鸱鸶鸸鸷鸹鸺鸾鹁鹂鹄鹆鹇鹈鹉鹋鹌鹎鹑鹕鹗鹚鹛鹜鹞鹣鹦鹧鹨鹩鹪鹫鹬鹱鹭鹳疒疔疖疠疝疬疣疳疴疸痄疱疰痃痂痖痍痣痨痦痤痫痧瘃痱痼痿瘐瘀瘅瘌瘗瘊瘥瘘瘕瘙瘛瘼瘢瘠癀瘭瘰瘿瘵癃瘾瘳癍癞癔癜癖癫癯翊竦穸穹窀窆窈窕窦窠窬窨窭窳衤衩衲衽衿袂袢裆袷袼裉裢裎裣裥裱褚裼裨裾裰褡褙褓褛褊褴褫褶襁襦襻疋胥皲皴矜耒耔耖耜耠耢耥耦耧耩耨耱耋耵聃聆聍聒聩聱覃顸颀颃颉颌颍颏颔颚颛颞颟颡颢颥颦虍虔虬虮虿虺虼虻蚨蚍蚋蚬蚝蚧蚣蚪蚓蚩蚶蛄蚵蛎蚰蚺蚱蚯蛉蛏蚴蛩蛱蛲蛭蛳蛐蜓蛞蛴蛟蛘蛑蜃蜇蛸蜈蜊蜍蜉蜣蜻蜞蜥蜮蜚蜾蝈蜴蜱蜩蜷蜿螂蜢蝽蝾蝻蝠蝰蝌蝮螋蝓蝣蝼蝤蝙蝥螓螯螨蟒蟆螈螅螭螗螃螫蟥螬螵螳蟋蟓螽蟑蟀蟊蟛蟪蟠蟮蠖蠓蟾蠊蠛蠡蠹蠼缶罂罄罅舐竺竽笈笃笄笕笊笫笏筇笸笪笙笮笱笠笥笤笳笾笞筘筚筅筵筌筝筠筮筻筢筲筱箐箦箧箸箬箝箨箅箪箜箢箫箴篑篁篌篝篚篥篦篪簌篾篼簏簖簋簟簪簦簸籁籀臾舁舂舄臬衄舡舢舣舭舯舨舫舸舻舳舴舾艄艉艋艏艚艟艨衾袅袈裘裟襞羝羟羧羯羰羲籼敉粑粝粜粞粢粲粼粽糁糇糌糍糈糅糗糨艮暨羿翎翕翥翡翦翩翮翳糸絷綦綮繇纛麸麴赳趄趔趑趱赧赭豇豉酊酐酎酏酤酢酡酰酩酯酽酾酲酴酹醌醅醐醍醑醢醣醪醭醮醯醵醴醺豕鹾趸跫踅蹙蹩趵趿趼趺跄跖跗跚跞跎跏跛跆跬跷跸跣跹跻跤踉跽踔踝踟踬踮踣踯踺蹀踹踵踽踱蹉蹁蹂蹑蹒蹊蹰蹶蹼蹯蹴躅躏躔躐躜躞豸貂貊貅貘貔斛觖觞觚觜觥觫觯訾謦靓雩雳雯霆霁霈霏霎霪霭霰霾龀龃龅龆龇龈龉龊龌黾鼋鼍隹隼隽雎雒瞿雠銎銮鋈錾鍪鏊鎏鐾鑫鱿鲂鲅鲆鲇鲈稣鲋鲎鲐鲑鲒鲔鲕鲚鲛鲞鲟鲠鲡鲢鲣鲥鲦鲧鲨鲩鲫鲭鲮鲰鲱鲲鲳鲴鲵鲶鲷鲺鲻鲼鲽鳄鳅鳆鳇鳊鳋鳌鳍鳎鳏鳐鳓鳔鳕鳗鳘鳙鳜鳝鳟鳢靼鞅鞑鞒鞔鞯鞫鞣鞲鞴骱骰骷鹘骶骺骼髁髀髅髂髋髌髑魅魃魇魉魈魍魑飨餍餮饕饔髟髡髦髯髫髻髭髹鬈鬏鬓鬟鬣麽麾縻麂麇麈麋麒鏖麝麟黛黜黝黠黟黢黩黧黥黪黯鼢鼬鼯鼹鼷鼽鼾齄\"");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);

			NewObj = NewObject<USDFFontCharset>(UseOuter, USDFFontCharset::StaticClass(), NAME_None, MaskedOuterFlags, nullptr);
			NewObj->Name = TEXT("Chinese");
			NewObj->bEnabled = false;
			NewObj->Charset = TEXT("[0x4E00, 0x9FBF]");
			NewObj->FontScale = 36;
			NewObj->PxRange = 4;
			Font->Charsets.Add(NewObj);
		}

		RebuildCharsetTree();
		
		TreeView->ClearSelection();

		TArray<TSharedPtr<FCharsetItem>> SelectedItems;
		for (int32 Index = 19; Index >= 0; --Index)
		{
			if (CharsetItems.IsValidIndex(Index))
			{
				SelectedItems.Add(CharsetItems[CharsetItems.Num() - (20 - Index)]);
			}
		}
		
		TreeView->SetItemSelection(SelectedItems, true);

		if (SelectedItems.Num() == 1)
		{
			OnRenameNode(nullptr); 
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
