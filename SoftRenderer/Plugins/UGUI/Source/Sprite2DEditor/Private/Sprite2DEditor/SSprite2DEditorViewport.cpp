#include "SSprite2DEditorViewport.h"
#include "Sprite2DEditorViewportInfo.h"
#include "PreviewViewport/SpriteEditorWidgetActor.h"
#include "UIEditorViewport/SUGUIEditorViewport.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"
#include "EngineUtils.h"

#define LOCTEXT_NAMESPACE "SSprite2DEditorViewport"

void SSprite2DEditorViewport::Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<FSprite2DEditor> Sprite2DEditor)
{
	ViewportClient = MakeShareable(new FUGUIEditorViewportClient(&PreviewScene));
	ViewportClient->SetZoomToFit(true, false);
	ViewportClient->SetAlwaysRealTime(true);

	USprite2DEditorViewportInfo* ViewportInfo = NewObject<USprite2DEditorViewportInfo>(GetTransientPackage());
	ViewportInfo->Sprite2DEditor = Sprite2DEditor;
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
					SNew(SUGUIEditorViewport, TEXT("SpriteEditorViewport"), ViewportClient.ToSharedRef())
				]
			]
		]
	];
}

FReply SSprite2DEditorViewport::OnZoomToFitClicked() const
{
	if( ViewportClient.IsValid() )
	{
		ViewportClient->SetZoomToFit(true, false);
	}

	return FReply::Handled();
}

AActor* SSprite2DEditorViewport::GetPreviewSceneActor() const
{
	UWorld* PreviewWorld = PreviewScene.GetWorld();
	for ( TActorIterator<AActor> It(PreviewWorld); It; ++It )
	{
		AActor* Actor = *It;
		if ( !Actor->IsPendingKillPending())
		{
			if(Cast<ASpriteEditorWidgetActor>(Actor))
			{
				return Actor;
			}
		}
	}
	return nullptr;
}

void SSprite2DEditorViewport::UpdatePreviewSceneActor() const
{
	ASpriteEditorWidgetActor* Actor = Cast<ASpriteEditorWidgetActor>(GetPreviewSceneActor());
	if(Actor)
	{
		Actor->UpdateActor();
	}
}

#undef LOCTEXT_NAMESPACE
