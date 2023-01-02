#include "UIEditorViewport/SUGUIEditorViewport.h"
#include "Slate/SceneViewport.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorViewportClient.h"
#include "SEditorViewport.h"
#include "UGUISettings.h"

#define LOCTEXT_NAMESPACE "UIBlueprintEditor"

/*-----------------------------------------------------------------------------
	SUGUIEditorViewport
-----------------------------------------------------------------------------*/

void SUGUIEditorViewport::Construct(const FArguments& InArgs, FName ViewportName, TSharedRef<class FUGUIEditorViewportClient> InViewportClient)
{
	OnPopulateViewportOverlays = InArgs._OnPopulateViewportOverlays;
	
	bIsActiveTimerRegistered = false;

	ViewportClient = InViewportClient;
	ViewportClient->SetupViewportInfo(ViewportName, ViewportClient, SharedThis(this));
	
	SEditorViewport::Construct( SEditorViewport::FArguments() );

	// Restore last used feature level
	if (ViewportClient.IsValid())
	{
		UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
		if (World != nullptr)
		{
			World->ChangeFeatureLevel(GWorld->FeatureLevel);
		}
	}

	// Use a delegate to inform the attached world of feature level changes.
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	PreviewFeatureLevelChangedHandle = Editor->OnPreviewFeatureLevelChanged().AddLambda([this](ERHIFeatureLevel::Type NewFeatureLevel)
		{
			if(ViewportClient.IsValid())
			{
				UWorld* World = ViewportClient->GetPreviewScene()->GetWorld();
				if (World != nullptr)
				{
					World->ChangeFeatureLevel(NewFeatureLevel);

					// Refresh the preview scene. Don't change the camera.
					RequestRefresh(false);
				}
			}
		});

	// Refresh the preview scene
	RequestRefresh(true);
}

SUGUIEditorViewport::~SUGUIEditorViewport()
{
	UEditorEngine* Editor = (UEditorEngine*)GEngine;
	Editor->OnPreviewFeatureLevelChanged().Remove(PreviewFeatureLevelChangedHandle);

	if(ViewportClient.IsValid())
	{
		// Reset this to ensure it's no longer in use after destruction
		ViewportClient->Viewport = nullptr;
	}
}

TSharedRef<FEditorViewportClient> SUGUIEditorViewport::MakeEditorViewportClient()
{
	return ViewportClient.ToSharedRef();
}

void SUGUIEditorViewport::Invalidate()
{
	ViewportClient->Invalidate();
}

void SUGUIEditorViewport::RequestRefresh(bool bResetCamera, bool bRefreshNow)
{
	if(bRefreshNow)
	{
		if(ViewportClient.IsValid())
		{
			ViewportClient->InvalidatePreview(bResetCamera);
		}
	}
	else
	{
		// Defer the update until the next tick. This way we don't accidentally spawn the preview actor in the middle of a transaction, for example.
		if (!bIsActiveTimerRegistered)
		{
			bIsActiveTimerRegistered = true;
			RegisterActiveTimer(0.f, FWidgetActiveTimerDelegate::CreateSP(this, &SUGUIEditorViewport::DeferredUpdatePreview, bResetCamera));
		}
	}
}

void SUGUIEditorViewport::PopulateViewportOverlays(TSharedRef<SOverlay> Overlay)
{
	SEditorViewport::PopulateViewportOverlays(Overlay);

	OnPopulateViewportOverlays.ExecuteIfBound(Overlay);
}

void SUGUIEditorViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SEditorViewport::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (ViewportClient.IsValid())
	{
		ViewportClient->bNeedsRedraw = true;
	}

	//if (ViewportClient.IsValid() && FSlateThrottleManager::Get().IsAllowingExpensiveTasks())
	//{
	// Tick and render the viewport
	//	ViewportClient->Tick(InDeltaTime);
	//	GEditor->UpdateSingleViewportClient(ViewportClient.Get(), /*bInAllowNonRealtimeViewportToDraw=*/ true, /*bLinkedOrthoMovement=*/ false);
	//}
}

EActiveTimerReturnType SUGUIEditorViewport::DeferredUpdatePreview(double InCurrentTime, float InDeltaTime, bool bResetCamera)
{
	if (ViewportClient.IsValid())
	{
		ViewportClient->InvalidatePreview(bResetCamera);
	}

	bIsActiveTimerRegistered = false;
	return EActiveTimerReturnType::Stop;
}

#undef LOCTEXT_NAMESPACE
