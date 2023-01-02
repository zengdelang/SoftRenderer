#pragma once

#include "CoreMinimal.h"
#include "SpriteAtlasPackerPrivate.h"
#include "UIEditorViewport/SUGUIEditorViewport.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SSpriteAtlasPackerViewport : public SSpriteAtlasPackerBaseWidget
{
public:
	SLATE_BEGIN_ARGS(SSpriteAtlasPackerViewport) { }
	SLATE_END_ARGS()

public:

	/**
	* Construct this widget
	*
	* @param InArgs The declaration data for this widget.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList);

protected:
	void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay);

	FReply OnZoomToFitClicked();
	
private:
	FPreviewScene PreviewScene;
	TSharedPtr<FUGUIEditorViewportClient> ViewportClient;
	
};
