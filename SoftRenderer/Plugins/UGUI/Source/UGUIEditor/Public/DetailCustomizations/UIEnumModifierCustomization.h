#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/Input/SEditableTextBox.h"

class IPropertyHandle;

DECLARE_DELEGATE(FOnCommitChange)

class SCustomEnumEditBox : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomEnumEditBox) { }
		SLATE_ARGUMENT(UEnum*, CustomEnumType)
		SLATE_ARGUMENT(int32, EnumIndex)
		SLATE_ARGUMENT(bool, bReadOnly)
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PropertyHandle)
		SLATE_EVENT(FOnCommitChange, OnCommitChange)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	FText GetName() const;
	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo) const;
	void OnTextChanged(const FText& NewText) const;

	void OnToolTipTextChanged(const FText& NewText) const;
	void NewToolTipEntered(const FText& NewText, ETextCommit::Type CommitInfo) const;
	FText GetToolTipText() const;

private:
	UEnum* EnumType = nullptr;
	int32 EnumIndex = -1;
	TSharedPtr<IPropertyHandle> PropertyHandle;
	FOnCommitChange						OnCommitChange;
	TSharedPtr<SEditableTextBox>		NameEditBox;
	TSharedPtr<SEditableTextBox>		ToolTipEditBox;
	
};

class FUIEnumModifierCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance() 
	{
		return MakeShareable( new FUIEnumModifierCustomization);
	}

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader( TSharedRef<class IPropertyHandle> InStructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;
	virtual void CustomizeChildren( TSharedRef<class IPropertyHandle> InStructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils ) override;

protected:
	void OnCommitChange() const;
	
private:
	/** Handle to the struct property being customized */
	TSharedPtr<IPropertyHandle> StructPropertyHandle;

	UEnum* EnumType = nullptr;

};
