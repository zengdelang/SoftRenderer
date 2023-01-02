#include "DetailCustomizations/UISimpleStaticMeshComponentDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/MaskableGraphicComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUISimpleStaticMeshComponentDetails::MakeInstance()
{
	return MakeShareable(new FUISimpleStaticMeshComponentDetails);
}

void FUISimpleStaticMeshComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// UISimpleStaticMesh Category
	IDetailCategoryBuilder& UISimpleStaticMeshCategory = DetailBuilder.EditCategory(TEXT("UI Simple Static Mesh"), FText(LOCTEXT("UISimpleStaticMeshComponentCategory", "UI Simple Static Mesh")));

	AddRowHeaderContent(UISimpleStaticMeshCategory, DetailBuilder);
	
	UISimpleStaticMeshCategory.AddProperty(TEXT("MeshRelativeLocation"));
	UISimpleStaticMeshCategory.AddProperty(TEXT("MeshRelativeRotation"));
	UISimpleStaticMeshCategory.AddProperty(TEXT("MeshRelativeScale3D"));
	UISimpleStaticMeshCategory.AddProperty(TEXT("StaticMesh"));
	UISimpleStaticMeshCategory.AddProperty(TEXT("Texture"));
	UISimpleStaticMeshCategory.AddProperty(TEXT("Color"), UGraphicComponent::StaticClass());
	UISimpleStaticMeshCategory.AddProperty(TEXT("Material"), UGraphicComponent::StaticClass());
	UISimpleStaticMeshCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	UISimpleStaticMeshCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());
	UISimpleStaticMeshCategory.AddProperty(TEXT("bAlignBoundingBoxCenter"));
}

#undef LOCTEXT_NAMESPACE
