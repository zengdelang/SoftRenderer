#pragma once

#include "CoreMinimal.h"
#include "UIBlueprintEditor/Public/UIEditorViewport/UGUIEditorViewportClient.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SSpriteAtlasVisualizer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SSpriteAtlasVisualizer ) {}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

protected:
	FReply OnFitToWindowClicked();
	
private:
	FPreviewScene PreviewScene;
	TSharedPtr<FUGUIEditorViewportClient> ViewportClient;
	
};
