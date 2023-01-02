#include "DetailCustomizations/CurveComponentDetails.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FCurveComponentDetails::MakeInstance()
{
	return MakeShareable(new FCurveComponentDetails);
}

void FCurveComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{

}

#undef LOCTEXT_NAMESPACE