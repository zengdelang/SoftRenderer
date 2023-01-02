#include "DetailCustomizations/UINiagaraComponentDetails.h"
#include "DetailCustomizations/UIStaticMeshComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUINiagaraComponentDetails::MakeInstance()
{
	return MakeShareable(new FUINiagaraComponentDetails);
}

void FUINiagaraComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// UINiagara Category
	IDetailCategoryBuilder& UINiagaraCategory = DetailBuilder.EditCategory(TEXT("UI Niagara"), FText(LOCTEXT("UINiagaraComponentCategory", "UI Niagara")));

	AddRowHeaderContent(UINiagaraCategory, DetailBuilder);
	
	UINiagaraCategory.AddProperty(TEXT("NiagaraSystemAsset"));
	UINiagaraCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	UINiagaraCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	UINiagaraCategory.AddProperty(TEXT("bActivateOnEnable"));
	UINiagaraCategory.AddProperty(TEXT("bDeactivateOnDisable"));
}

#undef LOCTEXT_NAMESPACE
