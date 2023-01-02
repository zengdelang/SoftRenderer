#include "AtlasVisualizer/SSpriteAtlasVisualizer.h"
#include "UIBlueprintEditor/Public/UIEditorViewport/SUGUIEditorViewport.h"

#define LOCTEXT_NAMESPACE "AtlasVisualizer"

void SSpriteAtlasVisualizer::Construct( const FArguments& InArgs )
{
	ViewportClient = MakeShareable(new FUGUIEditorViewportClient(&PreviewScene));
	ViewportClient->SetZoomToFit(true, false);
	ViewportClient->SetAlwaysRealTime(true);
	
	TSharedPtr<SViewport> Viewport;
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoHeight()
			[
				SNew( SHorizontalBox )
				+ SHorizontalBox::Slot()
				.Padding(2.0f)
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew( SButton )
					.Text(LOCTEXT("FitToWindow", "Fit to Window"))
					.OnClicked( this, &SSpriteAtlasVisualizer::OnFitToWindowClicked )
				]
			]
			+ SVerticalBox::Slot()
			.Padding(0)
			[
				SNew( SOverlay )
				+ SOverlay::Slot()
				[
					SNew(SUGUIEditorViewport, TEXT("SpriteAtlasViewport"), ViewportClient.ToSharedRef())
				]
			]
		]
	];
}

FReply SSpriteAtlasVisualizer::OnFitToWindowClicked()
{
	if( ViewportClient.IsValid() )
	{
		ViewportClient->SetZoomToFit(true, false);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
