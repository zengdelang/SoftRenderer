#include "DetailCustomizations/SubComponents/UISimpleStaticMeshSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FUISimpleStaticMeshSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FUISimpleStaticMeshSubComponentCustomization);
}

void FUISimpleStaticMeshSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("MeshRelativeLocation"), CategoryBuilder);
	AddProperty(TEXT("MeshRelativeRotation"), CategoryBuilder);
	AddProperty(TEXT("MeshRelativeScale3D"), CategoryBuilder);
	AddProperty(TEXT("StaticMesh"), CategoryBuilder);
	AddProperty(TEXT("Texture"), CategoryBuilder);
	AddProperty(TEXT("Color"), CategoryBuilder);
	AddProperty(TEXT("Material"), CategoryBuilder);
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bMaskable"), CategoryBuilder);
	AddProperty(TEXT("bAlignBoundingBoxCenter"), CategoryBuilder);

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
