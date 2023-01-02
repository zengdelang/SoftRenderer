#pragma once

#include "CoreMinimal.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SSDFFontEditorViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSDFFontEditorViewport) { }
	SLATE_END_ARGS()

public:
	/**
	* Construct this widget
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<class FSDFFontEditor> SDFFontEditor);

protected:
	FReply OnZoomToFitClicked() const;
	
private:
	FPreviewScene PreviewScene;
	TSharedPtr<FUGUIEditorViewportClient> ViewportClient;
	
};
