#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"

//////////////////////////////////////////////////////////////////////////
// FUGUIEditorViewportClient

class UIBLUEPRINTEDITOR_API FUGUIEditorViewportClient : public FEditorViewportClient, public TSharedFromThis<FUGUIEditorViewportClient>
{
public:
	FUGUIEditorViewportClient(FPreviewScene* InPreviewScene);

	/**
	 * Destructor.
	 */
	virtual ~FUGUIEditorViewportClient() override;

public:
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
	
	virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass = eSSP_FULL) override;

	virtual bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;

	/** FViewElementDrawer interface */
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;
	
	virtual EMouseCursor::Type GetCursor(FViewport* InViewport,int32 X,int32 Y) override;

	virtual bool InputChar(FViewport* InViewport,int32 ControllerId, TCHAR Character) override;
	virtual bool InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex) override;
	virtual void LostFocus(FViewport* InViewport) override;

protected:
	void ZoomToFit();
	
public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		FEditorViewportClient::AddReferencedObjects(Collector);
		
		if (EventViewportClient)
		{
			Collector.AddReferencedObject(EventViewportClient);
		}
	}

public:
	void SetupViewportInfo(FName ViewportName, TWeakPtr<FUGUIEditorViewportClient> InViewportClient,
		TWeakPtr<class SUGUIEditorViewport> InEditorViewport, class UUGUIEditorViewportInfo* ViewportInfo = nullptr);

	virtual void InvalidatePreview(bool bResetCamera = true) {}
	
public:
	/**
	 * Determines whether or not realtime preview is enabled.
	 * 
	 * @return true if realtime preview is enabled, false otherwise.
	 */
	bool GetRealtimePreview() const
	{
		return IsRealtime();
	}

	void SetViewportMouseCursor(EMouseCursor::Type InViewportMouseCursor)
	{
		ViewportMouseCursor = InViewportMouseCursor;
	}

	const FMatrix& GetViewProjectionMatrix() const
	{
		return ViewProjectionMatrix;
	}

	const FVector2D& GetEditorViewportSize() const
	{
		return EditorViewportSize;
	}

	void SetZoomToFit(bool bInZoomToFit, bool bInAnimationZoomFit)
	{
		bZoomToFit = bInZoomToFit;
		bAnimationZoom = bInAnimationZoomFit;
	}

	void SetZoomOffset(FVector2D InZoomOffset)
	{
		ZoomOffset = InZoomOffset;
	}

	void SetTargetViewSize(FVector2D InTargetViewSize)
	{
		TargetViewSize = InTargetViewSize;
	}

	void SetAlwaysRealTime(bool bInAlwaysRealTime)
	{
		bAlwaysRealTime = bInAlwaysRealTime;
	}

private:
	bool bZoomToFit = false;
	bool bAnimationZoom = false;
	FVector2D ZoomOffset = FVector2D(0, 0);
	FVector2D TargetViewSize = FVector2D(-1, -1);

	bool bAlwaysRealTime = false;
	
	class UDesignerEditorEventViewportClient* EventViewportClient = nullptr;
		
	FVector2D EditorViewportSize;
	FMatrix ViewProjectionMatrix;
	
	EMouseCursor::Type ViewportMouseCursor = EMouseCursor::Type::None;
	
	FWidget::EWidgetMode WidgetMode;
	ECoordSystem WidgetCoordSystem;

	/** If true then we are manipulating a specific property or component */
	bool bIsManipulating;

	/** If true, the physics simulation gets ticked */
	bool bIsSimulateEnabled;

};
