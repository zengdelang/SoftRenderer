#include "DetailCustomizations/SubComponentsDetailRow/SSubComponentDetailCategory.h"
#include "DetailCustomizations/SubComponentsDetailRow/SSubComponentOverlay.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

void SSubComponentDetailCategory::Construct( const FArguments& InArgs, const TSharedPtr<IPropertyHandle> InEnabledHandle, const TSharedRef< class IPropertyHandle >& InPropertyHandle, 
	const TSharedRef< class IPropertyHandle >& InSubComponentsPropertyHandle, class UObject* InOwnerObject, const TAttribute<bool>& InIsParentEnabled)
{
	SetCanTick(false);

	EnabledHandle = InEnabledHandle;

	OwnerObject = InOwnerObject;
	PropertyHandle = InPropertyHandle;
	SubComponentsPropertyHandle = InSubComponentsPropertyHandle;

	IsParentEnabled = InIsParentEnabled;

	SetVisibility(EVisibility::SelfHitTestInvisible);
	bHideEnableCheckBox = false;
}

void SSubComponentDetailCategory::SetIndex(int32 InIndex)
{
	Index = InIndex;
}

FVector2D SSubComponentDetailCategory::ComputeDesiredSize(float InValue) const
{
	auto ParentWidget = GetParentWidget();
	if (ParentWidget.IsValid())
	{
		ParentWidget = ParentWidget->GetParentWidget();
		if (ParentWidget.IsValid())
		{
			SSplitter* ParentSplitter = static_cast<SSplitter*>(ParentWidget.Get());
			if (ParentSplitter)
			{
				FChildren* Children = ParentSplitter->GetChildren();
				if (Children && Children->Num() > 0)
				{
					const TSharedPtr<SWidget> Child = Children->GetChildAt(0);
					if (Child.IsValid())
					{
						SHorizontalBox* HorizontalBox = static_cast<SHorizontalBox*>(Child.Get());
						if (HorizontalBox && HorizontalBox->NumSlots() == 2)
						{
							const TSharedPtr<SWidget> Child1 = HorizontalBox->GetChildren()->GetChildAt(1);
							const auto EnableWidget = EnabledHandle->CreatePropertyValueWidget();

							if (bHideEnableCheckBox)
							{
								EnableWidget->SetVisibility(EVisibility::Collapsed);
							}
							
							HorizontalBox->RemoveSlot(Child1.ToSharedRef());

							HorizontalBox->AddSlot()
								.AutoWidth()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								.Padding(0, 0, 2, 0)
								[
									SNew(SImage)
									.Image(ComponentIcon)
								];
							
							HorizontalBox->AddSlot()
								.AutoWidth()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								.Padding(2, 0)
								[
									EnableWidget
								];
							
							HorizontalBox->AddSlot()
								.HAlign(HAlign_Left)
								.VAlign(VAlign_Center)
								[
									Child1.ToSharedRef()
								];
						}
					}
				}

				ParentWidget = ParentWidget->GetParentWidget();
				if (ParentWidget.IsValid())
				{	
					SBorder* ParentBorder = static_cast<SBorder*>(ParentWidget.Get());
					if (ParentBorder && ParentBorder->GetTag() != TEXT("Overlay"))
					{
						ParentSplitter->SetVisibility(EVisibility::SelfHitTestInvisible);
						FChildren* BorderChildren = ParentBorder->GetChildren();
						if (BorderChildren && BorderChildren->Num() == 1)
						{
							const TSharedPtr<SWidget> Child0 = BorderChildren->GetChildAt(0);
							const TSharedRef<SOverlay> Overlay = SNew(SSubComponentOverlay, PropertyHandle, SubComponentsPropertyHandle, OwnerObject.Get(), Index, IsParentEnabled);
							Overlay->SetTag(TEXT("Overlay"));

							ParentBorder->SetContent(Overlay);
							
							Overlay->AddSlot()
								[
									Child0.ToSharedRef()
								];
						}
					}
				}
			}
		}
	}
	
	return SCompoundWidget::ComputeDesiredSize(InValue);
}

#undef LOCTEXT_NAMESPACE
