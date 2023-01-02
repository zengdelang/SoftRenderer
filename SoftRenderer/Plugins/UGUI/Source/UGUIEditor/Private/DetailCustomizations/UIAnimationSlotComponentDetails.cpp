#include "DetailCustomizations/UIAnimationSlotComponentDetails.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUIAnimationSlotComponentDetails::MakeInstance()
{
	return MakeShareable(new FUIAnimationSlotComponentDetails);
}

void FUIAnimationSlotComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

}

#undef LOCTEXT_NAMESPACE