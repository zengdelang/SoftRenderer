#include "DetailCustomizations/SubComponentsDetailRow/SComponentDetailCategory.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

void SComponentDetailCategory::Construct( const FArguments& InArgs, class UObject* InOwnerObject)
{
	SetCanTick(false);
	
	OwnerObject = InOwnerObject;

	SetVisibility(EVisibility::SelfHitTestInvisible);
	bHideEnableCheckBox = true;
}

FVector2D SComponentDetailCategory::ComputeDesiredSize(float InValue) const
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
									SNew(SCheckBox)
									.Visibility(EVisibility::Collapsed)
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
			}
		}
	}
	
	return SCompoundWidget::ComputeDesiredSize(InValue);
}

#undef LOCTEXT_NAMESPACE
