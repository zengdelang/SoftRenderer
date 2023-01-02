#include "SSpriteAtlasPackerSpriteListItem.h"
#include "SpriteAtlasPackerPrivate.h"
#include "Misc/FileHelper.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

#define LOCTEXT_NAMESPACE "SpriteAtlasPackerSpriteListItem"

void SAtlasPackerSpriteItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView, const TSharedRef<FSpriteItem>& InItem)
{
	Item = InItem;
	
	// Create the name field
	InlineWidget =
				SNew(SInlineEditableTextBlock)
					.Text(this, &SAtlasPackerSpriteItem::GetNameLabel)
					.ColorAndOpacity(this, &SAtlasPackerSpriteItem::GetColorForNameLabel)
					.OnVerifyTextChanged( this, &SAtlasPackerSpriteItem::OnNameTextVerifyChanged )
					.OnTextCommitted( this, &SAtlasPackerSpriteItem::OnNameTextCommit )
					.IsSelected( this, &SAtlasPackerSpriteItem::IsSelectedExclusively );
					
	Item->SetRenameRequestedDelegate(FSpriteItem::FOnRenameRequested::CreateSP(InlineWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode));

	FSuperRowType::FArguments Args = FSuperRowType::FArguments()
	.Style(//bIsSeparator ?
	//	&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.NoHoverTableRow") :
		&FEditorStyle::Get().GetWidgetStyle<FTableRowStyle>("SceneOutliner.TableViewRow"))
	.Padding(FMargin(0.f, 0.f, 0.f, 4.f));
	/*.ShowSelection(!bIsSeparator)
	.OnDragDetected(this, &SSCS_UI_RowWidget::HandleOnDragDetected)
	.OnDragEnter(this, &SSCS_UI_RowWidget::HandleOnDragEnter)
	.OnDragLeave(this, &SSCS_UI_RowWidget::HandleOnDragLeave)
	.OnCanAcceptDrop(this, &SSCS_UI_RowWidget::HandleOnCanAcceptDrop)
	.OnAcceptDrop(this, &SSCS_UI_RowWidget::HandleOnAcceptDrop);*/

	SMultiColumnTableRow<TSharedPtr<FSpriteItem>>::Construct( Args, OwnerTableView );
}

TSharedRef<SWidget> SAtlasPackerSpriteItem::GenerateWidgetForColumn(const FName& InColumnName)
{
	return SNew(SHorizontalBox)
	//.ToolTip(Tooltip)
	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(10, 2, 0, 0)
	.VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(20)
		.HeightOverride(20)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SImage)
			.Image(Item->ImageBrush.IsValid() ? Item->ImageBrush.Get() : nullptr)
		]
	]
	+SHorizontalBox::Slot()
	.VAlign(VAlign_Center)
	.Padding(2, 2, 0, 0)
	[
		InlineWidget.ToSharedRef()
	];
}

FText SAtlasPackerSpriteItem::GetNameLabel() const
{
	return FText::FromString(Item->Name);
}

FSlateColor SAtlasPackerSpriteItem::GetColorForNameLabel() const
{
	for (const auto& Sprite : FSpriteAtlasPacker::Get().GetRootSprites())
	{
		if (Sprite != Item && Sprite->Name == Item->Name)
		{
			return FLinearColor::Red;	
		}
	}
	return FLinearColor::White;	
}

void SAtlasPackerSpriteItem::OnNameTextCommit(const FText& InNewName, ETextCommit::Type InTextCommit)
{
	Item->OnCompleteRename(InNewName);
}

bool SAtlasPackerSpriteItem::OnNameTextVerifyChanged(const FText& InNewText, FText& OutErrorMessage)
{
	const FString NewTextString = InNewText.ToString();
	
	if (NewTextString.IsEmpty())
	{
		OutErrorMessage = NSLOCTEXT("SSCSEditor", "RenameFailed_LeftBlank", "Names cannot be left blank!");
		return false;
	}
	
	if (!FPaths::ValidatePath(NewTextString, &OutErrorMessage))
	{
		return false;
	}

	if (!FFileHelper::IsFilenameValidForSaving(NewTextString, OutErrorMessage))
	{
		return false;
	}

	if (NewTextString == Item->Name)
	{
		return true;
	}

	for (const auto& Sprite : FSpriteAtlasPacker::Get().GetRootSprites())
	{
		if (Sprite != Item && Sprite->Name == NewTextString)
		{
			OutErrorMessage = LOCTEXT("RenameFailed_ExistingName", "Another node already has the same name.");
			return false;
		}
	}
	
	return true;
}

#undef LOCTEXT_NAMESPACE
