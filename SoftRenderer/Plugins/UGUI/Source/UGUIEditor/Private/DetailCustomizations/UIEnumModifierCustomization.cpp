#include "DetailCustomizations/UIEnumModifierCustomization.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "DetailLayoutBuilder.h"
#include "UIEnumModifier.h"
#include "IDetailChildrenBuilder.h"
#include "Misc/MessageDialog.h"
#include "PropertyHandle.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
void SCustomEnumEditBox::Construct(const FArguments& InArgs)
{
	EnumType = InArgs._CustomEnumType;
	EnumIndex = InArgs._EnumIndex;
	PropertyHandle = InArgs._PropertyHandle;
	OnCommitChange = InArgs._OnCommitChange;
	check(EnumType);

	ChildSlot
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.MaxWidth(130)
		[
			SAssignNew(NameEditBox, SEditableTextBox)
			.Text(this, &SCustomEnumEditBox::GetName)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OnTextCommitted(this, &SCustomEnumEditBox::NewNameEntered)
			.OnTextChanged(this, &SCustomEnumEditBox::OnTextChanged)
			.IsReadOnly(EnumIndex == 0)
			.SelectAllTextWhenFocused(true)
			.MinDesiredWidth(130)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SSpacer)
			.Size(FVector2D(5, 0))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.MaxWidth(130)
		[
			SAssignNew(ToolTipEditBox, SEditableTextBox)
			.Text(this, &SCustomEnumEditBox::GetToolTipText)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OnTextCommitted(this, &SCustomEnumEditBox::NewToolTipEntered)
			.OnTextChanged(this, &SCustomEnumEditBox::OnToolTipTextChanged)
			.SelectAllTextWhenFocused(true)
			.IsReadOnly(EnumIndex == 0)
			.MinDesiredWidth(130)
		]
	];
}

void SCustomEnumEditBox::OnTextChanged(const FText& NewText) const
{
	const FString NewName = NewText.ToString();

	if (NewName.Find(TEXT(" ")) != INDEX_NONE)
	{
		// no white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SCustomEnumEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo) const
{
	if ((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		const FString NewName = NewText.ToString();
		if (NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			const FName NewStateName(*NewName);
			if (EnumType)
			{
				TArray<void*> RawData;
				PropertyHandle->AccessRawData(RawData);

				if (RawData.Num() > 0)
				{
					FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
					if (CustomEnum && CustomEnum->EnumItems.IsValidIndex(EnumIndex))
					{
						auto& EnumItem = CustomEnum->EnumItems[EnumIndex];
						if (EnumItem.Name != NAME_None && NewStateName == NAME_None)
						{
							if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SCustomEnumEditBox_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
							{
								return;
							}
						}
						if (NewStateName != EnumItem.Name)
						{
							EnumItem.Name = NewStateName;
							OnCommitChange.ExecuteIfBound();
						}
					}
				}
			}
		}
		else
		{
			// clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SCustomEnumEditBox::GetName() const
{
	if (EnumType)
	{
		if (EnumIndex == 0)
		{
			return EnumType->GetDisplayNameTextByIndex(EnumIndex);
		}
		
		TArray<void*> RawData;
		PropertyHandle->AccessRawData(RawData);

		if (RawData.Num() > 0)
		{
			FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
			if (CustomEnum && CustomEnum->EnumItems.IsValidIndex(EnumIndex))
			{
				return FText::FromName(CustomEnum->EnumItems[EnumIndex].Name);
			}
		}
	}
	return FText::GetEmpty();
}

void SCustomEnumEditBox::OnToolTipTextChanged(const FText& NewText) const
{
	const FString NewName = NewText.ToString();
	if (NewName.Find(TEXT(" ")) != INDEX_NONE)
	{
		// no white space
		ToolTipEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		ToolTipEditBox->SetError(TEXT(""));
	}
}

void SCustomEnumEditBox::NewToolTipEntered(const FText& NewText, ETextCommit::Type CommitInfo) const
{
	if ((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		const FString NewName = NewText.ToString();
		if (NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			const FName NewStateName(*NewName);
			if (EnumType)
			{
				TArray<void*> RawData;
				PropertyHandle->AccessRawData(RawData);

				if (RawData.Num() > 0)
				{
					FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
					if (CustomEnum && CustomEnum->EnumItems.IsValidIndex(EnumIndex))
					{
						auto& EnumItem = CustomEnum->EnumItems[EnumIndex];
						if (EnumItem.ToolTip != NAME_None && NewStateName == NAME_None)
						{
							if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SCustomEnumEditBox_DeleteConfirm", "Would you like to delete the tooltip?")) == EAppReturnType::No)
							{
								return;
							}
						}
						if (NewStateName != EnumItem.ToolTip)
						{
							EnumItem.ToolTip = NewStateName;
							OnCommitChange.ExecuteIfBound();
						}
					}
				}
			}
		}
		else
		{
			// clear error
			ToolTipEditBox->SetError(TEXT(""));
		}
	}
}

FText SCustomEnumEditBox::GetToolTipText() const
{
	if (EnumType)
	{		
		TArray<void*> RawData;
		PropertyHandle->AccessRawData(RawData);

		if (RawData.Num() > 0)
		{
			FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
			if (CustomEnum && CustomEnum->EnumItems.IsValidIndex(EnumIndex))
			{
				return FText::FromName(CustomEnum->EnumItems[EnumIndex].ToolTip);
			}
		}
	}
	return FText::GetEmpty();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FUIEnumModifierCustomization::CustomizeHeader( TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	StructPropertyHandle = InStructPropertyHandle;

	const FString& EnumName = StructPropertyHandle->GetMetaData("Enum");
	
	const FText DisplayNameFormat = FText::FromString(StructPropertyHandle->GetMetaData("Name"));

	EnumType = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
	const int32 EnumCount = EnumType ? EnumType->NumEnums() - 3 : 0;
	
	const FText DisplayName = FText::Format(DisplayNameFormat, EnumCount);

	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
	.ValueContent()
		// Make enough space for each child handle
		.MinDesiredWidth(250.0f)
		.MaxDesiredWidth(0.0f)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.AutoWrapText(true)
			.Text(DisplayName)
		];

}

void FUIEnumModifierCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (EnumType)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);

		if (RawData.Num() > 0)
		{
			FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
			if (CustomEnum)
			{
				if (CustomEnum->EnumItems.Num() < EnumType->NumEnums() - 2)
				{
					CustomEnum->EnumItems.AddDefaulted(EnumType->NumEnums() - 2 - CustomEnum->EnumItems.Num());
				}

				const FString& SearchString = StructPropertyHandle->GetMetaData("SearchString");
				for (int32 ChildIndex = 0; ChildIndex < EnumType->NumEnums() - 2; ++ChildIndex)
				{
					FDetailWidgetRow& Row = StructBuilder.AddCustomRow(FText::FromString(SearchString));

					FString TypeString = EnumType->GetNameStringByValue(ChildIndex);

					Row.NameContent()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TypeString))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						];

					Row.ValueContent()
						[
							SNew(SCustomEnumEditBox)
							.CustomEnumType(EnumType)
							.EnumIndex(ChildIndex)
							.PropertyHandle(StructPropertyHandle)
							.OnCommitChange(this, &FUIEnumModifierCustomization::OnCommitChange)
						];				
				}
			}
		}
	}
}

void FUIEnumModifierCustomization::OnCommitChange() const
{
	bool bDoCommit = true;
	// make sure it verifies all data is correct
	// skip the first one
	if (EnumType)
	{
		TArray<void*> RawData;
		StructPropertyHandle->AccessRawData(RawData);

		if (RawData.Num() > 0)
		{
			FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
			if (CustomEnum)
			{
				for (auto Iterator = CustomEnum->EnumItems.CreateConstIterator() + 1; Iterator; ++Iterator)
				{
					auto EnumItem = *Iterator;
					if (EnumItem.Name != NAME_None)
					{
						// make sure no same name exists
						for (auto InnerIterator = Iterator + 1; InnerIterator; ++InnerIterator)
						{
							const auto InnerItem = *InnerIterator;
							if (EnumItem.Name == InnerItem.Name)
							{
								// duplicate name, warn user and get out
								FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FUIEnumModifierCustomization_InvalidName", "Duplicate name found."));
								bDoCommit = false;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (bDoCommit)
	{
		StructPropertyHandle->NotifyPreChange();

		if (EnumType)
		{
			TArray<void*> RawData;
			StructPropertyHandle->AccessRawData(RawData);

			if (RawData.Num() > 0)
			{
				FUIEnumModifier* CustomEnum = static_cast<FUIEnumModifier*>(RawData[0]);
				if (CustomEnum)
				{
					CustomEnum->SetEnumInfo(EnumType);
				}
			}
		}

		StructPropertyHandle->NotifyPostChange();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
