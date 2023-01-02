#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "UnrealWidget.h"
#include "EditorViewportClient.h"
#include "Designer/DesignerEditorEventViewportClient.h"

class FUIBlueprintEditor;
class FCanvas;
class FPreviewScene;
class FScopedTransaction;
class SSCSUIEditorViewport;
class UStaticMeshComponent;

/**
 * An editor viewport client subclass for the SCS UI editor viewport.
 */
class UIBLUEPRINTEDITOR_API FSCSUIEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FSCSUIEditorViewportClient>
{
public:
	/**
	 * Constructor.
	 *
	 * @param InBlueprintEditorPtr A weak reference to the Blueprint Editor context.
	 * @param InPreviewScene The preview scene to use.
	 */
	FSCSUIEditorViewportClient(TWeakPtr<class FUIBlueprintEditor>& InBlueprintEditorPtr, FPreviewScene* InPreviewScene, const TSharedRef<SSCSUIEditorViewport>& InSCSEditorViewport);

	/**
	 * Destructor.
	 */
	virtual ~FSCSUIEditorViewportClient() override;

	// FEditorViewportClient interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI) override;
	virtual void DrawCanvas( FViewport& InViewport, FSceneView& View, FCanvas& Canvas ) override;
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad=false) override;
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override;
	virtual bool InputWidgetDelta( FViewport* Viewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale ) override;
	virtual void TrackingStarted( const struct FInputEventState& InInputState, bool bIsDragging, bool bNudge ) override;
	virtual void TrackingStopped() override;
	virtual FWidget::EWidgetMode GetWidgetMode() const override;
	virtual void SetWidgetMode( FWidget::EWidgetMode NewMode ) override;
	virtual void SetWidgetCoordSystemSpace( ECoordSystem NewCoordSystem ) override;
	virtual FVector GetWidgetLocation() const override;
	virtual FMatrix GetWidgetCoordSystem() const override;
	virtual ECoordSystem GetWidgetCoordSystemSpace() const override { return WidgetCoordSystem; }
	virtual int32 GetCameraSpeedSetting() const override;
	virtual void SetCameraSpeedSetting(int32 SpeedSetting) override;

#if SUPPORT_UI_BLUEPRINT_EDITOR
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass = eSSP_FULL) override;

	virtual bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

	/** FViewElementDrawer interface */
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
#endif

	virtual EMouseCursor::Type GetCursor(FViewport* InViewport,int32 X,int32 Y) override;

	virtual bool InputChar(FViewport* InViewport,int32 ControllerId, TCHAR Character) override;
	virtual bool InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex) override;
	virtual void LostFocus(FViewport* InViewport) override;
	
	/** 
	 * Recreates the preview scene and invalidates the owning viewport.
	 *
	 * @param bResetCamera Whether or not to reset the camera after recreating the preview scene.
	 */
	void InvalidatePreview(bool bResetCamera = true);

	/**
	 * Resets the camera position
	 */
	void ResetCamera();

	/**
	 * Determines whether or not realtime preview is enabled.
	 * 
	 * @return true if realtime preview is enabled, false otherwise.
	 */
	bool GetRealtimePreview() const
	{
		return IsRealtime();
	}

	/**
	 * Toggles realtime preview on/off.
	 */
	void ToggleRealtimePreview();

	/**
	 * Focuses the viewport on the selected components
	 */
	void FocusViewportToSelection();

	/**
	 * Returns true if simulate is enabled in the viewport
	 */
	bool GetIsSimulateEnabled() const;

	/**
	 * Will toggle the simulation mode of the viewport
	 */
	void ToggleIsSimulateEnabled();
	
	/*
	 * Will toggle the show the raycast regions
	 */
	void ToggleShowRaycastRegion() const;

	/*
	 * Return true if the raycast regions is showing
	 */
	bool IsShowRaycast() const;

	/*
	 * Will toggle trace the selected item
	 */
	void ToggleTrackSelectedComponent() const;

	/*
	 * Return true if trace the selected item
	 */
	bool GetTrackSelectedComponent() const;

	/*
	 * Will toggle Edit mode
	 */
	void ToggleRawEditMode() const;

	/*
	 * Return true if the editMode  is raw
	 */
	bool GetRawEditMode() const;

	/*
	 * Will toggle show background image
	 */
	void ToggleShowBackgroundImage();

	/*
	 * Return true if show the background image
	 */
	bool GetShowBackgroundImage();

	/*
	 * Will toggle show the stats
	 */
	void ToggleShowStats();

	/*
	 * Return true if show the stats
	 */
	bool GetShowStats();
	
	/**
	 * Gets the current preview actor instance.
	 */
	AActor* GetPreviewActor() const;

	/*
	 * Reset the zoom
	 */
	void ResetZoom();

	TWeakPtr<class FUIBlueprintEditor> GetBlueprintEditor() const
	{
		return BlueprintEditorPtr;
	}

protected:
	void ZoomToFit();
	
public:
	/**
	 * Initiates a transaction.
	 */
	void BeginTransaction(const FText& Description);

	/**
	 * Ends the current transaction, if one exists.
	 */
	void EndTransaction();

	/**
	 * Updates preview bounds and floor positioning
	 */
	void RefreshPreviewBounds();

protected:
	void OnUIBlueprintEditorBeginTransaction(UActorComponent* ActorComponent, const FText& Description);
	void OnUIBlueprintEditorEndTransaction(UActorComponent* ActorComponent);
	
public:
	FVector2D EditorViewportSize;
	FMatrix ViewProjectionMatrix;

	bool bUpdateWidgetActorComponents = true;

	EMouseCursor::Type ViewportMouseCursor = EMouseCursor::Type::None;
	
	TWeakObjectPtr<UDesignerEditorEventViewportClient> EventViewportClient;
	TWeakPtr<SSCSUIEditorViewport> UIEditorViewport;
	
private:
	FWidget::EWidgetMode WidgetMode;
	ECoordSystem WidgetCoordSystem;

	/** Weak reference to the editor hosting the viewport */
	TWeakPtr<class FUIBlueprintEditor> BlueprintEditorPtr;

	/** The full bounds of the preview scene (encompasses all visible components) */
	FBoxSphereBounds PreviewActorBounds;

	bool bZoomToFit = true;
	bool bAnimationZoom = false;
	FVector2D ZoomOffset = FVector2D(200, 200);

	/** If true then we are manipulating a specific property or component */
	bool bIsManipulating;

	bool bRegisterTransactionDelegate;

	/** The current transaction for undo/redo */
	FScopedTransaction* ScopedTransaction;

	/** If true, the physics simulation gets ticked */
	bool bIsSimulateEnabled;
};
