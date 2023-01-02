#include "UIEditorViewport/UGUIEditorViewportClient.h"
#include "EngineUtils.h"
#include "ShadowMouseDeltaTracker.h"
#include "UGUISubsystem.h"
#include "UnrealEdGlobals.h"
#include "MouseDeltaTracker.h"
#include "ViewportWorldInteraction.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "EditorModeManager.h"
#include "UIEditorViewport/UGUIEditorViewportInfo.h"
#include "Designer/DesignerEditorEventViewportClient.h"

/////////////////////////////////////////////////////////////////////////
// FUGUIEditorViewportClient

FUGUIEditorViewportClient::FUGUIEditorViewportClient(FPreviewScene* InPreviewScene)
	: FEditorViewportClient(nullptr, InPreviewScene, nullptr)
	, bIsManipulating(false)
	, bIsSimulateEnabled(false)
{
	WidgetMode = FWidget::WM_None;
	WidgetCoordSystem = COORD_Local;
	EngineShowFlags.DisableAdvancedFeatures();

	check(Widget);
	Widget->SetSnapEnabled(true);
	
	EditorViewportSize = FVector2D(1, 1);

	// Selectively set particular show flags that we need
	EngineShowFlags.SetSelectionOutline(GetDefault<ULevelEditorViewportSettings>()->bUseSelectionOutline);

	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowGrid;

	// Turn off so that actors added to the world do not have a lifespan (so they will not auto-destroy themselves).
	PreviewScene->GetWorld()->bBegunPlay = false;

	PreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);

	EventViewportClient = NewObject<UDesignerEditorEventViewportClient>(GetTransientPackage());
	UUGUISubsystem::SetEventViewportClient(PreviewScene->GetWorld(), EventViewportClient);
	
	if (IsSetShowGridChecked())
	{
		SetShowGrid();
	}

	ViewportType = ELevelViewportType::LVT_OrthoXZ;
	FEditorViewportClient::SetViewMode(DefaultPerspectiveViewMode);
	SetOrthoZoom(12458.4961f);
}

FUGUIEditorViewportClient::~FUGUIEditorViewportClient()
{
	EventViewportClient = nullptr;
}

void FUGUIEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);
	
	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread)
	{
		// Allow full tick only if preview simulation is enabled and we're not currently in an active SIE or PIE session
		if(bIsSimulateEnabled && GEditor->PlayWorld == nullptr && !GEditor->bIsSimulatingInEditor)
		{
			PreviewScene->GetWorld()->Tick((bAlwaysRealTime || IsRealtime()) ? LEVELTICK_All : LEVELTICK_TimeOnly, DeltaSeconds);
		}
		else
		{
			PreviewScene->GetWorld()->Tick((bAlwaysRealTime || IsRealtime()) ? LEVELTICK_All : LEVELTICK_TimeOnly, DeltaSeconds);
		}
	}

	if (EventViewportClient)
	{
		EventViewportClient->TickInput();
	}

	ZoomToFit();
}

void FUGUIEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);
}

FSceneView* FUGUIEditorViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass)
{
	FSceneView* SceneView = FEditorViewportClient::CalcSceneView(ViewFamily, StereoPass);

	const ELevelViewportType EffectiveViewportType = GetViewportType();

	if (EffectiveViewportType == LVT_OrthoXZ)
	{
		SceneView->SceneViewInitOptions.ViewRotationMatrix = FMatrix(
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 1, 0),
		FPlane(0, 0, 0, 1));
	}

	const FMatrix ViewMatrix = FTranslationMatrix(-SceneView->SceneViewInitOptions.ViewOrigin) * SceneView->SceneViewInitOptions.ViewRotationMatrix;
	ViewProjectionMatrix = ViewMatrix * SceneView->SceneViewInitOptions.ProjectionMatrix;
	
	SceneView->ViewMatrices = FViewMatrices(SceneView->SceneViewInitOptions);
	return SceneView;
}

namespace UIEditorViewportClient
{
	static constexpr float GridSize = 2048.0f;
	static constexpr int32 CellSize = 16;
	static constexpr float LightRotSpeed = 0.22f;
}

namespace UIPreviewLightConstants
{
	constexpr float MovingPreviewLightTimerDuration = 1.0f;

	constexpr float MinMouseRadius = 100.0f;
	constexpr float MinArrowLength = 10.0f;
	constexpr float ArrowLengthToSizeRatio = 0.1f;
	constexpr float MouseLengthToArrowLenghtRatio = 0.2f;

	constexpr float ArrowLengthToThicknessRatio = 0.05f;
	constexpr float MinArrowThickness = 2.0f;

	// Note: MinMouseRadius must be greater than MinArrowLength
}

bool FUGUIEditorViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta,
	float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (EventViewportClient)
	{
		EventViewportClient->InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
	}
	
	if (bDisableInput)
	{
		return true;
	}

	// Let the current mode have a look at the input before reacting to it.
	if (ModeTools->InputAxis(this, Viewport, ControllerId, Key, Delta, DeltaTime))
	{
		return true;
	}

	const bool bMouseButtonDown = InViewport->KeyState(EKeys::LeftMouseButton) || InViewport->KeyState(EKeys::MiddleMouseButton) || InViewport->KeyState(EKeys::RightMouseButton);
	const bool bLightMoveDown = InViewport->KeyState(EKeys::L);

	// Look at which axis is being dragged and by how much
	const float DragX = (Key == EKeys::MouseX) ? Delta : 0.f;
	const float DragY = (Key == EKeys::MouseY) ? Delta : 0.f;

	if (bLightMoveDown && bMouseButtonDown && PreviewScene)
	{
		// Adjust the preview light direction
		FRotator LightDir = PreviewScene->GetLightDirection();

		LightDir.Yaw += -DragX * UIEditorViewportClient::LightRotSpeed;
		LightDir.Pitch += -DragY * UIEditorViewportClient::LightRotSpeed;

		PreviewScene->SetLightDirection(LightDir);

		// Remember that we adjusted it for the visualization
		MovingPreviewLightTimer = UIPreviewLightConstants::MovingPreviewLightTimerDuration;
		MovingPreviewLightSavedScreenPos = FVector2D(LastMouseX, LastMouseY);

		Invalidate();
	}
	else
	{
		/**Save off axis commands for future camera work*/
		/*FCachedJoystickState* JoystickState = GetJoystickState(ControllerId);
		if (JoystickState)
		{
			JoystickState->AxisDeltaValues.Add(Key, Delta);
		}*/

		if (bIsTracking)
		{
			// Accumulate and snap the mouse movement since the last mouse button click.

			auto LastEnd = FVector::ZeroVector;

			if (GetViewportType() == LVT_OrthoXZ)
			{
				const FShadowMouseDeltaTracker* ShadowMouseDeltaTracker = reinterpret_cast<FShadowMouseDeltaTracker*>(MouseDeltaTracker);
				LastEnd = ShadowMouseDeltaTracker->End;
			}
			
			MouseDeltaTracker->AddDelta(this, Key, Delta, 0);
		
			if (GetViewportType() == LVT_OrthoXZ)
			{
				FShadowMouseDeltaTracker* ShadowMouseDeltaTracker = reinterpret_cast<FShadowMouseDeltaTracker*>(MouseDeltaTracker);
				const auto NewEnd = ShadowMouseDeltaTracker->End;
				auto EndDelta = NewEnd - LastEnd;
				EndDelta.Y = EndDelta.Z;
				EndDelta.Z = 0;
				ShadowMouseDeltaTracker->End = LastEnd + EndDelta;
				ShadowMouseDeltaTracker->EndSnapped = ShadowMouseDeltaTracker->End;
			}
		}
	}

	// If we are using a drag tool, paint the viewport so we can see it update.
	if (MouseDeltaTracker->UsingDragTool())
	{
		Invalidate(false, false);
	}

	return true;
}

void FUGUIEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	const bool bOldDrawAxes = bDrawAxes;
	bDrawAxes = false;

	FEditorViewportClient::Draw(InViewport, Canvas);

	bDrawAxes = bOldDrawAxes;

	if (bDrawAxes)
	{
		FViewport* ViewportBackup = Viewport;
		Viewport = InViewport ? InViewport : Viewport;

		// Allow HMD to modify the view later, just before rendering
		const bool bStereoRendering = GEngine->IsStereoscopic3D(InViewport);
		Canvas->SetScaledToRenderTarget(bStereoRendering);
		Canvas->SetStereoRendering(bStereoRendering);

		FEngineShowFlags UseEngineShowFlags = EngineShowFlags;
		if (OverrideShowFlagsFunc)
		{
			OverrideShowFlagsFunc(UseEngineShowFlags);
		}

		// Setup a FSceneViewFamily/FSceneView for the viewport.
		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			Canvas->GetRenderTarget(),
			GetScene(),
			UseEngineShowFlags)
			.SetRealtimeUpdate(IsRealtime() && FSlateThrottleManager::Get().IsAllowingExpensiveTasks()));

		ViewFamily.EngineShowFlags = UseEngineShowFlags;

		FSceneView* View = nullptr;

		const int32 NumViews = bStereoRendering ? GEngine->StereoRenderingDevice->GetDesiredNumberOfViews(bStereoRendering) : 1;
		for (int StereoViewIndex = 0; StereoViewIndex < NumViews; ++StereoViewIndex)
		{
			const EStereoscopicPass StereoPass = bStereoRendering ? GEngine->StereoRenderingDevice->GetViewPassForIndex(bStereoRendering, StereoViewIndex) : eSSP_FULL;

			View = CalcSceneView(&ViewFamily, StereoPass);
		}

		// Axes indicators
		if (bDrawAxes && !ViewFamily.EngineShowFlags.Game && !GLevelEditorModeTools().IsViewportUIHidden() && !IsVisualizeCalibrationMaterialEnabled())
		{
			switch (GetViewportType())
			{
			case LVT_OrthoXY:
			{
				const FRotator XYRot(-90.0f, -90.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &XYRot, EAxisList::XY);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
				break;
			}
			case LVT_OrthoXZ:
			{
#if SUPPORT_UI_BLUEPRINT_EDITOR
				const FRotator XYRot(90.0f, -90.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &XYRot, EAxisList::XY);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
#else
				const FRotator XZRot(0.0f, -90.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &XZRot, EAxisList::XZ);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
#endif
				break;
			}
			case LVT_OrthoYZ:
			{
				const FRotator YZRot(0.0f, 0.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &YZRot, EAxisList::YZ);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
				break;
			}
			case LVT_OrthoNegativeXY:
			{
				const FRotator XYRot(90.0f, 90.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &XYRot, EAxisList::XY);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
				break;
			}
			case LVT_OrthoNegativeXZ:
			{
				const FRotator XZRot(0.0f, 90.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &XZRot, EAxisList::XZ);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
				break;
			}
			case LVT_OrthoNegativeYZ:
			{
				const FRotator YZRot(0.0f, 180.0f, 0.0f);
				DrawAxes(Viewport, Canvas, &YZRot, EAxisList::YZ);
				if (View)
				{
					DrawScaleUnits(Viewport, Canvas, *View);
				}
				break;
			}
			default:
			{
				DrawAxes(Viewport, Canvas);
				break;
			}
			}
		}
	}
}

EMouseCursor::Type FUGUIEditorViewportClient::GetCursor(FViewport* InViewport, int32 X, int32 Y)
{
	EMouseCursor::Type MouseCursor = EMouseCursor::Default;

	// StaticFindObject is used lower down in this code, and that's not allowed while saving packages.
	if ( GIsSavingPackage )
	{
		return MouseCursor;
	}

	bool bMoveCanvasMovement = ShouldUseMoveCanvasMovement();

	if (RequiredCursorVisibiltyAndAppearance.bOverrideAppearance &&
		RequiredCursorVisibiltyAndAppearance.bHardwareCursorVisible)
	{
		MouseCursor = RequiredCursorVisibiltyAndAppearance.RequiredCursor;
	}
	else if (MouseDeltaTracker->UsingDragTool())
	{
		MouseCursor = EMouseCursor::Default;
	}
	else if (!RequiredCursorVisibiltyAndAppearance.bHardwareCursorVisible)
	{
		MouseCursor = EMouseCursor::None;
	}
	//only camera movement gets the hand icon
	else if (bMoveCanvasMovement && (Widget->GetCurrentAxis() == EAxisList::None) && bHasMouseMovedSinceClick)
	{
		//We're grabbing the canvas so the icon should look "grippy"
		MouseCursor = EMouseCursor::GrabHandClosed;
	}
	else if (bMoveCanvasMovement &&
		bHasMouseMovedSinceClick &&
		(GetWidgetMode() == FWidget::WM_Translate || GetWidgetMode() == FWidget::WM_TranslateRotateZ || GetWidgetMode() == FWidget::WM_2D))
	{
		MouseCursor = EMouseCursor::CardinalCross;
	}
	//wyisyg mode
	else if (IsUsingAbsoluteTranslation(true) && bHasMouseMovedSinceClick)
	{
		MouseCursor = EMouseCursor::CardinalCross;
	}
	// Don't select widget axes by mouse over while they're being controlled by a mouse drag.
	else if( InViewport->IsCursorVisible() && !bWidgetAxisControlledByDrag )
	{
		// allow editor modes to override cursor
		EMouseCursor::Type EditorModeCursor = EMouseCursor::Default;
		if (ModeTools->GetCursor(EditorModeCursor))
		{
			MouseCursor = EditorModeCursor;
		}
		else
		{
			HHitProxy* HitProxy = InViewport->GetHitProxy(X,Y);

			// Change the mouse cursor if the user is hovering over something they can interact with.
			if( HitProxy && !bUsingOrbitCamera )
			{
				MouseCursor = HitProxy->GetMouseCursor();
				bShouldCheckHitProxy = true;
			}
			else
			{
				// Turn off widget highlight if there currently is one
				if( Widget->GetCurrentAxis() != EAxisList::None )
				{
					SetCurrentWidgetAxis( EAxisList::None );
					Invalidate( false, false );
				}
			}
		}
	}

	// Allow the viewport interaction to override any previously set mouse cursor
	UWorld* World = GetWorld();
	UViewportWorldInteraction* WorldInteraction = (World ? Cast<UViewportWorldInteraction>(GEditor->GetEditorWorldExtensionsManager()->GetEditorWorldExtensions(World)->FindExtension(UViewportWorldInteraction::StaticClass())) : nullptr);
	if (WorldInteraction != nullptr)
	{
		if (WorldInteraction->ShouldForceCursor())
		{
			MouseCursor = EMouseCursor::Crosshairs;
			SetRequiredCursor(false, true);
			UpdateRequiredCursorVisibility();
		}
		else if (WorldInteraction->ShouldSuppressExistingCursor())
		{
			MouseCursor = EMouseCursor::None;
			SetRequiredCursor(false, false);
			UpdateRequiredCursorVisibility();
		}
	}

	CachedMouseX = X;
	CachedMouseY = Y;

	const bool RightMouseButtonDown = Viewport->KeyState(EKeys::RightMouseButton) ? true : false;
	if (RightMouseButtonDown)
	{
		ViewportMouseCursor = EMouseCursor::Type::None;
	}

	if (RequiredCursorVisibiltyAndAppearance.RequiredCursor == EMouseCursor::CardinalCross)
	{
		RequiredCursorVisibiltyAndAppearance.RequiredCursor = EMouseCursor::Default;
	}
	
	if (ViewportMouseCursor != EMouseCursor::Type::None)
	{
		return ViewportMouseCursor;
	}

	return MouseCursor;
}

bool FUGUIEditorViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
	if (EventViewportClient)
	{
		EventViewportClient->InputChar(InViewport, ControllerId, Character);
	}
	return FEditorViewportClient::InputChar(InViewport, ControllerId, Character);
}

bool FUGUIEditorViewportClient::InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle,
	ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex)
{
	if (EventViewportClient)
	{
		EventViewportClient->InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchPadIndex);
	}
	return FEditorViewportClient::InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force,
	                                         DeviceTimestamp, TouchPadIndex);
}

void FUGUIEditorViewportClient::LostFocus(FViewport* InViewport)
{
	if (EventViewportClient)
	{
		EventViewportClient->LostFocus(InViewport);
	}
	FEditorViewportClient::LostFocus(InViewport);
}

void FUGUIEditorViewportClient::ZoomToFit()
{
	if (bZoomToFit && IsOrtho())
	{
		const FIntPoint ViewportSize = Viewport->GetSizeXY();
		if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
		{
			return;
		}

		if (TargetViewSize.X <= 0 || TargetViewSize.Y <= 0)
		{
			return;
		}
		
		bZoomToFit = false;
		

		const FVector ZoomPosition = FVector::ZeroVector;
		const FVector2D ZoomSize = TargetViewSize;
		
		float AspectToUse = AspectRatio;
		if (!bUseControllingActorViewInfo && ViewportSize.X > 0 && ViewportSize.Y > 0)
		{
			AspectToUse = Viewport->GetDesiredAspectRatio();
		}

		FViewportCameraTransform& ViewTransform = GetViewTransform();
		ViewTransform.TransitionToLocation(ZoomPosition, EditorViewportWidget, !bAnimationZoom);

		const FVector2D TargetSize = FVector2D(FMath::Abs(ZoomSize.X) + ZoomOffset.X, FMath::Abs(ZoomSize.Y) + ZoomOffset.Y);
		float TargetAspectToUse = AspectRatio;
		if (TargetSize.X > 0 && TargetSize.Y > 0)
		{
			TargetAspectToUse = TargetSize.X / TargetSize.Y;
		}

		float AxisSize = TargetSize.X > TargetSize.Y ? TargetSize.X : TargetSize.Y;
		if (TargetAspectToUse < AspectToUse)
		{
			AxisSize = AspectToUse * TargetSize.Y;
		}
		
		const float Zoom = AxisSize * GetOrthoZoom() / (ViewportSize.X * GetOrthoUnitsPerPixel(Viewport));
		ViewTransform.SetOrthoZoom(Zoom);
	}
}

void FUGUIEditorViewportClient::SetupViewportInfo(FName ViewportName, TWeakPtr<FUGUIEditorViewportClient> InViewportClient,
	TWeakPtr<SUGUIEditorViewport> InEditorViewport, UUGUIEditorViewportInfo* ViewportInfo)
{
	EditorViewportWidget = InEditorViewport;

	if (PreviewScene->GetWorld()->ExtraReferencedObjects.Num() > 0)
	{
		if (UUGUIEditorViewportInfo* InfoObj = Cast<UUGUIEditorViewportInfo>(PreviewScene->GetWorld()->ExtraReferencedObjects[0]))
		{
			InfoObj->ViewportName = ViewportName;
			InfoObj->ViewportClient = InViewportClient;
			InfoObj->EditorViewport = InEditorViewport;
		}
	}
	else if (ViewportInfo == nullptr)
	{
		UUGUIEditorViewportInfo* InfoObj = NewObject<UUGUIEditorViewportInfo>(GetTransientPackage());
		InfoObj->ViewportName = ViewportName;
		InfoObj->ViewportClient = InViewportClient;
		InfoObj->EditorViewport = InEditorViewport;
		PreviewScene->GetWorld()->ExtraReferencedObjects.Add(InfoObj);
	}
	else
	{
		PreviewScene->GetWorld()->ExtraReferencedObjects.Add(ViewportInfo);
	}
}

void FUGUIEditorViewportClient::DrawCanvas( FViewport& InViewport, FSceneView& View, FCanvas& Canvas )
{
	FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(GetPreviewScene()->GetWorld(), true);
	if (WorldViewportInfo)
	{
		WorldViewportInfo->UpdateViewportSize(GetPreviewScene()->GetWorld(), InViewport.GetSizeXY());
	}

	EditorViewportSize = InViewport.GetSizeXY();
}

bool FUGUIEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	if (EventViewportClient)
	{
		EventViewportClient->InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
	}
	
	bool bHandled = GUnrealEd->ComponentVisManager.HandleInputKey(this, InViewport, Key, Event);;

	if( !bHandled )
	{
		bHandled = FEditorViewportClient::InputKey(InViewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
	}

	return bHandled;
}

void FUGUIEditorViewportClient::ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	if (HitProxy)
	{
		if (HitProxy->IsA(HInstancedStaticMeshInstance::StaticGetType()))
		{
			return;
		}
		else if (HitProxy->IsA(HWidgetAxis::StaticGetType()))
		{
			const bool bOldModeWidgets1 = EngineShowFlags.ModeWidgets;
			const bool bOldModeWidgets2 = View.Family->EngineShowFlags.ModeWidgets;

			EngineShowFlags.SetModeWidgets(false);
			FSceneViewFamily* SceneViewFamily = const_cast<FSceneViewFamily*>(View.Family);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(false);
			const bool bWasWidgetDragging = Widget->IsDragging();
			Widget->SetDragging(false);

			// Invalidate the hit proxy map so it will be rendered out again when GetHitProxy
			// is called
			Viewport->InvalidateHitProxy();

			// This will actually re-render the viewport's hit proxies!
			HHitProxy* HitProxyWithoutAxisWidgets = Viewport->GetHitProxy(HitX, HitY);
			if (HitProxyWithoutAxisWidgets != nullptr && !HitProxyWithoutAxisWidgets->IsA(HWidgetAxis::StaticGetType()))
			{
				// Try this again, but without the widget this time!
				ProcessClick(View, HitProxyWithoutAxisWidgets, Key, Event, HitX, HitY);
			}

			// Undo the evil
			EngineShowFlags.SetModeWidgets(bOldModeWidgets1);
			SceneViewFamily->EngineShowFlags.SetModeWidgets(bOldModeWidgets2);

			Widget->SetDragging(bWasWidgetDragging);

			// Invalidate the hit proxy map again so that it'll be refreshed with the original
			// scene contents if we need it again later.
			Viewport->InvalidateHitProxy();
			return;
		}
		else if (HitProxy->IsA(HActor::StaticGetType()))
		{
			HActor* ActorProxy = static_cast<HActor*>(HitProxy);
			if (ActorProxy && ActorProxy->Actor && ActorProxy->PrimComponent)
			{
				USceneComponent* SelectedCompInstance = nullptr;

				if (ActorProxy->Actor->IsChildActor())
				{
					const AActor* TestActor = ActorProxy->Actor;
					while (TestActor->GetParentActor()->IsChildActor())
					{
						TestActor = TestActor->GetParentActor();
					}
				}
			}

			Invalidate();
			return;
		}
	}
	
	GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click);
}

bool FUGUIEditorViewportClient::InputWidgetDelta( FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale )
{
	bool bHandled = false;
	if(bIsManipulating && CurrentAxis != EAxisList::None)
	{
		bHandled = true;
		Invalidate();
	}

	return bHandled;
}

void FUGUIEditorViewportClient::TrackingStarted( const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge )
{
	if( !bIsManipulating && bIsDraggingWidget )
	{
		// Suspend component modification during each delta step to avoid recording unnecessary overhead into the transaction buffer
		GEditor->DisableDeltaModification(true);
		
		bIsManipulating = true;
	}
}

void FUGUIEditorViewportClient::TrackingStopped() 
{
	if( bIsManipulating )
	{
		// End transaction
		bIsManipulating = false;

		// Restore component delta modification
		GEditor->DisableDeltaModification(false);
	}
}

FWidget::EWidgetMode FUGUIEditorViewportClient::GetWidgetMode() const
{
	// Default to not drawing the widget
	return FWidget::WM_None;
}

void FUGUIEditorViewportClient::SetWidgetMode( FWidget::EWidgetMode NewMode )
{
	WidgetMode = NewMode;
}

void FUGUIEditorViewportClient::SetWidgetCoordSystemSpace( ECoordSystem NewCoordSystem ) 
{
	WidgetCoordSystem = NewCoordSystem;
}

FVector FUGUIEditorViewportClient::GetWidgetLocation() const
{
	FVector ComponentVisWidgetLocation;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	{
		return ComponentVisWidgetLocation;
	}
	
	return FVector::ZeroVector;
}

FMatrix FUGUIEditorViewportClient::GetWidgetCoordSystem() const
{
	FMatrix ComponentVisWidgetCoordSystem;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetCustomInputCoordinateSystem(this, ComponentVisWidgetCoordSystem))
	{
		return ComponentVisWidgetCoordSystem;
	}

	FMatrix Matrix = FMatrix::Identity;
	if(!Matrix.Equals(FMatrix::Identity))
	{
		Matrix.RemoveScaling();
	}

	return Matrix;
}

int32 FUGUIEditorViewportClient::GetCameraSpeedSetting() const
{
	return GetDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed;
}

void FUGUIEditorViewportClient::SetCameraSpeedSetting(int32 SpeedSetting)
{
	GetMutableDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed = SpeedSetting;
}

