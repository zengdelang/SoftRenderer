#include "DetailCustomizations/SubComponents/UIStaticMeshSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/Mesh/UIStaticMeshSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FUIStaticMeshSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FUIStaticMeshSubComponentCustomization);
}

void FUIStaticMeshSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("StaticMesh"), CategoryBuilder)->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bMaskable"), CategoryBuilder);
	
	TargetScriptPtr = Cast<UUIStaticMeshSubComponent>(SubComponent.Get());

	AddUIPrimitiveDetails(DetailBuilder, CategoryBuilder);

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
