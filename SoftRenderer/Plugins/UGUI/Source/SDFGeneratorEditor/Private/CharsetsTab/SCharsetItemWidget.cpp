#include "SCharsetItemWidget.h"

#include "SCharsetsTabWidget.h"
#include "Misc/FileHelper.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "CharsetItemWidget"

//////////////////////////////////////////////////////////////////////////
// FCharsetItemDragDropOp

class FCharsetItemDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(FCharsetItemDragDropOp, FDragDropOperation)
	
	TArray<TSharedPtr<FCharsetItem> > SourceItems;

	bool bCanDrop = false;

	static TSharedRef<FCharsetItemDragDropOp> New();
};

TSharedRef<FCharsetItemDragDropOp> FCharsetItemDragDropOp::New()
{
	TSharedPtr<FCharsetItemDragDropOp> Operation = MakeShareable(new FCharsetItemDragDropOp);
	Operation->Construct();
	return Operation.ToSharedRef();
}

//////////////////////////////////////////////////////////////////////////
// SCharsetItemWidget

void SCharsetItemWidget::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<FCharsetItem>& InItem, TWeakPtr<class SCharsetsTabWidget> InCharsetTabWidget)
{
	Item = InItem;
	CharsetTabWidget = InCharsetTabWidget;
	
	// Create the name field
	InlineWidget =
				SNew(SInlineEditableTextBlock)
					.Text(this, &SCharsetItemWidget::GetNameLabel)
					.ColorAndOpacity(this, &SCharsetItemWidget::GetColorForNameLabel)
					.OnVerifyTextChanged( this, &SCharsetItemWidget::OnNameTextVerifyChanged )
					.OnTextCommitted( this, &SCharsetItemWidget::OnNameTextCommit )
					.IsSelected( this, &SCharsetItemWidget::IsSelectedExclusively );
					
	Item->SetRenameRequestedDelegate(FCharsetItem::FOnRenameRequested::CreateSP(InlineWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode));

	FSuperRowType::FArguments Args = FSuperRowType::FArguments()
	.Style(//bIsSeparator ?
	//	&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.NoHoverTableRow") :
		&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("SceneOutliner.TableViewRow"))
	.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
	.ShowSelection(true)
	.OnDragDetected(this, &SCharsetItemWidget::HandleOnDragDetected)
	.OnDragEnter(this, &SCharsetItemWidget::HandleOnDragEnter)
	.OnDragLeave(this, &SCharsetItemWidget::HandleOnDragLeave)
	.OnCanAcceptDrop(this, &SCharsetItemWidget::HandleOnCanAcceptDrop)
	.OnAcceptDrop(this, &SCharsetItemWidget::HandleOnAcceptDrop);

	SMultiColumnTableRow<TSharedPtr<FCharsetItem>>::Construct( Args, OwnerTableView );
}

TSharedRef<SWidget> SCharsetItemWidget::GenerateWidgetForColumn(const FName& InColumnName)
{
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.VAlign(VAlign_Center)
	.Padding(2, 2, 0, 0)
	[
		InlineWidget.ToSharedRef()
	];
}

FText SCharsetItemWidget::GetNameLabel() const
{
	return FText::FromString(Item->GetCharsetName());
}

FSlateColor SCharsetItemWidget::GetColorForNameLabel() const
{
	if (Item.IsValid() && Item->FontCharset && !Item->FontCharset->bEnabled)
	{
		return FLinearColor::Gray;	
	}
	return FLinearColor::White;	
}

void SCharsetItemWidget::OnNameTextCommit(const FText& InNewName, ETextCommit::Type InTextCommit)
{
	Item->OnCompleteRename(InNewName);
}

bool SCharsetItemWidget::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	const FString NewTextString = InNewText.ToString();
	
	if (NewTextString.IsEmpty())
	{
		OutErrorMessage = NSLOCTEXT("SSCSEditor", "RenameFailed_LeftBlank", "Names cannot be left blank!");
		return false;
	}
	
	return true;
}

void SCharsetItemWidget::HandleOnDragEnter(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return;
	}

	TSharedPtr<FCharsetItemDragDropOp> DragRowOp = DragDropEvent.GetOperationAs<FCharsetItemDragDropOp>();
	if (DragRowOp.IsValid())
	{
		DragRowOp->bCanDrop = true;
	}
}

void SCharsetItemWidget::HandleOnDragLeave(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FCharsetItemDragDropOp> DragRowOp = DragDropEvent.GetOperationAs<FCharsetItemDragDropOp>();
	if (DragRowOp.IsValid())
	{
		DragRowOp->bCanDrop = false;
	}
}

FReply SCharsetItemWidget::HandleOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TSharedPtr<SCharsetsTabWidget> CharsetTabWidgetPtr = CharsetTabWidget.Pin();
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && CharsetTabWidgetPtr.IsValid())
	{
		TArray<TSharedPtr<FCharsetItem>> SelectedItemPtrs = CharsetTabWidgetPtr->GetSelectedItems();
		if (SelectedItemPtrs.Num() == 0)
		{
			return FReply::Unhandled();
		}

		TSharedRef<FCharsetItemDragDropOp> Operation = FCharsetItemDragDropOp::New();
		Operation->SourceItems = SelectedItemPtrs;
		return FReply::Handled().BeginDragDrop(Operation);
	}
	
	return FReply::Unhandled();
}

TOptional<EItemDropZone> SCharsetItemWidget::HandleOnCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TSharedPtr<FCharsetItem> TargetItem)
{
	TOptional<EItemDropZone> ReturnDropZone;

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid())
	{
		if (Operation->IsOfType<FCharsetItemDragDropOp>() && Item.IsValid() && Item->FontCharset != nullptr)
		{
			TSharedPtr<FCharsetItemDragDropOp> DragRowOp = StaticCastSharedPtr<FCharsetItemDragDropOp>(Operation);
			check(DragRowOp.IsValid());

			if (DragRowOp->bCanDrop)
			{
				ReturnDropZone = DropZone;
			}
		}
	}

	return ReturnDropZone;
}

FReply SCharsetItemWidget::HandleOnAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone,
	TSharedPtr<FCharsetItem> TargetItem)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (!Operation.IsValid())
	{
		return FReply::Handled();
	}
	
	if (Operation->IsOfType<FCharsetItemDragDropOp>() && Item.IsValid() && Item->FontCharset != nullptr)
	{
		TSharedPtr<FCharsetItemDragDropOp> DragRowOp = StaticCastSharedPtr<FCharsetItemDragDropOp>( Operation );	
		check(DragRowOp.IsValid());

		if (DragRowOp->bCanDrop)
		{
			if (CharsetTabWidget.IsValid())
			{
			    CharsetTabWidget.Pin()->ReparentItems(DragRowOp->SourceItems, TargetItem, DropZone);
			}
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
