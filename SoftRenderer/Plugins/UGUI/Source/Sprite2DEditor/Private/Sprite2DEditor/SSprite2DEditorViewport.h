#pragma once

#include "CoreMinimal.h"
#include "UIEditorViewport/UGUIEditorViewportClient.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SSprite2DEditorViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSprite2DEditorViewport) { }
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<FUICommandList>& InCommandList, TWeakPtr<class FSprite2DEditor> Sprite2DEditor);

public:
	void UpdatePreviewSceneActor() const;
	
protected:
	FReply OnZoomToFitClicked() const;
	AActor* GetPreviewSceneActor() const;

private:
	FPreviewScene PreviewScene;
	TSharedPtr<FUGUIEditorViewportClient> ViewportClient;
	
};
