#include "SSDFFontEditorViewport.h"
#include "SDFFontEditorViewportInfo.h"
#include "UIEditorViewport/SUGUIEditorViewport.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"

#define LOCTEXT_NAMESPACE "SSDFFontEditorViewport"

void SSDFFontEditorViewport::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<FSDFFontEditor> SDFFontEditor)
{
	ViewportClient = MakeShareable(new FUGUIEditorViewportClient(&PreviewScene));
	ViewportClient->SetZoomToFit(true, false);
	ViewportClient->SetAlwaysRealTime(true);

	USDFFontEditorViewportInfo* ViewportInfo = NewObject<USDFFontEditorViewportInfo>(GetTransientPackage());
	ViewportInfo->SDFFontEditor = SDFFontEditor;
	ViewportClient->SetupViewportInfo(NAME_None, nullptr, nullptr, ViewportInfo);
	
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
					SNew(SUGUIEditorViewport, TEXT("SDFFontEditorViewport"), ViewportClient.ToSharedRef())
				]
			]
		]
	];
}

FReply SSDFFontEditorViewport::OnZoomToFitClicked() const
{
	if( ViewportClient.IsValid() )
	{
		ViewportClient->SetZoomToFit(true, false);
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
