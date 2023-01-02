#include "DetailCustomizations/UICascadeComponentDetails.h"
#include "DetailCustomizations/UIStaticMeshComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUICascadeComponentDetails::MakeInstance()
{
	return MakeShareable(new FUICascadeComponentDetails);
}

void FUICascadeComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// UICascade Category
	IDetailCategoryBuilder& UICascadeCategory = DetailBuilder.EditCategory(TEXT("UI Cascade"), FText(LOCTEXT("UICascadeComponentCategory", "UI Cascade")));

	AddRowHeaderContent(UICascadeCategory, DetailBuilder);
	
	UICascadeCategory.AddProperty(TEXT("ParticleTemplate"));
	UICascadeCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	UICascadeCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	
	UICascadeCategory.AddProperty(TEXT("bActivateOnEnable"));
	UICascadeCategory.AddProperty(TEXT("bDeactivateOnDisable"));

	AddUIPrimitiveDetails(DetailBuilder, UICascadeCategory);
}

#undef LOCTEXT_NAMESPACE
