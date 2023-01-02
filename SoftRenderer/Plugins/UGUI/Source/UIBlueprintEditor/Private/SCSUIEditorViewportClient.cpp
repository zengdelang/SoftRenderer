#include "SCSUIEditorViewportClient.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "CanvasItem.h"
#include "Editor/EditorPerProjectUserSettings.h"
#include "Settings/LevelEditorViewportSettings.h"
#include "Editor/UnrealEdEngine.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "Engine/StaticMesh.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "EngineUtils.h"
#include "UnrealEdGlobals.h"
#include "SEditorViewport.h"
#include "Editor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "ScopedTransaction.h"
#include "ISCSEditorCustomization.h"
#include "CanvasTypes.h"
#include "EditorModeManager.h"
#include "IXRTrackingSystem.h"
#include "MouseDeltaTracker.h"
#include "SceneViewExtension.h"
#include "ShadowMouseDeltaTracker.h"
#include "SKismetInspector.h"
#include "SSCSUIEditorViewport.h"
#include "UGUISubsystem.h"
#include "UIBlueprintEditor.h"
#include "Core/BehaviourComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "SSCSComponentEditor.h"
#include "UIBlueprintEditorModule.h"
#include "UIEditorPerProjectUserSettings.h"
#include "ViewportWorldInteraction.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasSubComponent.h"

static TAutoConsoleVariable<int32> CVarEditorUIViewportTest(
	TEXT("r.Test.UIEditorConstrainedView"),
	0,
	TEXT("Allows to test different viewport rectangle configuations (in game only) as they can happen when using Matinee/Editor.\n")
	TEXT("0: off(default)\n")
	TEXT("1..7: Various Configuations"),
	ECVF_RenderThreadSafe);

namespace
{
	/** Automatic translation applied to the camera in the default editor viewport logic when orbit mode is enabled. */
	constexpr float AutoViewportOrbitCameraTranslate = 256.0f;

	void DrawAngles(FCanvas* Canvas, int32 XPos, int32 YPos, EAxisList::Type ManipAxis, FWidget::EWidgetMode MoveMode, const FRotator& Rotation, const FVector& Translation)
	{
		FString OutputString(TEXT(""));
		if(MoveMode == FWidget::WM_Rotate && Rotation.IsZero() == false)
		{
			//Only one value moves at a time
			const FVector EulerAngles = Rotation.Euler();
			if(ManipAxis == EAxisList::X)
			{
				OutputString += FString::Printf(TEXT("Roll: %0.2f"), EulerAngles.X);
			}
			else if(ManipAxis == EAxisList::Y)
			{
				OutputString += FString::Printf(TEXT("Pitch: %0.2f"), EulerAngles.Y);
			}
			else if(ManipAxis == EAxisList::Z)
			{
				OutputString += FString::Printf(TEXT("Yaw: %0.2f"), EulerAngles.Z);
			}
		}
		else if(MoveMode == FWidget::WM_Translate && Translation.IsZero() == false)
		{
			//Only one value moves at a time
			if(ManipAxis == EAxisList::X)
			{
				OutputString += FString::Printf(TEXT(" %0.2f"), Translation.X);
			}
			else if(ManipAxis == EAxisList::Y)
			{
				OutputString += FString::Printf(TEXT(" %0.2f"), Translation.Y);
			}
			else if(ManipAxis == EAxisList::Z)
			{
				OutputString += FString::Printf(TEXT(" %0.2f"), Translation.Z);
			}
		}

		if(OutputString.Len() > 0)
		{
			FCanvasTextItem TextItem( FVector2D(XPos, YPos), FText::FromString( OutputString ), GEngine->GetSmallFont(), FLinearColor::White );
			Canvas->DrawItem( TextItem );
		} 
	}

	// Determine whether or not the given node has a parent node that is not the root node, is movable and is selected
	bool IsMovableParentNodeSelected(const FSCSComponentEditorTreeNodePtrType& NodePtr, const TArray<FSCSComponentEditorTreeNodePtrType>& SelectedNodes)
	{
		if(NodePtr.IsValid())
		{
			// Check for a valid parent node
			const FSCSComponentEditorTreeNodePtrType ParentNodePtr = NodePtr->GetParent();
			if(ParentNodePtr.IsValid() && !ParentNodePtr->IsRootComponent())
			{
				if(SelectedNodes.Contains(ParentNodePtr))
				{
					// The parent node is not the root node and is also selected; success
					return true;
				}
				else
				{
					// Recursively search for any other parent nodes farther up the tree that might be selected
					return IsMovableParentNodeSelected(ParentNodePtr, SelectedNodes);
				}
			}
		}

		return false;
	}
}

/////////////////////////////////////////////////////////////////////////
// FSCSUIEditorViewportClient

FSCSUIEditorViewportClient::FSCSUIEditorViewportClient(TWeakPtr<FUIBlueprintEditor>& InBlueprintEditorPtr, FPreviewScene* InPreviewScene, const TSharedRef<SSCSUIEditorViewport>& InSCSEditorViewport)
	: FEditorViewportClient(nullptr, InPreviewScene, StaticCastSharedRef<SEditorViewport>(InSCSEditorViewport))
	, BlueprintEditorPtr(InBlueprintEditorPtr)
	, PreviewActorBounds(ForceInitToZero)
	, bIsManipulating(false)
	, ScopedTransaction(nullptr)
	, bIsSimulateEnabled(false)
{
	WidgetMode = FWidget::WM_None;
	WidgetCoordSystem = COORD_Local;
	EngineShowFlags.DisableAdvancedFeatures();

	check(Widget);
	Widget->SetSnapEnabled(true);

	UIEditorViewport = InSCSEditorViewport;
	
	EditorViewportSize = FVector2D(1, 1);

	// Selectively set particular show flags that we need
	EngineShowFlags.SetSelectionOutline(GetDefault<ULevelEditorViewportSettings>()->bUseSelectionOutline);

	// Set if the grid will be drawn
	DrawHelper.bDrawGrid = GetDefault<UEditorPerProjectUserSettings>()->bSCSEditorShowGrid;

	// Turn off so that actors added to the world do not have a lifespan (so they will not auto-destroy themselves).
	PreviewScene->GetWorld()->bBegunPlay = false;

	PreviewScene->SetSkyCubemap(GUnrealEd->GetThumbnailManager()->AmbientCubemap);

#if SUPPORT_UI_BLUEPRINT_EDITOR
	if (IsSetShowGridChecked())
	{
		SetShowGrid();
	}

	ViewportType = ELevelViewportType::LVT_OrthoXZ;
	FEditorViewportClient::SetViewMode(DefaultPerspectiveViewMode);
	SetOrthoZoom(12458.4961f);
#endif

	bRegisterTransactionDelegate = true;
}

FSCSUIEditorViewportClient::~FSCSUIEditorViewportClient()
{
	// Ensure that an in-progress transaction is ended
	EndTransaction();
}

void FSCSUIEditorViewportClient::Tick(float DeltaSeconds)
{
	FEditorViewportClient::Tick(DeltaSeconds);

	if (bRegisterTransactionDelegate)
	{
		bRegisterTransactionDelegate = false;
		IUIBlueprintEditorModule::OnUIBlueprintEditorBeginTransaction.AddSP(this, &FSCSUIEditorViewportClient::OnUIBlueprintEditorBeginTransaction);
		IUIBlueprintEditorModule::OnUIBlueprintEditorEndTransaction.AddSP(this, &FSCSUIEditorViewportClient::OnUIBlueprintEditorEndTransaction);
	}
	
	// Register the selection override delegate for the preview actor's components
	const TSharedPtr<SSCSComponentEditor> SCSComponentEditor = BlueprintEditorPtr.Pin()->GetSCSComponentEditor();
	AActor* PreviewActor = GetPreviewActor();
	if (PreviewActor != nullptr)
	{
		for (UActorComponent* Component : PreviewActor->GetComponents())
		{
			if (UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(Component))
			{
				if (!PrimComponent->SelectionOverrideDelegate.IsBound())
				{
					SCSComponentEditor->SetSelectionOverride(PrimComponent);
				}
			}
		}
	}
	else
	{
		InvalidatePreview(false);
	}

	// Tick the preview scene world.
	if (!GIntraFrameDebuggingGameThread)
	{
		// Ensure that the preview actor instance is up-to-date for component editing (e.g. after compiling the Blueprint, the actor may be reinstanced outside of this class)
		if(PreviewActor != BlueprintEditorPtr.Pin()->GetBlueprintObj()->SimpleConstructionScript->GetComponentEditorActorInstance())
		{
			BlueprintEditorPtr.Pin()->GetBlueprintObj()->SimpleConstructionScript->SetComponentEditorActorInstance(PreviewActor);
		}

		// Allow full tick only if preview simulation is enabled and we're not currently in an active SIE or PIE session
		if(bIsSimulateEnabled && GEditor->PlayWorld == nullptr && !GEditor->bIsSimulatingInEditor)
		{
			PreviewScene->GetWorld()->Tick(IsRealtime() ? LEVELTICK_All : LEVELTICK_TimeOnly, DeltaSeconds);
		}
		else
		{
			PreviewScene->GetWorld()->Tick(IsRealtime() ? LEVELTICK_ViewportsOnly : LEVELTICK_TimeOnly, DeltaSeconds);
		}
	}

	if (EventViewportClient.IsValid())
	{
		EventViewportClient->TickInput();
	}

	ZoomToFit();
}

void FSCSUIEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FEditorViewportClient::Draw(View, PDI);

	bool bHitTesting = PDI->IsHitTesting();
	AActor* PreviewActor = GetPreviewActor();
	if(PreviewActor)
	{
		if(GUnrealEd != nullptr)
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
			for (int32 SelectionIndex = 0; SelectionIndex < SelectedNodes.Num(); ++SelectionIndex)
			{
				const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes[SelectionIndex];

				UActorComponent* Comp = SelectedNode->FindComponentInstanceInActor(PreviewActor);
				if(Comp != nullptr && Comp->IsRegistered())
				{
					// Try and find a visualizer
					TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(Comp->GetClass());
					if (Visualizer.IsValid())
					{
						Visualizer->DrawVisualization(Comp, View, PDI);
					}
				}
			}
		}
	}
}

#if SUPPORT_UI_BLUEPRINT_EDITOR

FSceneView* FSCSUIEditorViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const EStereoscopicPass StereoPass)
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

namespace EditorViewportClient
{
	static constexpr float GridSize = 2048.0f;
	static constexpr int32 CellSize = 16;
	static constexpr float LightRotSpeed = 0.22f;
}

namespace PreviewLightConstants
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

bool FSCSUIEditorViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta,
	float DeltaTime, int32 NumSamples, bool bGamepad)
{
	if (EventViewportClient.IsValid())
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

		LightDir.Yaw += -DragX * EditorViewportClient::LightRotSpeed;
		LightDir.Pitch += -DragY * EditorViewportClient::LightRotSpeed;

		PreviewScene->SetLightDirection(LightDir);

		// Remember that we adjusted it for the visualization
		MovingPreviewLightTimer = PreviewLightConstants::MovingPreviewLightTimerDuration;
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

void FSCSUIEditorViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
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

EMouseCursor::Type FSCSUIEditorViewportClient::GetCursor(FViewport* InViewport, int32 X, int32 Y)
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
	else if( MouseDeltaTracker->UsingDragTool() )
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
		if(ModeTools->GetCursor(EditorModeCursor))
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

#endif

bool FSCSUIEditorViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
{
	if (EventViewportClient.IsValid())
	{
		EventViewportClient->InputChar(InViewport, ControllerId, Character);
	}
	return FEditorViewportClient::InputChar(InViewport, ControllerId, Character);
}

bool FSCSUIEditorViewportClient::InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle,
	ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchPadIndex)
{
	if (EventViewportClient.IsValid())
	{
		EventViewportClient->InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchPadIndex);
	}
	return FEditorViewportClient::InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force,
	                                         DeviceTimestamp, TouchPadIndex);
}

void FSCSUIEditorViewportClient::LostFocus(FViewport* InViewport)
{
	if (EventViewportClient.IsValid())
	{
		EventViewportClient->LostFocus(InViewport);
	}
	FEditorViewportClient::LostFocus(InViewport);
}

void FSCSUIEditorViewportClient::DrawCanvas( FViewport& InViewport, FSceneView& View, FCanvas& Canvas )
{
	FWorldViewportInfo* WorldViewportInfo = UUGUISubsystem::GetWorldViewportInfo(GetPreviewScene()->GetWorld(), true);
	if (WorldViewportInfo)
	{
		WorldViewportInfo->UpdateViewportSize(GetPreviewScene()->GetWorld(), InViewport.GetSizeXY());
	}

	EditorViewportSize = InViewport.GetSizeXY();
	
	AActor* PreviewActor = GetPreviewActor();
	if(PreviewActor)
	{
		if (GUnrealEd != nullptr)
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
			for (int32 SelectionIndex = 0; SelectionIndex < SelectedNodes.Num(); ++SelectionIndex)
			{
				const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes[SelectionIndex];

				UActorComponent* Comp = SelectedNode->FindComponentInstanceInActor(PreviewActor);
				if (Comp != nullptr && Comp->IsRegistered())
				{
					// Try and find a visualizer
					TSharedPtr<FComponentVisualizer> Visualizer = GUnrealEd->FindComponentVisualizer(Comp->GetClass());
					if (Visualizer.IsValid())
					{
						Visualizer->DrawVisualizationHUD(Comp, &InViewport, &View, &Canvas);
					}
				}
			}
		}

		TGuardValue<bool> AutoRestore(GAllowActorScriptExecutionInEditor, true);

		const int32 HalfX = 0.5f * Viewport->GetSizeXY().X;
		const int32 HalfY = 0.5f * Viewport->GetSizeXY().Y;

		TArray<TSharedPtr<FSCSComponentEditorTreeNode>> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
		if(bIsManipulating && SelectedNodes.Num() > 0)
		{
			USceneComponent* SceneComp = Cast<USceneComponent>(SelectedNodes[0]->FindComponentInstanceInActor(PreviewActor));
			if(SceneComp)
			{
				const FVector WidgetLocation = GetWidgetLocation();
				const FPlane Proj = View.Project(WidgetLocation);
				if(Proj.W > 0.0f)
				{
					const int32 XPos = HalfX + (HalfX * Proj.X);
					const int32 YPos = HalfY + (HalfY * (Proj.Y * -1));
					DrawAngles(&Canvas, XPos, YPos, GetCurrentWidgetAxis(), GetWidgetMode(), GetWidgetCoordSystem().Rotator(), WidgetLocation);
				}
			}
		}
	}

	int32 UIBatches = 0;
	int32 BlurUIBatches = 0;
	int32 GlitchUIBatches = 0;
	
	if (PreviewActor)
	{
		URectTransformComponent* RootComponent = Cast<URectTransformComponent>(PreviewActor->GetRootComponent());
		if (IsValid(RootComponent))
		{
			UCanvasSubComponent* CanvasComp = Cast<UCanvasSubComponent>(RootComponent->GetComponent(UCanvasSubComponent::StaticClass(), true));
			
			if (CanvasComp)
			{
				UIBatches += CanvasComp->UIBatches;
				BlurUIBatches += CanvasComp->BlurUIBatches;
				GlitchUIBatches += CanvasComp->GlitchUIBatches;

				for (TObjectIterator<UCanvasSubComponent> It; It; ++It)
				{
					if (*It != CanvasComp)
					{
						if (It->GetRootCanvas() == CanvasComp)
						{
							UIBatches += It->UIBatches;
							BlurUIBatches += It->BlurUIBatches;
							GlitchUIBatches += It->GlitchUIBatches;
						}
					}
				}
			}
		}
	}

	TArray<SSCSUIEditorViewport::FOverlayTextItem> TextItems;
				
	TextItems.Add(SSCSUIEditorViewport::FOverlayTextItem(
		FText::Format(NSLOCTEXT("UIBlueprintEditor", "TotalUIBatches", "\tTotal UI Batches: {0}"), FText::AsNumber(UIBatches))));
	UIEditorViewport.Pin()->PopulateOverlayText(TextItems);

	TextItems.Add(SSCSUIEditorViewport::FOverlayTextItem(
		FText::Format(NSLOCTEXT("UIBlueprintEditor", "BlurUIBatches", "\t\tBlur UI Batches: {0}"), FText::AsNumber(BlurUIBatches))));
	UIEditorViewport.Pin()->PopulateOverlayText(TextItems);

	TextItems.Add(SSCSUIEditorViewport::FOverlayTextItem(
		FText::Format(NSLOCTEXT("UIBlueprintEditor", "GlitchUIBatches", "\t\tGlitch UI Batches: {0}"), FText::AsNumber(GlitchUIBatches))));
	UIEditorViewport.Pin()->PopulateOverlayText(TextItems);
}

bool FSCSUIEditorViewportClient::InputKey(FViewport* InViewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	if (EventViewportClient.IsValid())
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

void FSCSUIEditorViewportClient::ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY)
{
	const FViewportClick Click(&View, this, Key, Event, HitX, HitY);

	if (HitProxy)
	{
		if (HitProxy->IsA(HInstancedStaticMeshInstance::StaticGetType()))
		{
			const HInstancedStaticMeshInstance* InstancedStaticMeshInstanceProxy = static_cast<HInstancedStaticMeshInstance*>(HitProxy);

			const TSharedPtr<ISCSEditorCustomization> Customization = BlueprintEditorPtr.Pin()->CustomizeSCSEditor(InstancedStaticMeshInstanceProxy->Component);
			if (Customization.IsValid() && Customization->HandleViewportClick(AsShared(), View, HitProxy, Key, Event, HitX, HitY))
			{
				Invalidate();
			}

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
			AActor* PreviewActor = GetPreviewActor();
			if (ActorProxy && ActorProxy->Actor && ActorProxy->PrimComponent)
			{
				USceneComponent* SelectedCompInstance = nullptr;

				if (ActorProxy->Actor == PreviewActor)
				{
					UPrimitiveComponent* TestComponent = const_cast<UPrimitiveComponent*>(ActorProxy->PrimComponent);
					if (ActorProxy->Actor->GetComponents().Contains(TestComponent))
					{
						SelectedCompInstance = TestComponent;
					}
				}
				else if (ActorProxy->Actor->IsChildActor())
				{
					const AActor* TestActor = ActorProxy->Actor;
					while (TestActor->GetParentActor()->IsChildActor())
					{
						TestActor = TestActor->GetParentActor();
					}

					if (TestActor->GetParentActor() == PreviewActor)
					{
						SelectedCompInstance = TestActor->GetParentComponent();
					}
				}
#if SUPPORT_UI_BLUEPRINT_EDITOR	
				else
				{
					AActor* TestActor = ActorProxy->Actor;
					if (TestActor && TestActor->GetRootComponent())
					{
						UBehaviourComponent* ActorAttachParent = Cast<UBehaviourComponent>(TestActor->GetRootComponent()->GetAttachParent());
						if (ActorAttachParent)
						{
							SelectedCompInstance = ActorAttachParent;
						}
					}
				}
#endif

				if (SelectedCompInstance)
				{
					const TSharedPtr<ISCSEditorCustomization> Customization = BlueprintEditorPtr.Pin()->CustomizeSCSEditor(SelectedCompInstance);
					if (!(Customization.IsValid() && Customization->HandleViewportClick(AsShared(), View, HitProxy, Key, Event, HitX, HitY)))
					{
						const bool bIsCtrlKeyDown = Viewport->KeyState(EKeys::LeftControl) || Viewport->KeyState(EKeys::RightControl);
						if (BlueprintEditorPtr.IsValid())
						{
							// Note: This will find and select any node associated with the component instance that's attached to the proxy (including visualizers)
							BlueprintEditorPtr.Pin()->FindAndSelectSCSEditorTreeNode(SelectedCompInstance, bIsCtrlKeyDown);
						}
					}
				}
			}

			Invalidate();
			return;
		}
	}
	
	GUnrealEd->ComponentVisManager.HandleClick(this, HitProxy, Click);
}

bool FSCSUIEditorViewportClient::InputWidgetDelta( FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale )
{
	bool bHandled = false;
	if(bIsManipulating && CurrentAxis != EAxisList::None)
	{
		bHandled = true;
		AActor* PreviewActor = GetPreviewActor();
		const TSharedPtr<FUIBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin();
		if (PreviewActor && BlueprintEditor.IsValid())
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditor->GetSelectedSCSComponentEditorTreeNodes();
			if(SelectedNodes.Num() > 0)
			{
				FVector ModifiedScale = Scale;

				// (mirrored from Level Editor VPC) - we don't scale components when we only have a very small scale change
				if (!Scale.IsNearlyZero())
				{
					if (GEditor->UsePercentageBasedScaling())
					{
						ModifiedScale = Scale * ((GEditor->GetScaleGridSize() / 100.0f) / GEditor->GetGridSize());
					}
				}
				else
				{
					ModifiedScale = FVector::ZeroVector;
				}

				for (const FSCSComponentEditorTreeNodePtrType& SelectedNodePtr : SelectedNodes)
				{
					// Don't allow editing of a root node, inherited SCS node or child node that also has a movable (non-root) parent node selected
					const bool bCanEdit = GUnrealEd->ComponentVisManager.IsActive() ||
						(!SelectedNodePtr->IsRootComponent() && !IsMovableParentNodeSelected(SelectedNodePtr, SelectedNodes));

					if(bCanEdit)
					{
						if (GUnrealEd->ComponentVisManager.HandleInputDelta(this, InViewport, Drag, Rot, Scale))
						{
							GUnrealEd->RedrawLevelEditingViewports();
							Invalidate();
							return true;
						}
						
						USceneComponent* SceneComp = Cast<USceneComponent>(SelectedNodePtr->FindComponentInstanceInActor(PreviewActor));
						USceneComponent* SelectedTemplate = Cast<USceneComponent>(SelectedNodePtr->GetOrCreateEditableComponentTemplate(BlueprintEditor->GetBlueprintObj()));
						if(SceneComp != nullptr && SelectedTemplate != nullptr)
						{
							// Cache the current default values for propagation
							FVector OldRelativeLocation = SelectedTemplate->GetRelativeLocation();
							FRotator OldRelativeRotation = SelectedTemplate->GetRelativeRotation();
							FVector OldRelativeScale3D = SelectedTemplate->GetRelativeScale3D();

							// Adjust the deltas as necessary
							FComponentEditorUtils::AdjustComponentDelta(SceneComp, Drag, Rot);

							TSharedPtr<ISCSEditorCustomization> Customization = BlueprintEditor->CustomizeSCSEditor(SceneComp);
							if(Customization.IsValid() && Customization->HandleViewportDrag(SceneComp, SelectedTemplate, Drag, Rot, ModifiedScale, GetWidgetLocation()))
							{
								// Handled by SCS Editor customization
							}
							else
							{
								// Apply delta to the template component object 
								// (the preview scene component will be set in one of the ArchetypeInstances loops below... to keep the two in sync) 
								GEditor->ApplyDeltaToComponent(
										SelectedTemplate,
										true,
										&Drag,
										&Rot,
										&ModifiedScale,
										SelectedTemplate->GetRelativeLocation());
							}

							UBlueprint* PreviewBlueprint = UBlueprint::GetBlueprintFromClass(PreviewActor->GetClass());
							if(PreviewBlueprint != nullptr)
							{
								// Like PostEditMove(), but we only need to re-run construction scripts
								if(PreviewBlueprint && PreviewBlueprint->bRunConstructionScriptOnDrag)
								{
									PreviewActor->RerunConstructionScripts();
								}

								SceneComp->PostEditComponentMove(true); // @TODO HACK passing 'finished' every frame...

								// If a constraint, copy back updated constraint frames to template
								UPhysicsConstraintComponent* ConstraintComp = Cast<UPhysicsConstraintComponent>(SceneComp);
								UPhysicsConstraintComponent* TemplateComp = Cast<UPhysicsConstraintComponent>(SelectedTemplate);
								if(ConstraintComp && TemplateComp)
								{
									TemplateComp->ConstraintInstance.CopyConstraintGeometryFrom(&ConstraintComp->ConstraintInstance);
								}

								// Iterate over all the active archetype instances and propagate the change(s) to the matching component instance
								TArray<UObject*> ArchetypeInstances;
								if(SelectedTemplate->HasAnyFlags(RF_ArchetypeObject))
								{
									SelectedTemplate->GetArchetypeInstances(ArchetypeInstances);
									for(int32 InstanceIndex = 0; InstanceIndex < ArchetypeInstances.Num(); ++InstanceIndex)
									{
										SceneComp = Cast<USceneComponent>(ArchetypeInstances[InstanceIndex]);
										if(SceneComp != nullptr)
										{
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeLocation_DirectMutable(), OldRelativeLocation, SelectedTemplate->GetRelativeLocation());
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeRotation_DirectMutable(), OldRelativeRotation, SelectedTemplate->GetRelativeRotation());
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeScale3D_DirectMutable(),  OldRelativeScale3D,  SelectedTemplate->GetRelativeScale3D());
										}
									}
								}
								else if(UObject* Outer = SelectedTemplate->GetOuter())
								{
									Outer->GetArchetypeInstances(ArchetypeInstances);
									for(int32 InstanceIndex = 0; InstanceIndex < ArchetypeInstances.Num(); ++InstanceIndex)
									{
										SceneComp = static_cast<USceneComponent*>(FindObjectWithOuter(ArchetypeInstances[InstanceIndex], SelectedTemplate->GetClass(), SelectedTemplate->GetFName()));
										if(SceneComp)
										{
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeLocation_DirectMutable(), OldRelativeLocation, SelectedTemplate->GetRelativeLocation());
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeRotation_DirectMutable(), OldRelativeRotation, SelectedTemplate->GetRelativeRotation());
											FComponentEditorUtils::ApplyDefaultValueChange(SceneComp, SceneComp->GetRelativeScale3D_DirectMutable(), OldRelativeScale3D, SelectedTemplate->GetRelativeScale3D());
										}
									}
								}
							}
						}
					}
				}
				GUnrealEd->RedrawLevelEditingViewports();
			}
		}

		Invalidate();
	}

	return bHandled;
}

void FSCSUIEditorViewportClient::TrackingStarted( const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge )
{
	if( !bIsManipulating && bIsDraggingWidget )
	{
		// Suspend component modification during each delta step to avoid recording unnecessary overhead into the transaction buffer
		GEditor->DisableDeltaModification(true);

		// Begin transaction
		BeginTransaction( NSLOCTEXT("UnrealEd", "ModifyComponents", "Modify Component(s)") );
		bIsManipulating = true;
	}
}

void FSCSUIEditorViewportClient::TrackingStopped() 
{
	if( bIsManipulating )
	{
		// Re-run construction scripts if we haven't done so yet (so that the components in the preview actor can update their transforms)
		AActor* PreviewActor = GetPreviewActor();
		if(PreviewActor != nullptr)
		{
			UBlueprint* PreviewBlueprint = UBlueprint::GetBlueprintFromClass(PreviewActor->GetClass());
			if(PreviewBlueprint != nullptr && !PreviewBlueprint->bRunConstructionScriptOnDrag)
			{
				PreviewActor->RerunConstructionScripts();
			}
		}

		// End transaction
		bIsManipulating = false;
		EndTransaction();

		// Restore component delta modification
		GEditor->DisableDeltaModification(false);
	}
}

FWidget::EWidgetMode FSCSUIEditorViewportClient::GetWidgetMode() const
{
	// Default to not drawing the widget
	FWidget::EWidgetMode ReturnWidgetMode = FWidget::WM_None;

	AActor* PreviewActor = GetPreviewActor();
	if(!bIsSimulateEnabled && PreviewActor)
	{
		const TSharedPtr<FUIBlueprintEditor> BluePrintEditor = BlueprintEditorPtr.Pin();
		if ( BluePrintEditor.IsValid() )
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BluePrintEditor->GetSelectedSCSComponentEditorTreeNodes();
			if (BluePrintEditor->GetSCSComponentEditor()->GetActorNode().IsValid())
			{
				const TArray<FSCSComponentEditorTreeNodePtrType>& RootNodes = BluePrintEditor->GetSCSComponentEditor()->GetActorNode()->GetComponentNodes();

				if (GUnrealEd->ComponentVisManager.IsActive() &&
					GUnrealEd->ComponentVisManager.IsVisualizingArchetype())
				{
					// Component visualizer is active and editing the archetype
					ReturnWidgetMode = WidgetMode;
				}
				else
				{
					// if the selected nodes array is empty, or only contains entries from the
					// root nodes array, or isn't visible in the preview actor, then don't display a transform widget
					for (int32 CurrentNodeIndex = 0; CurrentNodeIndex < SelectedNodes.Num(); CurrentNodeIndex++)
					{
						FSCSComponentEditorTreeNodePtrType CurrentNodePtr = SelectedNodes[CurrentNodeIndex];
						if ((CurrentNodePtr.IsValid() &&
							((!RootNodes.Contains(CurrentNodePtr) && !CurrentNodePtr->IsRootComponent()) ||
							(CurrentNodePtr->GetObject<UInstancedStaticMeshComponent>() && // show widget if we are editing individual instances even if it is the root component
								CastChecked<UInstancedStaticMeshComponent>(CurrentNodePtr->FindComponentInstanceInActor(GetPreviewActor()))->SelectedInstances.Contains(true))) &&
							CurrentNodePtr->CanEdit() &&
							CurrentNodePtr->FindComponentInstanceInActor(PreviewActor)))
						{
							// a non-NULL, non-root item is selected, draw the widget
							ReturnWidgetMode = WidgetMode;
							break;
						}
					}
				}
			}
		}
	}

	return ReturnWidgetMode;
}

void FSCSUIEditorViewportClient::SetWidgetMode( FWidget::EWidgetMode NewMode )
{
	WidgetMode = NewMode;
}

void FSCSUIEditorViewportClient::SetWidgetCoordSystemSpace( ECoordSystem NewCoordSystem ) 
{
	WidgetCoordSystem = NewCoordSystem;
}

FVector FSCSUIEditorViewportClient::GetWidgetLocation() const
{
	FVector ComponentVisWidgetLocation;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetWidgetLocation(this, ComponentVisWidgetLocation))
	{
		return ComponentVisWidgetLocation;
	}

	FVector Location = FVector::ZeroVector;

	AActor* PreviewActor = GetPreviewActor();
	if(PreviewActor)
	{
		TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
		if(SelectedNodes.Num() > 0)
		{
			// Use the last selected item for the widget location
			USceneComponent* SceneComp = Cast<USceneComponent>(SelectedNodes.Last().Get()->FindComponentInstanceInActor(PreviewActor));
			if( SceneComp )
			{
				TSharedPtr<ISCSEditorCustomization> Customization = BlueprintEditorPtr.Pin()->CustomizeSCSEditor(SceneComp);
				FVector CustomLocation;
				if(Customization.IsValid() && Customization->HandleGetWidgetLocation(SceneComp, CustomLocation))
				{
					Location = CustomLocation;
				}
				else
				{
					Location = SceneComp->GetComponentLocation();
				}
			}
		}
	}

	return Location;
}

FMatrix FSCSUIEditorViewportClient::GetWidgetCoordSystem() const
{
	FMatrix ComponentVisWidgetCoordSystem;
	if (GUnrealEd->ComponentVisManager.IsVisualizingArchetype() &&
		GUnrealEd->ComponentVisManager.GetCustomInputCoordinateSystem(this, ComponentVisWidgetCoordSystem))
	{
		return ComponentVisWidgetCoordSystem;
	}

	FMatrix Matrix = FMatrix::Identity;
	if (GetWidgetCoordSystemSpace() == COORD_Local)
	{
		AActor* PreviewActor = GetPreviewActor();
		const TSharedPtr<FUIBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin();
		if (PreviewActor && BlueprintEditor.IsValid())
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditor->GetSelectedSCSComponentEditorTreeNodes();
			if(SelectedNodes.Num() > 0)
			{
				const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes.Last();
				USceneComponent* SceneComp = SelectedNode.IsValid() ? Cast<USceneComponent>(SelectedNode->FindComponentInstanceInActor(PreviewActor)) : nullptr;
				if( SceneComp )
				{
					const TSharedPtr<ISCSEditorCustomization> Customization = BlueprintEditor->CustomizeSCSEditor(SceneComp);
					FMatrix CustomTransform;
					if(Customization.IsValid() && Customization->HandleGetWidgetTransform(SceneComp, CustomTransform))
					{
						Matrix = CustomTransform;
					}					
					else
					{
						Matrix = FQuatRotationMatrix( SceneComp->GetComponentQuat() );
					}
				}
			}
		}
	}

	if(!Matrix.Equals(FMatrix::Identity))
	{
		Matrix.RemoveScaling();
	}

	return Matrix;
}

int32 FSCSUIEditorViewportClient::GetCameraSpeedSetting() const
{
	return GetDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed;
}

void FSCSUIEditorViewportClient::SetCameraSpeedSetting(int32 SpeedSetting)
{
	GetMutableDefault<UEditorPerProjectUserSettings>()->SCSViewportCameraSpeed = SpeedSetting;
}

void FSCSUIEditorViewportClient::InvalidatePreview(bool bResetCamera)
{
	// Ensure that the editor is valid before continuing
	if(!BlueprintEditorPtr.IsValid())
	{
		return;
	}

	UBlueprint* Blueprint = BlueprintEditorPtr.Pin()->GetBlueprintObj();
	check(Blueprint);

	const bool bIsPreviewActorValid = GetPreviewActor() != nullptr;

#if SUPPORT_UI_BLUEPRINT_EDITOR
	// Create or update the Blueprint actor instance in the preview scene
	BlueprintEditorPtr.Pin()->UpdatePreviewWidgetActor(Blueprint, true);
#else
	// Create or update the Blueprint actor instance in the preview scene
	BlueprintEditorPtr.Pin()->UpdatePreviewWidgetActor(Blueprint, !bIsPreviewActorValid);
#endif

	Invalidate();
	RefreshPreviewBounds();
	
	if( bResetCamera )
	{
#if SUPPORT_UI_BLUEPRINT_EDITOR
		//ResetCamera();
#else
		ResetCamera();
#endif
	}
}

void FSCSUIEditorViewportClient::ResetCamera()
{
	const UBlueprint* Blueprint = BlueprintEditorPtr.Pin()->GetBlueprintObj();

	// For now, loosely base default camera positioning on thumbnail preview settings
	USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(Blueprint->ThumbnailInfo);
	if(ThumbnailInfo == nullptr)
	{
		ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();
	}

	// Clamp zoom to the actor's bounding sphere radius
	float OrbitZoom = ThumbnailInfo->OrbitZoom;
	if (PreviewActorBounds.SphereRadius + OrbitZoom < 0)
	{
		OrbitZoom = -PreviewActorBounds.SphereRadius;
	}

	ToggleOrbitCamera(true);
	{
		float TargetDistance = PreviewActorBounds.SphereRadius;
		if(TargetDistance <= 0.0f)
		{
			TargetDistance = AutoViewportOrbitCameraTranslate;
		}

		const FRotator ThumbnailAngle(ThumbnailInfo->OrbitPitch, ThumbnailInfo->OrbitYaw, 0.0f);

		SetViewLocationForOrbiting(PreviewActorBounds.Origin);
		SetViewLocation( GetViewLocation() + FVector(0.0f, TargetDistance * 1.5f + OrbitZoom - AutoViewportOrbitCameraTranslate, 0.0f) );
		SetViewRotation( ThumbnailAngle );
	}

	Invalidate();
}

void FSCSUIEditorViewportClient::ToggleRealtimePreview()
{
	SetRealtime(!IsRealtime());

	Invalidate();
}

AActor* FSCSUIEditorViewportClient::GetPreviewActor() const
{
	return BlueprintEditorPtr.Pin()->GetPreviewActor();
}

void FSCSUIEditorViewportClient::ZoomToFit()
{
	if (bZoomToFit && IsOrtho() && GetPreviewActor() && GetPreviewActor()->GetRootComponent())
	{
		const FIntPoint ViewportSize = Viewport->GetSizeXY();
		if (ViewportSize.X <= 0 || ViewportSize.Y <= 0)
		{
			return;
		}
		
		bZoomToFit = false;

		FVector WorldCorners[4];
		const auto Comp = Cast<URectTransformComponent>(GetPreviewActor()->GetRootComponent());
		if (!IsValid(Comp))
		{
			return;
		}
		Comp->GetWorldCorners(WorldCorners);

		const FVector ZoomPosition = FVector::ZeroVector;
		const FVector ZoomSize = WorldCorners[2] - WorldCorners[0];
		
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

void FSCSUIEditorViewportClient::ResetZoom()
{
	bZoomToFit = true;
}

void FSCSUIEditorViewportClient::FocusViewportToSelection()
{
	AActor* PreviewActor = GetPreviewActor();
	if(PreviewActor)
	{
		TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
		if(SelectedNodes.Num() > 0)
		{
			// Use the last selected item for the widget location
			USceneComponent* SceneComp = Cast<USceneComponent>(SelectedNodes.Last()->FindComponentInstanceInActor(PreviewActor));
			if( SceneComp )
			{
				FocusViewportOnBox( SceneComp->Bounds.GetBox() );
			}
		}
		else
		{
			FocusViewportOnBox( PreviewActor->GetComponentsBoundingBox( true ) );
		}
	}
}

bool FSCSUIEditorViewportClient::GetIsSimulateEnabled() const
{ 
	return bIsSimulateEnabled;
}

void FSCSUIEditorViewportClient::ToggleIsSimulateEnabled() 
{
	// Must destroy existing actors before we toggle the world state
	BlueprintEditorPtr.Pin()->DestroyPreview();

	bIsSimulateEnabled = !bIsSimulateEnabled;
	PreviewScene->GetWorld()->bBegunPlay = bIsSimulateEnabled;
	PreviewScene->GetWorld()->bShouldSimulatePhysics = bIsSimulateEnabled;

	const TSharedPtr<SWidget> SCSComponentEditor = BlueprintEditorPtr.Pin()->GetSCSComponentEditor();
	const TSharedRef<SKismetInspector> Inspector = BlueprintEditorPtr.Pin()->GetInspector();

	// When simulate is enabled, we don't want to allow the user to modify the components
	BlueprintEditorPtr.Pin()->UpdatePreviewActor(BlueprintEditorPtr.Pin()->GetBlueprintObj(), true);

	SCSComponentEditor->SetEnabled(!bIsSimulateEnabled);
	Inspector->SetEnabled(!bIsSimulateEnabled);

	if(!IsRealtime())
	{
		ToggleRealtimePreview();
	}
}

void FSCSUIEditorViewportClient::ToggleShowRaycastRegion() const
{
	auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();

	bool bShowRaycastRegion = Settings->bShowRaycastRegion;

	bShowRaycastRegion = !bShowRaycastRegion;
	Settings->bShowRaycastRegion = bShowRaycastRegion;
	Settings->PostEditChange();
}

bool FSCSUIEditorViewportClient::IsShowRaycast() const
{
	return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bShowRaycastRegion;
}

void FSCSUIEditorViewportClient::ToggleTrackSelectedComponent() const
{
	auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();

	bool bTrackSelectedComponent = Settings->bTrackSelectedComponent;

	bTrackSelectedComponent = !bTrackSelectedComponent;
	Settings->bTrackSelectedComponent = bTrackSelectedComponent;
	Settings->PostEditChange();
}

bool FSCSUIEditorViewportClient::GetTrackSelectedComponent() const
{
	return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bTrackSelectedComponent;
}

void FSCSUIEditorViewportClient::ToggleRawEditMode() const
{
	auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();

	bool bRawEditMode = Settings->bRawEditMode;
	bRawEditMode = !bRawEditMode;
	Settings->bRawEditMode = bRawEditMode;
	Settings->PostEditChange();
}

bool FSCSUIEditorViewportClient::GetRawEditMode() const
{
	return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bRawEditMode;
}

void FSCSUIEditorViewportClient::ToggleShowBackgroundImage()
{
	auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();

	bool bShowBackgroundImage = Settings->bShowBackgroundImage;
	bShowBackgroundImage = !bShowBackgroundImage;
	Settings->bShowBackgroundImage = bShowBackgroundImage;
	Settings->PostEditChange();
}

bool FSCSUIEditorViewportClient::GetShowBackgroundImage()
{
	return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bShowBackgroundImage;
}

void FSCSUIEditorViewportClient::ToggleShowStats()
{
	auto* Settings = GetMutableDefault<UUIEditorPerProjectUserSettings>();
	
	bShowStats = !bShowStats;
	Settings->bShowStats = bShowStats;
	Settings->PostEditChange();
}

bool FSCSUIEditorViewportClient::GetShowStats()
{
	return GetMutableDefault<UUIEditorPerProjectUserSettings>()->bShowStats;
}

void FSCSUIEditorViewportClient::BeginTransaction(const FText& Description)
{
	if(!ScopedTransaction)
	{
		ScopedTransaction = new FScopedTransaction(Description);

		const TSharedPtr<FUIBlueprintEditor> BlueprintEditor = BlueprintEditorPtr.Pin();
		if (BlueprintEditor.IsValid())
		{
			UBlueprint* PreviewBlueprint = BlueprintEditor->GetBlueprintObj();
			if (PreviewBlueprint != nullptr)
			{
				FBlueprintEditorUtils::MarkBlueprintAsModified(PreviewBlueprint);
			}

			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditor->GetSelectedSCSComponentEditorTreeNodes();
			for (const FSCSComponentEditorTreeNodePtrType& Node : SelectedNodes)
			{
				if(Node.IsValid())
				{
					if(USCS_Node* SCS_Node = Node->GetSCSNode())
					{
						USimpleConstructionScript* SCS = SCS_Node->GetSCS();
						UBlueprint* Blueprint = SCS ? SCS->GetBlueprint() : nullptr;
						if (Blueprint == PreviewBlueprint)
						{
							SCS_Node->Modify();
						}
					}

					// Modify template, any instances will be reconstructed as part of PostUndo:
					UActorComponent* ComponentTemplate = Node->GetOrCreateEditableComponentTemplate(PreviewBlueprint);
					if (ComponentTemplate != nullptr)
					{
						ComponentTemplate->SetFlags(RF_Transactional);
						ComponentTemplate->Modify();
					}
				}
			}
		}
	}
}

void FSCSUIEditorViewportClient::EndTransaction()
{
	if(ScopedTransaction)
	{
		delete ScopedTransaction;
		ScopedTransaction = nullptr;
	}
}

void FSCSUIEditorViewportClient::RefreshPreviewBounds()
{
	AActor* PreviewActor = GetPreviewActor();

	if(PreviewActor)
	{
		// Compute actor bounds as the sum of its visible parts
		PreviewActorBounds = FBoxSphereBounds(ForceInitToZero);
		for (UActorComponent* Component : PreviewActor->GetComponents())
		{
			// Aggregate primitive components that either have collision enabled or are otherwise visible components in-game
			if (const UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
			{
				if (PrimComp->IsRegistered() && (!PrimComp->bHiddenInGame || PrimComp->IsCollisionEnabled()) && PrimComp->Bounds.SphereRadius < HALF_WORLD_MAX)
				{
					PreviewActorBounds = PreviewActorBounds + PrimComp->Bounds;
				}
			}
		}
	}
}

void FSCSUIEditorViewportClient::OnUIBlueprintEditorBeginTransaction(UActorComponent* ActorComponent,
	const FText& Description)
{
	if (!IsValid(ActorComponent) || !ActorComponent->IsTemplate())
	{
		return;
	}

	if(ActorComponent->HasAnyFlags(RF_ArchetypeObject))
	{
		TArray<UObject*> ArchetypeInstances;
		ActorComponent->GetArchetypeInstances(ArchetypeInstances);
		for(int32 InstanceIndex = 0; InstanceIndex < ArchetypeInstances.Num(); ++InstanceIndex)
		{
			const auto ActorInstanceComp = Cast<UActorComponent>(ArchetypeInstances[InstanceIndex]);
			if(ActorInstanceComp != nullptr && ActorInstanceComp->GetWorld() == PreviewScene->GetWorld())
			{
				BeginTransaction(Description);
				break;
			}
		}
	}
}

void FSCSUIEditorViewportClient::OnUIBlueprintEditorEndTransaction(UActorComponent* ActorComponent)
{
	if (!IsValid(ActorComponent) || !ActorComponent->IsTemplate())
	{
		return;
	}

	if(ActorComponent->HasAnyFlags(RF_ArchetypeObject))
	{
		TArray<UObject*> ArchetypeInstances;
		ActorComponent->GetArchetypeInstances(ArchetypeInstances);
		for(int32 InstanceIndex = 0; InstanceIndex < ArchetypeInstances.Num(); ++InstanceIndex)
		{
			const auto ActorInstanceComp = Cast<UActorComponent>(ArchetypeInstances[InstanceIndex]);
			if(ActorInstanceComp != nullptr && ActorInstanceComp->GetWorld() == PreviewScene->GetWorld())
			{
				EndTransaction();
				break;
			}
		}
	}
}
