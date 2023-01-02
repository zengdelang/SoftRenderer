#include "DetailCustomizations/SubComponents/RawImageSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "UIBlueprintEditorModule.h"
#include "Core/Widgets/GraphicSubComponent.h"
#include "Core/Widgets/RawImageSubComponent.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FRawImageSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FRawImageSubComponentCustomization);
}

void FRawImageSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);

	AddProperty(TEXT("Texture"), CategoryBuilder)
		->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	AddProperty(TEXT("Material"), CategoryBuilder);
	AddProperty(TEXT("bRaycastTarget"), CategoryBuilder);
	AddProperty(TEXT("bAntiAliasing"), CategoryBuilder)
		->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
	AddProperty(TEXT("bMaskable"), CategoryBuilder);
	AddProperty(TEXT("UVRect"), CategoryBuilder);
	
	TargetScriptPtr = Cast<URawImageSubComponent>(SubComponent.Get());

	if (TargetScriptPtr.IsValid() && TargetScriptPtr->IsAntiAliasing())
	{
		AddProperty(TEXT("ImageMaskType"), CategoryBuilder)
			->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		if (TargetScriptPtr->GetImageMaskMode() == ERawImageMaskMode::MaskMode_CircleRing)
		{
			AddProperty(TEXT("Thickness"), CategoryBuilder);
		}
	}
	
	if (TargetScriptPtr.IsValid() && IsValid(TargetScriptPtr->GetTexture()))
	{
		AddProperty(TEXT("bPreserveAspect"), CategoryBuilder)
			->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
		if (TargetScriptPtr->GetPreserveAspect())
		{
			AddProperty(TEXT("bFillViewRect"), CategoryBuilder);
			AddProperty(TEXT("OverrideTextureSize"), CategoryBuilder);
		}
		
		/// SetNativeSize
		const auto OnClickedLambda = FOnClicked::CreateLambda([&]
		{
			if (TargetScriptPtr.IsValid())
			{
				const auto RectTransform = Cast<URectTransformComponent>(TargetScriptPtr->GetOuter());
				if (!IsValid(RectTransform))
				{
					return FReply::Handled();
				}

				IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.Broadcast(RectTransform, NSLOCTEXT("RectTransformComponentDetailsSetNativeSize", "ChangeSetNativeSize", "Change SetNativeSize"));
		
				const auto AnchorMin = RectTransform->GetAnchorMin();
				const auto NativeSize = TargetScriptPtr->GetNativeSize();

				const auto AnchorMaxProperty = DetailBuilder.GetProperty(TEXT("AnchorMax"), URectTransformComponent::StaticClass());
				AnchorMaxProperty->SetValue(AnchorMin, EPropertyValueSetFlags::NotTransactable);

				const auto SizeDeltaProperty = DetailBuilder.GetProperty(TEXT("SizeDelta"), URectTransformComponent::StaticClass());
				SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);
				SizeDeltaProperty->SetValue(NativeSize, EPropertyValueSetFlags::NotTransactable);

				IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.Broadcast(RectTransform);
			}

			return FReply::Handled();
		});
		
		CategoryBuilder.AddCustomRow(LOCTEXT("RawImageSubSetNativeSizeRow", ""))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RawImageSubSetNativeSizeTitle", ""))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.HAlign(EHorizontalAlignment::HAlign_Fill)
			[
				SNew(SButton)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Light")
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.OnClicked(OnClickedLambda)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RawImageSubSetNativeSize", "Set Native Size"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			];
	}

	AddProperty(TEXT("bGraying"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("bInvertColor"), CategoryBuilder, EPropertyLocation::Advanced);
	AddProperty(TEXT("RenderOpacity"), CategoryBuilder, EPropertyLocation::Advanced);
}

#undef LOCTEXT_NAMESPACE
