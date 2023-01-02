#include "DetailCustomizations/UIStaticMeshComponentDetails.h"
#include "AssetSelection.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/GraphicComponent.h"
#include "Core/Widgets/Mesh/UIStaticMeshComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FUIStaticMeshComponentDetails::MakeInstance()
{
	return MakeShareable(new FUIStaticMeshComponentDetails);
}

void FUIStaticMeshComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Graphic"));

	/// UIStaticMesh Category
	IDetailCategoryBuilder& UIStaticMeshCategory = DetailBuilder.EditCategory(TEXT("UI Static Mesh"), FText(LOCTEXT("UIStaticMeshComponentCategory", "UI Static Mesh")));

	AddRowHeaderContent(UIStaticMeshCategory, DetailBuilder);
	
	UIStaticMeshCategory.AddProperty(TEXT("StaticMesh"));
	UIStaticMeshCategory.AddProperty(TEXT("bRaycastTarget"), UGraphicComponent::StaticClass());
	UIStaticMeshCategory.AddProperty(TEXT("bMaskable"), UMaskableGraphicComponent::StaticClass());

	DetailBuilder.GetProperty(TEXT("StaticMesh"))
		->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UUIStaticMeshComponent>(TargetObjects[0].Get());

	AddUIPrimitiveDetails(DetailBuilder, UIStaticMeshCategory);
}

#undef LOCTEXT_NAMESPACE
