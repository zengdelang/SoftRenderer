#include "DetailCustomizations/SubComponents/CanvasSubComponentCustomization.h"
#include "DetailCategoryBuilder.h"
#include "Core/Render//CanvasSubComponent.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCustomizations/SubComponentsDetailRow/ISubComponentDetailCustomization.h"

#define LOCTEXT_NAMESPACE "UGUIEditor"

TSharedRef<ISubComponentDetailCustomization> FCanvasSubComponentCustomization::MakeInstance()
{
	return MakeShareable(new FCanvasSubComponentCustomization);
}

void FCanvasSubComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder,
	IDetailCategoryBuilder& CategoryBuilder)
{
	TargetScriptPtr = Cast<UCanvasSubComponent>(SubComponent);
	
	AddRowHeaderContent(CategoryBuilder, DetailBuilder);
	
	AddProperty(TEXT("ProjectionType"), CategoryBuilder);
	
	if (TargetScriptPtr.IsValid())
	{
		bool bIsRootCanvas = true;
		bool bOverrideSorting = false;
		if (TargetScriptPtr->IsTemplate())
		{
			TArray<UObject*> Instances;
			TargetScriptPtr->GetArchetypeInstances(Instances);

			for (const auto& Instance : Instances)
			{
				const auto CanvasInstance = Cast<UCanvasSubComponent>(Instance);
				if (IsValid(CanvasInstance) && CanvasInstance->GetWorld()->WorldType == EWorldType::EditorPreview)
				{
					if (!CanvasInstance->IsRootCanvas() && CanvasInstance->IsOverrideSorting())
					{
						bOverrideSorting = true;
					}

					if (!CanvasInstance->IsRootCanvas() && !CanvasInstance->IsOverrideSorting())
					{
						bIsRootCanvas = false;
						bOverrideSorting = false;
					}
				}
			}
		}
		else
		{
			bIsRootCanvas = TargetScriptPtr->IsRootCanvas();
		}
		
		if (bIsRootCanvas)
		{
			AddProperty(TEXT("RenderMode"), CategoryBuilder)
				->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			if (TargetScriptPtr.IsValid() && TargetScriptPtr->GetRenderMode() != ECanvasRenderMode::CanvasRenderMode_ScreenSpaceOverlay)
			{
				AddProperty(TEXT("bPerformFrustumCull"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_bPerformFrustumCull", "\tPerform Frustum Cull"));
				AddProperty(TEXT("bCheckSceneViewVisibility"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_bCheckSceneViewVisibility", "\tCheck Scene View Visibility"));
			}

			if (!bOverrideSorting)
			{
				AddProperty(TEXT("bPixelPerfect"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_bPixelPerfect", "\tPixel Perfect"));
			}
			
			if (bOverrideSorting)
			{
				AddProperty(TEXT("CanvasPixelPerfectInheritMode"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_CanvasPixelPerfectInheritMode", "Pixel Perfect"));	
		
				const auto OverrideSortingProperty = AddProperty(TEXT("bOverrideSorting"), CategoryBuilder);
				OverrideSortingProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

				AddProperty(TEXT("SortingOrder"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_SortingOrder", "\tSorting Order"));	
			}

			CategoryBuilder.AddCustomRow(LOCTEXT("CanvasSubComponent_RenderingRow", ""))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
				.Text(LOCTEXT("CanvasSubComponent_Rendering", "Rendering"))
			]
			.ValueContent()
			[
				SNew(SSpacer)
			];
			
			const auto RenderTargetModeProperty = AddProperty(TEXT("RenderTargetMode"), CategoryBuilder);
			RenderTargetModeProperty->DisplayName(LOCTEXT("CanvasSubComponent_RenderTargetMode", "\tRender Target Mode"));
			RenderTargetModeProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));

			if (TargetScriptPtr.IsValid())
			{
				if (TargetScriptPtr->GetRenderTargetMode() == ECanvasRenderTargetMode::ExternalTarget)
				{
					AddProperty(TEXT("RenderTimes"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_RenderTimes", "\tRender Times"));
					AddProperty(TEXT("RenderTarget"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_RenderTarget", "\tRender Target"));
				}
			}
		}
		else
		{
			AddProperty(TEXT("CanvasPixelPerfectInheritMode"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_CanvasPixelPerfectInheritMode", "Pixel Perfect"));	
			
			const auto OverrideSortingProperty = AddProperty(TEXT("bOverrideSorting"), CategoryBuilder);
			OverrideSortingProperty->GetPropertyHandle()->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&] { DetailBuilder.ForceRefreshDetails(); }));
			
			if (TargetScriptPtr->IsOverrideSorting())
			{
				AddProperty(TEXT("SortingOrder"), CategoryBuilder);
			}
		}
	}
	
	CategoryBuilder.AddCustomRow(LOCTEXT("CanvasSubComponent_MaterialsRow", ""))
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
		.Text(LOCTEXT("CanvasSubComponent_Materials", "Materials"))
	]
	.ValueContent()
	[
		SNew(SSpacer)
	];

	AddProperty(TEXT("DefaultMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_DefaultMaterial", "\tDefault Material"));
	AddProperty(TEXT("DefaultAAMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_DefaultAAMaterial", "\tDefault Anti-aliasing Material"));
	AddProperty(TEXT("DefaultTextMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_DefaultTextMaterial", "\tDefault Text Material"));
	AddProperty(TEXT("EditorPreviewDefaultMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_EditorPreviewDefaultMaterial", "\tEditor Preview Default Material"));
	AddProperty(TEXT("EditorPreviewDefaultAAMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_EditorPreviewDefaultAAMaterial", "\tEditor Preview Default Anti-aliasing Material"));
	AddProperty(TEXT("EditorPreviewDefaultTextMaterial"), CategoryBuilder)->DisplayName(LOCTEXT("CanvasSubComponent_EditorPreviewDefaultTextMaterial", "\tEditor Preview Default Text Material"));
}

#undef LOCTEXT_NAMESPACE
