#include "SSpriteAtlasPackerViewport.h"
#include "UIEditorViewport/SUGUIEditorViewport.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"

#define LOCTEXT_NAMESPACE "SSpriteAtlasPackerViewport"

void SSpriteAtlasPackerViewport::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList)
{
	ViewportClient = MakeShareable(new FUGUIEditorViewportClient(&PreviewScene));
	ViewportClient->SetZoomToFit(true, false);
	ViewportClient->SetAlwaysRealTime(true);
	
	TSharedPtr<SViewport> Viewport;
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(0)
		[
			SNew( SVerticalBox )
			+ SVerticalBox::Slot()
			.Padding(0)
			[
				SNew( SOverlay )
				+ SOverlay::Slot()
				.Padding(0)
				[
					SNew(SUGUIEditorViewport, TEXT("SpriteAtlasPackerViewport"), ViewportClient.ToSharedRef())
					.OnPopulateViewportOverlays_Raw(this, &SSpriteAtlasPackerViewport::PopulateViewportOverlays)
				]
			]
		]
	];
}

void SSpriteAtlasPackerViewport::PopulateViewportOverlays(TSharedRef<SOverlay> Overlay)
{
	Overlay->AddSlot()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(0, 2.0f)
				.AutoWidth()
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Right)
				[
					SNew(SButton)
					.ButtonStyle(FEditorStyle::Get(), "ViewportMenu.Button")
					.ToolTipText(LOCTEXT("ZoomToFit_ToolTip", "Zoom To Fit"))
					.OnClicked(this, &SSpriteAtlasPackerViewport::OnZoomToFitClicked)
					.ContentPadding(FEditorStyle::Get().GetMargin("ViewportMenu.SToolBarButtonBlock.Button.Padding"))
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("UMGEditor.ZoomToFit"))
					]
				]
			]
		];
}

FReply SSpriteAtlasPackerViewport::OnZoomToFitClicked()
{
	if( ViewportClient.IsValid() )
	{
		ViewportClient->SetZoomToFit(true, false);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
