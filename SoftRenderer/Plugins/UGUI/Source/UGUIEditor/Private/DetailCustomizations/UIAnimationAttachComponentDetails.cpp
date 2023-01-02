#include "DetailCustomizations/UIAnimationAttachComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUIAnimationAttachComponentDetails::MakeInstance()
{
	return MakeShareable(new FUIAnimationAttachComponentDetails);
}

void FUIAnimationAttachComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	/// UIAnimationAttach Category
	IDetailCategoryBuilder& UIAnimationAttachCategory = DetailBuilder.EditCategory(TEXT("UI Animation Attach"), FText(LOCTEXT("UIAnimationAttachComponentCategory", "UI Animation Attach")));

	AddRowHeaderContent(UIAnimationAttachCategory, DetailBuilder);
}

#undef LOCTEXT_NAMESPACE