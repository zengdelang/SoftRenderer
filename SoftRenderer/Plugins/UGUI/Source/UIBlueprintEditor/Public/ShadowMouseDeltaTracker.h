#pragma once

#include "CoreMinimal.h"
#include "UnrealWidget.h"

class FCanvas;
class FDragTool;
class FEditorViewportClient;
class FPrimitiveDrawInterface;
class FSceneView;
struct FInputEventState;

/**
 * Keeps track of mouse movement deltas in the viewports.
 */
class UIBLUEPRINTEDITOR_API FShadowMouseDeltaTracker
{
public:

	FShadowMouseDeltaTracker();
	~FShadowMouseDeltaTracker();
	
public:
	/** The unsnapped start position of the current mouse drag. */
	FVector Start;
	/** The snapped start position of the current mouse drag. */
	FVector StartSnapped;
	/** The screen space start position of the current mouse drag (may be scaled or rotated according to the ortho zoom or view). */
	FVector StartScreen;
	/** The unsnapped end position of the current mouse drag. */
	FVector End;
	/** The snapped end position of the current mouse drag. */
	FVector EndSnapped;
	/** The screen space end position of the current mouse drag (may be scaled or rotated according to the ortho zoom or view). */
	FVector EndScreen;
	/** The raw unscaled mouse delta in pixels */
	FVector RawDelta;

	/** The amount that the End vectors have been reduced by since dragging started, this is added to the deltas to get an absolute delta. */
	FVector ReductionAmount;

	/**
	 * If there is a dragging tool being used, this will point to it.
	 * Gets newed/deleted in StartTracking/EndTracking.
	 */
	TSharedPtr<FDragTool> DragTool;


	/** Keeps track of whether AddDelta has been called since StartTracking. */
	bool bHasReceivedAddDelta;

	/** True if we attempted to use a drag tool since StartTracking was called */
	bool bHasAttemptedDragTool;

	/** Tracks whether keyboard/mouse wheel/etc have caused simulated mouse movement.  Reset on StartTracking */
	bool bExternalMovement;

	/** Tracks if the user used a modifier to drag a selected item. (Rather than using a widget handle).Reset on StartTracking */
	bool bUsedDragModifier;

	/** Tracks whether the drag tool is in the process of being deleted (to protect against reentrancy) */
	bool bIsDeletingDragTool;

	/** Stores the widget mode active when the tracker begins tracking to help stop it change mid-track */
	FWidget::EWidgetMode TrackingWidgetMode;

};
