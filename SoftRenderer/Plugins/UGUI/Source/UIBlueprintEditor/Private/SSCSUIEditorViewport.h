#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SCSUIEditorViewportClient.h"
#include "SEditorViewport.h"
#include "UIBlueprintEditor.h"
#include "Core/UICommonDefinitions.h"
#include "SViewportToolBarComboMenu.h"

class SMenuAnchor;

/**
 * Implements the viewport widget that's hosted in the SCS component editor tab.
 */
class SSCSUIEditorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SSCSUIEditorViewport){}
		SLATE_ARGUMENT(TWeakPtr<class FUIBlueprintEditor>, BlueprintEditor)
	SLATE_END_ARGS()

	/**
	 * Constructs this widget with the given parameters.
	 *
	 * @param InArgs Construction parameters.
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * Destructor.
	 */
	virtual ~SSCSUIEditorViewport();

	/**
	 * Invalidates the viewport client
	 */
	void Invalidate();

	/**
	 * Sets whether or not the preview should be enabled.
	 *
	 * @param bEnable Whether or not to enable the preview.
	 */
	void EnablePreview(bool bEnable);

	/**
	 * Request a refresh of the preview scene/world. Will recreate actors as needed.
	 *
	 * @param bResetCamera If true, the camera will be reset to its default position based on the preview.
	 * @param bRefreshNow If true, the preview will be refreshed immediately. Otherwise, it will be deferred until the next tick (default behavior).
	 */
	void RequestRefresh(bool bResetCamera = false, bool bRefreshNow = false);

	// SWidget interface
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End of SWidget interface

	/**
	 * Called when the selected component changes in the SCS editor.
	 */
	void OnComponentSelectionChanged();

	/**
	 * Focuses the viewport on the currently selected components
	 */
	virtual void OnFocusViewportToSelection() override;

	/**
	 * Returns true if simulation is enabled for the viewport
	 */
	bool GetIsSimulateEnabled();

	/*
     * Will toggle the show the raycast regions
     */
	void ToggleShowRaycast();

	/*
	 * Will toggle the raw edit Mode
	 */
	void ToggleRawEditMode();

	/*
     * Returns true if Edit Mode is raw
     */
	bool GetRawEditMode();

	/*
	 * Will toggle show background image
	 */
	void ToggleShowBackgroundImage();

	/*
	 * Return true if show the background image
	 */
	bool GetShowBackgroundImage();

	/*
	 * Will adjust zoom to fit viewport
	 */
	void ResetZoom();

	/*
	 * Will toggle show stats
	 */
	void ToggleShowStats();

	/*
	 * Return true if show the stats
	 */
	bool GetShowStats();

	/** Struct defining the text and its style of each item in the overlay widget */
	struct FOverlayTextItem
	{
		explicit FOverlayTextItem(const FText& InText, const FName& InStyle = "TextBlock.ShadowedText")
			: Text(InText), Style(InStyle)
		{}

		FText Text;
		FName Style;
	};

	void PopulateOverlayText( const TArray<FOverlayTextItem>& TextItems );

	void SetOwnerTab(TSharedRef<SDockTab> Tab);

	TSharedPtr<SDockTab> GetOwnerTab() const;

	virtual void Toggle2DViewMode(bool b2DViewMode);
	virtual void ToggleOverlay(bool bOverlay);
	virtual void UpdatePreviewActorVisibility();
	
	bool Is2DViewportType() const
	{
		if (ViewportClient.IsValid())
		{
			return ViewportClient->ViewportType == LVT_OrthoXZ;
		}
		return false;
	}

	bool IsShowRaycast() const
	{
	    if(ViewportClient.IsValid())
	    {
			return ViewportClient->IsShowRaycast();
	    }
		return false;
	}

	bool IsSimulateGameOverlay() const
	{
		return bSimulateGameOverlay;
	}

protected:
	/**
	 * Determines if the viewport widget is visible.
	 *
	 * @return true if the viewport is visible; false otherwise.
	 */
	bool IsVisible() const override;

	/**
	 * Returns true if the viewport commands should be shown
	 */
	bool ShouldShowViewportCommands() const;

	/** Called when the simulation toggle command is fired */
	void ToggleIsSimulateEnabled();

	void UpdateCanvasRenderMode();

	/** SEditorViewport interface */
	virtual TSharedRef<class FEditorViewportClient> MakeEditorViewportClient() override;
	virtual TSharedPtr<class SWidget> MakeViewportToolbar() override;
	virtual void PopulateViewportOverlays(TSharedRef<class SOverlay> Overlay) override;
	virtual void BindCommands() override;

private:
	/** One-off active timer to update the preview */
	EActiveTimerReturnType DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera);

private:
	/** Pointer back to editor tool (owner) */
	TWeakPtr<class FUIBlueprintEditor> BlueprintEditorPtr;

	/** Viewport client */
	TSharedPtr<class FSCSUIEditorViewportClient> ViewportClient;
	
	/** Whether the active timer (for updating the preview) is registered */
	bool bIsActiveTimerRegistered;

	bool bSimulateGameOverlay;

	/** The owner dock tab for this viewport. */
	TWeakPtr<SDockTab> OwnerTab;

	ECanvasRenderMode LastCanvasRenderMode;
	
	/** Pointer to the vertical box into which the overlay text items are added */
	TSharedPtr<SVerticalBox> OverlayTextVerticalBox;

	/** Handle to the registered OnPreviewFeatureLevelChanged delegate. */
	FDelegateHandle PreviewFeatureLevelChangedHandle;
};
