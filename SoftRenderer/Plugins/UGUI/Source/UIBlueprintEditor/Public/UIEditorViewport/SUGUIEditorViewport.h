#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SEditorViewport.h"
#include "UGUIEditorViewportClient.h"

DECLARE_DELEGATE_OneParam(FOnPopulateViewportOverlays, TSharedRef<SOverlay> Overlay);

class UIBLUEPRINTEDITOR_API SUGUIEditorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SUGUIEditorViewport){}
		SLATE_EVENT( FOnPopulateViewportOverlays, OnPopulateViewportOverlays )
	SLATE_END_ARGS()

	/**
	 * Constructs this widget with the given parameters.
	 *
	 * @param InArgs Construction parameters.
	 */
	void Construct(const FArguments& InArgs, FName ViewportName, TSharedRef<class FUGUIEditorViewportClient> InViewportClient);

	/**
	 * Destructor.
	 */
	virtual ~SUGUIEditorViewport() override;

	/**
	 * Invalidates the viewport client
	 */
	void Invalidate();

	/**
	 * Request a refresh of the preview scene/world. Will recreate actors as needed.
	 *
	 * @param bResetCamera If true, the camera will be reset to its default position based on the preview.
	 * @param bRefreshNow If true, the preview will be refreshed immediately. Otherwise, it will be deferred until the next tick (default behavior).
	 */
	void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);

	virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
	
	// SWidget interface
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;
	// End of SWidget interface

protected:
	/** SEditorViewport interface */
	virtual TSharedRef<class FEditorViewportClient> MakeEditorViewportClient() override;

private:
	/** One-off active timer to update the preview */
	EActiveTimerReturnType DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera);

private:
	/** Viewport client */
	TSharedPtr<class FUGUIEditorViewportClient> ViewportClient;

	/** Whether the active timer (for updating the preview) is registered */
	bool bIsActiveTimerRegistered = false;

	/** Handle to the registered OnPreviewFeatureLevelChanged delegate. */
	FDelegateHandle PreviewFeatureLevelChangedHandle;

public:
	FOnPopulateViewportOverlays OnPopulateViewportOverlays;
	
};
