#include "DetailCustomizations/SafeZoneComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FSafeZoneComponentDetails::MakeInstance()
{
	return MakeShareable(new FSafeZoneComponentDetails);
}

void FSafeZoneComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Layout"));

	IDetailCategoryBuilder& SafeZoneCategory = DetailBuilder.EditCategory(TEXT("Safe Zone"));

	AddRowHeaderContent(SafeZoneCategory, DetailBuilder);
	
	SafeZoneCategory.AddProperty(TEXT("ExtraPadding"));
	SafeZoneCategory.AddProperty(TEXT("bPadLeft"));
	SafeZoneCategory.AddProperty(TEXT("bPadRight"));
	SafeZoneCategory.AddProperty(TEXT("bPadTop"));
	SafeZoneCategory.AddProperty(TEXT("bPadBottom"));
	SafeZoneCategory.AddProperty(TEXT("SafeAreaScale"));
}

#undef LOCTEXT_NAMESPACE
