#include "DetailCustomizations/ScrollRectComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "Core/Widgets/ScrollRectComponent.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<IDetailCustomization> FScrollRectComponentDetails::MakeInstance()
{
	return MakeShareable(new FScrollRectComponentDetails);
}

void FScrollRectComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.HideCategory(TEXT("Events"));
	DetailBuilder.HideCategory(TEXT("ScrollRect"));
	DetailBuilder.HideCategory(TEXT("Selectable"));
	
	TArray<TWeakObjectPtr<UObject>> TargetObjects;
	DetailBuilder.GetObjectsBeingCustomized(TargetObjects);
	TargetScriptPtr = Cast<UScrollRectComponent>(TargetObjects[0].Get());

	IDetailCategoryBuilder& ScrollRectCategory = DetailBuilder.EditCategory(TEXT("Scroll Rect"), FText(LOCTEXT("ScrollRectComponentCategory", "Scroll Rect")), ECategoryPriority::Important);

	AddRowHeaderContent(ScrollRectCategory, DetailBuilder);

	ScrollRectCategory.AddProperty(TEXT("ContentPath"));
	
	ScrollRectCategory.AddProperty(TEXT("bHorizontal"));
	ScrollRectCategory.AddProperty(TEXT("bVertical"));

	ScrollRectCategory.AddProperty(TEXT("MovementType"));
	const auto MovementTypeProperty = DetailBuilder.GetProperty(TEXT("MovementType"));
	MovementTypeProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->MovementType == EScrollRectMovementType::MovementType_Elastic)
	{
		ScrollRectCategory.AddProperty(TEXT("Elasticity")).DisplayName(LOCTEXT("ScrollRectElasticityTitle", "\tElasticity"));
	}

	ScrollRectCategory.AddProperty(TEXT("bInertia"));
	const auto InertiaProperty = DetailBuilder.GetProperty(TEXT("bInertia"));
	InertiaProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->bInertia)
	{
		ScrollRectCategory.AddProperty(TEXT("DecelerationRate")).DisplayName(LOCTEXT("ScrollRectDecelerationRateTitle", "\tDecelerationRate"));
	}

	ScrollRectCategory.AddProperty(TEXT("ScrollSensitivity"));

	ScrollRectCategory.AddProperty(TEXT("ViewportPath"));

	ScrollRectCategory.AddProperty(TEXT("HorizontalScrollbarPath"));
	ScrollRectCategory.AddProperty(TEXT("HorizontalScrollbarVisibility")).DisplayName(LOCTEXT("ScrollRectHorizontalScrollbarVisibilityTitle", "\tVisibility"));
	const auto HorizontalScrollbarVisibilityProperty = DetailBuilder.GetProperty(TEXT("HorizontalScrollbarVisibility"));
	HorizontalScrollbarVisibilityProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->GetHorizontalScrollbarVisibility() == EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport)
	{
		ScrollRectCategory.AddProperty(TEXT("HorizontalScrollbarSpacing")).DisplayName(LOCTEXT("ScrollRectHorizontalScrollbarSpacingTitle", "\tSpacing"));
	}

	ScrollRectCategory.AddProperty(TEXT("VerticalScrollbarPath"));
	ScrollRectCategory.AddProperty(TEXT("VerticalScrollbarVisibility")).DisplayName(LOCTEXT("ScrollRectVerticalScrollbarVisibilityTitle", "\tVisibility"));
	const auto VerticalScrollbarVisibilityProperty = DetailBuilder.GetProperty(TEXT("VerticalScrollbarVisibility"));
	VerticalScrollbarVisibilityProperty->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	if (TargetScriptPtr->GetVerticalScrollbarVisibility() == EScrollRectScrollbarVisibility::ScrollbarVisibility_AutoHideAndExpandViewport)
	{
		ScrollRectCategory.AddProperty(TEXT("VerticalScrollbarSpacing")).DisplayName(LOCTEXT("ScrollRectVerticalScrollbarSpacingTitle", "\tSpacing"));
	}
	
	AddEventProperty(ScrollRectCategory, DetailBuilder, TEXT("OnValueChanged"));
}

#undef LOCTEXT_NAMESPACE