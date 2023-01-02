#include "Designer/UIPrimitiveSelectComponent.h"
#include "EditorSupportDelegates.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "Core/Widgets/ImageSubComponent.h"
#include "SCSUIEditorViewportClient.h"
#include "SSCSUIEditorViewport.h"
#include "UGUISubsystem.h"
#include "UIBlueprintEditor.h"
#include "UIEditorPerProjectUserSettings.h"
#include "Core/Layout/RectTransformUtility.h"

/////////////////////////////////////////////////////
// UUIPrimitiveSelectComponent

UUIPrimitiveSelectComponent::UUIPrimitiveSelectComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bHighPriority = true;
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;

	SelectedRectDrawerComponent = nullptr;
	SelectedParentCompInstance = nullptr;
	CheckBoxDrawComponent = nullptr;

	Transition = ESelectableTransition::Transition_None;
	LastMouseScreenPos = FVector2D(-1, -1);
	bHasDragged = false;
	SelectedCompInstance = nullptr;

	DragMode = EUISelectDragMode::None;

	SelectedRectTopLeft = FVector2D::ZeroVector;
	SelectedRectBottomRight = FVector2D::ZeroVector;
	bHasValidSelectedRect = false;

	CheckRectTopLeft = FVector2D::ZeroVector;
	CheckRectBottomRight = FVector2D::ZeroVector;
	bHasValidCheckRect = false;
}

void UUIPrimitiveSelectComponent::Awake()
{
	Super::Awake();

	const auto ImageComp = Cast<UImageSubComponent>(GetComponent(UImageSubComponent::StaticClass(), true));
	if (IsValid(ImageComp))
	{
#if WITH_EDITOR
		ImageComp->bRegisterGraphicForEditor = true;
#endif
	}

	const auto RendererComp = Cast<UCanvasRendererSubComponent>(GetComponent(UCanvasRendererSubComponent::StaticClass(), true));
	if (IsValid(RendererComp))
	{
		RendererComp->SetHidePrimitive(true);
	}

	SetAnchorAndOffset(FVector2D::ZeroVector, FVector2D(1, 1), FVector2D::ZeroVector, FVector2D::ZeroVector);
}

void UUIPrimitiveSelectComponent::TickComponent(float DeltaTime, ELevelTick Tick,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);

	SelectedRectTopLeft = FVector2D::ZeroVector;
	SelectedRectBottomRight = FVector2D::ZeroVector;
	bHasValidSelectedRect = false;

	CenterDragBox = FRectBox();
	
	LeftEdgeBox = FRectBox();
	RightEdgeBox = FRectBox();
	TopEdgeBox = FRectBox();
	BottomEdgeBox = FRectBox();

	TopLeftPointBox = FRectBox();
	TopRightPointBox = FRectBox();
	BottomLeftPointBox = FRectBox();
	BottomRightPointBox = FRectBox();

	EMouseCursor::Type Cursor = EMouseCursor::None;
	
	bool bReturn = false;
	
	if (!IsEnabledInHierarchy())
	{
		bReturn = true;
	}

	TWeakPtr<class FUIBlueprintEditor> BlueprintEditorPtr = nullptr;
	const AActor* WidgetActor = nullptr;
	
	if (!ViewportClient.IsValid())
	{
		bReturn = true;
	}
	else
	{
		BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
		if (!BlueprintEditorPtr.IsValid())
		{
			bReturn = true;
		}
	
		WidgetActor = ViewportClient.Pin()->GetPreviewActor();
		if (!IsValid(WidgetActor))
		{
			bReturn = true;
		}

		const auto UIEditorViewport = ViewportClient.Pin()->UIEditorViewport;
		if (UIEditorViewport.IsValid())
		{
			if (!UIEditorViewport.Pin()->Is2DViewportType())
			{
				bReturn = true;
			}
			else if (UIEditorViewport.Pin()->IsSimulateGameOverlay())
			{
				bReturn = true;
			}
		}
	}
	
	if (!bReturn)
	{
		float TopLeftX = MAX_flt;
		float TopLeftY = MAX_flt;
		float BottomRightX = -MAX_flt;
		float BottomRightY = -MAX_flt;

		bool bHasValidTopLeftX = false;
		bool bHasValidTopLeftY = false;
		bool bHasValidBottomRightX = false;
		bool bHasValidBottomRightY = false;

		if (BlueprintEditorPtr.IsValid() && IsValid(WidgetActor))
		{
			TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
			for (int32 SelectionIndex = 0; SelectionIndex < SelectedNodes.Num(); ++SelectionIndex)
			{
				const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes[SelectionIndex];

				UActorComponent* Comp = SelectedNode->FindComponentInstanceInActor(WidgetActor);
				if(Comp != nullptr && Comp->IsRegistered())
				{
					URectTransformComponent* RectComp = Cast<URectTransformComponent>(Comp);
					if (RectComp)
					{
						FVector WorldCorners[4];
						RectComp->GetWorldCorners(WorldCorners);

						for (int32 Index = 0; Index < 4; ++Index)
						{
							FVector2D ViewportPos;
							if (GetViewportPoint(WorldCorners[Index], ViewportPos))
							{
								if (ViewportPos.X < TopLeftX)
								{
									TopLeftX = ViewportPos.X;
									bHasValidTopLeftX = true;
								}

								if (ViewportPos.X > BottomRightX)
								{
									BottomRightX = ViewportPos.X;
									bHasValidBottomRightX = true;
								}

								if (ViewportPos.Y < TopLeftY)
								{
									TopLeftY = ViewportPos.Y;
									bHasValidTopLeftY = true;
								}

								if (ViewportPos.Y > BottomRightY)
								{
									BottomRightY = ViewportPos.Y;
									bHasValidBottomRightY = true;
								}
							}
						}
					}
				}
			}
		}

		SelectedRectTopLeft = FVector2D(TopLeftX, TopLeftY);
		SelectedRectBottomRight = FVector2D(BottomRightX, BottomRightY);
		bHasValidSelectedRect = bHasValidTopLeftX && bHasValidBottomRightX && bHasValidTopLeftY && bHasValidBottomRightY;

		if (bHasValidSelectedRect)
		{
			int32 SuccessCount = 0;
			const FVector2D TopLeftScreenPos = SelectedRectTopLeft * ViewportClient.Pin()->EditorViewportSize;
			FVector2D TopLeftLocalPos;
			if (ScreenPointToLocalPoint(this, TopLeftScreenPos, TopLeftLocalPos))
			{
				++SuccessCount;
			}

			const FVector2D BottomRightScreenPos = SelectedRectBottomRight * ViewportClient.Pin()->EditorViewportSize;
			FVector2D BottomRightLocalPos;
			if (ScreenPointToLocalPoint(this, BottomRightScreenPos, BottomRightLocalPos))
			{
				++SuccessCount;
			}

			float DotAlpha = FMath::Clamp((BottomRightScreenPos.X - TopLeftScreenPos.X) / 32.0f, 0.0f, 1.0f);
			DotAlpha = FMath::Min(DotAlpha, FMath::Clamp((BottomRightScreenPos.Y - TopLeftScreenPos.Y) / 32.0f, 0.0f, 1.0f));
			
			if (IsValid(SelectedRectDrawerComponent))
			{
				if (SuccessCount >= 2)
				{
					SelectedRectDrawerComponent->SetRect(TopLeftLocalPos, BottomRightLocalPos, DotAlpha);

					CenterDragBox.SetMinMax(TopLeftScreenPos, BottomRightScreenPos);

					LeftEdgeBox.SetMinMax(TopLeftScreenPos - FVector2D(8, 0), FVector2D(TopLeftScreenPos.X, BottomRightScreenPos.Y) + FVector2D(8, 0));
					RightEdgeBox.SetMinMax(FVector2D(BottomRightScreenPos.X, TopLeftScreenPos.Y) - FVector2D(8, 0), BottomRightScreenPos + FVector2D(8, 0));
					TopEdgeBox.SetMinMax(TopLeftScreenPos + FVector2D(8, -8), FVector2D(BottomRightScreenPos.X, TopLeftScreenPos.Y) + FVector2D(-8, 8));
					BottomEdgeBox.SetMinMax(FVector2D(TopLeftScreenPos.X, BottomRightScreenPos.Y) + FVector2D(8, -8), BottomRightScreenPos + FVector2D(-8, 8));

					if (DotAlpha >= 0.8)
					{
						TopLeftPointBox.SetCenterSize(TopLeftScreenPos, FVector2D(8, 8));
						TopRightPointBox.SetCenterSize(FVector2D(BottomRightScreenPos.X, TopLeftScreenPos.Y), FVector2D(8, 8));
						BottomLeftPointBox.SetCenterSize(FVector2D(TopLeftScreenPos.X, BottomRightScreenPos.Y), FVector2D(8, 8));
						BottomRightPointBox.SetCenterSize(BottomRightScreenPos, FVector2D(8, 8));
					}

					FVector2D MousePosition = FVector2D::ZeroVector;

					if (ViewportClient.IsValid() && ViewportClient.Pin()->Viewport && FSlateApplication::Get().IsMouseAttached())
					{
						FIntPoint MousePos;
						ViewportClient.Pin()->Viewport->GetMousePos(MousePos);
						MousePosition = FVector2D(MousePos);
					}
					
					if (TopLeftPointBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeSouthEast;
					}
					else if (TopRightPointBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeSouthWest;
					}
					else if (BottomLeftPointBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeSouthWest;
					}
					else if (BottomRightPointBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeSouthEast;
					}
					else if (LeftEdgeBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeLeftRight;
					}
					else if (RightEdgeBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeLeftRight;
					}
					else if (TopEdgeBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeUpDown;
					}
					else if (BottomEdgeBox.Contains(MousePosition))
					{
						Cursor = EMouseCursor::ResizeUpDown;
					}
				}
				else
				{
					SelectedRectDrawerComponent->SetShowLine(false);
				}
			}
		}
		else
		{
			if (IsValid(SelectedRectDrawerComponent))
			{
				SelectedRectDrawerComponent->SetShowLine(false);
			}
		}
	}
	else
	{
		if (IsValid(SelectedRectDrawerComponent))
		{
			SelectedRectDrawerComponent->SetShowLine(false);
		}
	}
	
	if (ViewportClient.IsValid())
	{ 
		if (bIsPointerDown)
		{
			ViewportClient.Pin()->ViewportMouseCursor = PressCursor;
		}
		else
		{
			ViewportClient.Pin()->ViewportMouseCursor = Cursor;
		}
	}

	if (bHasValidCheckRect)
	{
	    CheckSelectedComponents();
	}
}

bool UUIPrimitiveSelectComponent::GetViewportPoint(const FVector& WorldPos, FVector2D& ViewportPos) const
{
	if (!ViewportClient.IsValid())
	{
		return false;
	}
	
	const FVector4& ClipSpacePos = ViewportClient.Pin()->ViewProjectionMatrix.TransformPosition(WorldPos);
	if (ClipSpacePos.W > 0.0f)
	{
		// the result of this will be x and y coords in -1..1 projection space
		const float RHW = 1.0f / ClipSpacePos.W;

		// Move from projection space to normalized 0..1 UI space
		const float NormalizedX = (ClipSpacePos.X * RHW * 0.5f) + 0.5f;
		//const float NormalizedY = 1.f - (ClipSpacePos.Y * RHW * 0.5f) - 0.5f;
		const float NormalizedY = -(ClipSpacePos.Y * RHW * 0.5f) + 0.5f;

		const float NormalizedZ = ClipSpacePos.Z * RHW;

		if (NormalizedZ >= 0 && NormalizedZ <= 1)
		{
			ViewportPos = FVector2D(NormalizedX, NormalizedY);
			return true;
		}
	}

	return false;
}

void UUIPrimitiveSelectComponent::OnPointerDown(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	Super::OnPointerDown(EventData);

	bHasDragged = false;
	DragMode = EUISelectDragMode::None;

	LastDragScreenPos = FVector2D(EventData->Position);
	
	PressCursor = EMouseCursor::None;
	if (TopLeftPointBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::TopLeftPoint;
		PressCursor = EMouseCursor::ResizeSouthEast;
	}
	else if (TopRightPointBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::TopRightPoint;
		PressCursor = EMouseCursor::ResizeSouthWest;
	}
	else if (BottomLeftPointBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::BottomLeftPoint;
		PressCursor = EMouseCursor::ResizeSouthWest;
	}
	else if (BottomRightPointBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::BottomRightPoint;
		PressCursor = EMouseCursor::ResizeSouthEast;
	}
	else if (LeftEdgeBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::LeftEdge;
		PressCursor = EMouseCursor::ResizeLeftRight;
	}
	else if (RightEdgeBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::RightEdge;
		PressCursor = EMouseCursor::ResizeLeftRight;
	}
	else if (TopEdgeBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::TopEdge;
		PressCursor = EMouseCursor::ResizeUpDown;
	}
	else if (BottomEdgeBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::BottomEdge;
		PressCursor = EMouseCursor::ResizeUpDown;
	}
	else if (CenterDragBox.Contains(LastDragScreenPos))
	{
		DragMode = EUISelectDragMode::CenterBox;
	}
}

void UUIPrimitiveSelectComponent::OnBeginDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	bHasDragged = true;

	if (DragMode != EUISelectDragMode::None)
	{
		if (!ViewportClient.IsValid())
		{
			return;
		}

		if (DragMode == EUISelectDragMode::CenterBox)
		{
			ViewportClient.Pin()->BeginTransaction(NSLOCTEXT("UIBlueprintEditor", "MoveRectTransforms", "Move RectTransform(s)") );
		}
		else
		{
			ViewportClient.Pin()->BeginTransaction(NSLOCTEXT("UIBlueprintEditor", "ResizeRectTransforms", "Resize RectTransform(s)") );
		}
	}
	else
	{
		const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
		if(!BlueprintEditorPtr.IsValid())
		{
			return;
		}

		BlueprintEditorPtr.Pin()->ClearSelectionStateFor(TEXT("Components"));
        CheckRectTopLeft = FVector2D(EventData->Position.X,EventData->Position.Y);
		bHasValidCheckRect = true;
	}
}

void UUIPrimitiveSelectComponent::OnDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if(DragMode != EUISelectDragMode::None )
	{
        if (DragMode == EUISelectDragMode::CenterBox)
	    {
		    DragSelectedComponents(FVector2D(EventData->Position));
	    }
        else
        {
            ResizeSelectedComponents(FVector2D(EventData->Position));
        }
	}
	else
	{
        CheckRectBottomRight = FVector2D(EventData->Position.X, EventData->Position.Y);
		UpdateCheckBox(true);
	}

	LastDragScreenPos = FVector2D(EventData->Position); // TODO 如果是倾斜的移动
}

void UUIPrimitiveSelectComponent::OnEndDrag(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	if (DragMode != EUISelectDragMode::None)
	{
		if (!ViewportClient.IsValid())
		{
			return;
		}

		ViewportClient.Pin()->EndTransaction();
	}

	DragMode = EUISelectDragMode::None;
	UpdateCheckBox(false);
	bHasValidCheckRect = false;
}

void UUIPrimitiveSelectComponent::DragSelectedComponents(const FVector2D& CurScreenPos)
{
	if (!ViewportClient.IsValid())
	{
		return;
	}

	const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}
	
	const auto WidgetActor = ViewportClient.Pin()->GetPreviewActor();
	if (!IsValid(WidgetActor))
	{
		return;
	}
	
	TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();

	TArray<FSCSComponentEditorTreeNodePtrType> DisableNodes;

	for(const auto& SelectedNode : SelectedNodes)
	{
		auto Children = SelectedNode->GetChildren();
		for(const auto& Child : Children)
		{
			GetDisableList(Child, &DisableNodes);
		}
	}

	for (int32 SelectionIndex = 0; SelectionIndex < SelectedNodes.Num(); ++SelectionIndex)
	{
		const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes[SelectionIndex];

		if(DisableNodes.Contains(SelectedNode))
		{
			continue;
		}

		URectTransformComponent* Comp = Cast<URectTransformComponent>(SelectedNode->FindComponentInstanceInActor(WidgetActor));
		URectTransformComponent* SelectedTemplateComp = Cast<URectTransformComponent>(SelectedNode->GetOrCreateEditableComponentTemplate(BlueprintEditorPtr.Pin()->GetBlueprintObj()));
		if(Comp != nullptr && Comp->IsRegistered())
		{
			const URectTransformComponent* ParentRectComp = Cast<URectTransformComponent>(Comp->GetAttachParent());
			if (IsValid(ParentRectComp))
			{
				FVector2D LastLocalPosition = FVector2D::ZeroVector;
				if (!ScreenPointToLocalPointInRectangle(ParentRectComp, nullptr, LastDragScreenPos, LastLocalPosition, true))
				{
					continue;;	 
				}

				FVector2D CurLocalPosition = FVector2D::ZeroVector;
				if (!ScreenPointToLocalPointInRectangle(ParentRectComp, nullptr, CurScreenPos, CurLocalPosition, true))
				{
					continue;;	 
				}

				Comp->SetLocalLocation(Comp->GetLocalLocation() - FVector(LastLocalPosition - CurLocalPosition, 0));

				TArray<UObject*> ArchetypeInstances;
				SelectedTemplateComp->GetArchetypeInstances(ArchetypeInstances);

				for (const auto ArchetypeInstance : ArchetypeInstances)
				{
					URectTransformComponent* ArchetypeInstanceComp = Cast<URectTransformComponent>(ArchetypeInstance);
					if (IsValid(ArchetypeInstanceComp) && ArchetypeInstanceComp != Comp)
					{
						ArchetypeInstanceComp->Modify();
						
						ArchetypeInstanceComp->GetRelativeLocation_DirectMutable().Z = Comp->GetRelativeLocation().Z;
						ArchetypeInstanceComp->SetAnchoredPosition(Comp->GetAnchoredPosition());
						ArchetypeInstanceComp->SetOffsetMin(Comp->GetOffsetMin());
						ArchetypeInstanceComp->SetOffsetMax(Comp->GetOffsetMax());

						ArchetypeInstanceComp->MarkPackageDirty();    
					}
				}
				
				{
					SelectedTemplateComp->Modify();
				
					SelectedTemplateComp->GetRelativeLocation_DirectMutable().Z = Comp->GetRelativeLocation().Z;
					SelectedTemplateComp->SetAnchoredPosition(Comp->GetAnchoredPosition());
					SelectedTemplateComp->SetOffsetMin(Comp->GetOffsetMin());
					SelectedTemplateComp->SetOffsetMax(Comp->GetOffsetMax());

					// Update the actor before leaving.
					SelectedTemplateComp->MarkPackageDirty();
				
					// Fire callbacks
					FEditorSupportDelegates::RefreshPropertyWindows.Broadcast();
					FEditorSupportDelegates::UpdateUI.Broadcast();
				}
			}
		}
	}
}

void UUIPrimitiveSelectComponent::ResizeSelectedComponents(const FVector2D& CurScreenPos)
{
	if (!ViewportClient.IsValid())
	{
		return;
	}

	const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}
	
	const auto WidgetActor = ViewportClient.Pin()->GetPreviewActor();
	if (!IsValid(WidgetActor))
	{
		return;
	}
	
	TArray<FSCSComponentEditorTreeNodePtrType> SelectedNodes = BlueprintEditorPtr.Pin()->GetSelectedSCSComponentEditorTreeNodes();
	for (int32 SelectionIndex = 0; SelectionIndex < SelectedNodes.Num(); ++SelectionIndex)
	{
		const FSCSComponentEditorTreeNodePtrType SelectedNode = SelectedNodes[SelectionIndex];

		URectTransformComponent* Comp = Cast<URectTransformComponent>(SelectedNode->FindComponentInstanceInActor(WidgetActor));
		URectTransformComponent* SelectedTemplateComp = Cast<URectTransformComponent>(SelectedNode->GetOrCreateEditableComponentTemplate(BlueprintEditorPtr.Pin()->GetBlueprintObj()));
		if(Comp != nullptr && Comp->IsRegistered())
		{
			const URectTransformComponent* ParentRectComp = Cast<URectTransformComponent>(Comp->GetAttachParent());
			if (IsValid(ParentRectComp))
			{
				FVector2D LastLocalPosition = FVector2D::ZeroVector;
				if (!ScreenPointToLocalPointInRectangle(ParentRectComp, nullptr, LastDragScreenPos, LastLocalPosition, true))
				{
					continue;;	 
				}

				FVector2D CurLocalPosition = FVector2D::ZeroVector;
				if (!ScreenPointToLocalPointInRectangle(ParentRectComp, nullptr, CurScreenPos, CurLocalPosition, true))
				{
					continue;;	 
				}

				FVector2D OldOffsetMax = Comp->GetOffsetMax();
				FVector2D OldOffsetMin = Comp->GetOffsetMin();
				FVector2D OldSizeDelta = Comp->GetSizeDelta();

				FVector2D OffsetDelta = CurLocalPosition - LastLocalPosition;

				auto CurDragMode = DragMode;

				/*if (OldSizeDelta.X < 0)
				{
					if (CurDragMode == EUISelectDragMode::RightEdge)
					{
						CurDragMode = EUISelectDragMode::LeftEdge;
					}
				}*/
				
				if (CurDragMode == EUISelectDragMode::RightEdge)
				{
					OffsetDelta.Y = 0;
					Comp->SetOffsetMax(OldOffsetMax + OffsetDelta);
				}
				else if (CurDragMode == EUISelectDragMode::LeftEdge)
				{
					OffsetDelta.Y = 0;
					Comp->SetOffsetMin(OldOffsetMin + OffsetDelta);
				}
				else if (CurDragMode == EUISelectDragMode::TopEdge)
				{
					OffsetDelta.X = 0;
					Comp->SetOffsetMax(OldOffsetMax + OffsetDelta);
				}
				else if (CurDragMode == EUISelectDragMode::BottomEdge)
				{
					OffsetDelta.X = 0;
					Comp->SetOffsetMin(OldOffsetMin + OffsetDelta);
				}
				else if (CurDragMode == EUISelectDragMode::TopRightPoint)
				{
					Comp->SetOffsetMax(OldOffsetMax + OffsetDelta);
				}
				else if (CurDragMode == EUISelectDragMode::BottomRightPoint)
				{
					Comp->SetOffsetMax(OldOffsetMax + FVector2D(OffsetDelta.X, 0));
					Comp->SetOffsetMin(OldOffsetMin + FVector2D(0, OffsetDelta.Y));
				}
				else if (CurDragMode == EUISelectDragMode::TopLeftPoint)
				{
					Comp->SetOffsetMax(OldOffsetMax + FVector2D(0, OffsetDelta.Y));
					Comp->SetOffsetMin(OldOffsetMin + FVector2D(OffsetDelta.X, 0));
				}
				else if (CurDragMode == EUISelectDragMode::BottomLeftPoint)
				{
					Comp->SetOffsetMin(OldOffsetMin + OffsetDelta);
				}

				TArray<UObject*> ArchetypeInstances;
				SelectedTemplateComp->GetArchetypeInstances(ArchetypeInstances);

				for (const auto ArchetypeInstance : ArchetypeInstances)
				{
					URectTransformComponent* ArchetypeInstanceComp = Cast<URectTransformComponent>(ArchetypeInstance);
					if (IsValid(ArchetypeInstanceComp) && ArchetypeInstanceComp != Comp)
					{
						ArchetypeInstanceComp->Modify();
						
						ArchetypeInstanceComp->SetOffsetMin(Comp->GetOffsetMin());
						ArchetypeInstanceComp->SetOffsetMax(Comp->GetOffsetMax());
						ArchetypeInstanceComp->SetAnchoredPosition(Comp->GetAnchoredPosition());
						ArchetypeInstanceComp->SetSizeDelta(Comp->GetSizeDelta());

						ArchetypeInstanceComp->MarkPackageDirty();    
					}
				}

				// TODO bug 如果反过来的拖动就不对了
				{
					SelectedTemplateComp->Modify();
					
					SelectedTemplateComp->SetOffsetMin(Comp->GetOffsetMin());
					SelectedTemplateComp->SetOffsetMax(Comp->GetOffsetMax());
					SelectedTemplateComp->SetAnchoredPosition(Comp->GetAnchoredPosition());
					SelectedTemplateComp->SetSizeDelta(Comp->GetSizeDelta());
					
					// Update the actor before leaving.
					SelectedTemplateComp->MarkPackageDirty();
				
					// Fire callbacks
					FEditorSupportDelegates::RefreshPropertyWindows.Broadcast();
					FEditorSupportDelegates::UpdateUI.Broadcast();
				}
			}
		}
	}
}

void UUIPrimitiveSelectComponent::CheckSelectedComponents()
{
	const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}
	
	UpdateDesignerRects();

	CheckRect = GetCheckRect(CheckRectTopLeft, CheckRectBottomRight,true);
	
	for (int32 Index = DesignerRects.Num() - 1; Index >= 0; --Index)
	{
		auto DesignerRectTransform = DesignerRects[Index];
		FVector WorldCorners[4];
		DesignerRectTransform->GetWorldCorners(WorldCorners);

		FVector2D TopLeft(MAX_flt, MAX_flt);
		FVector2D BottomRight(-MAX_flt, -MAX_flt);

		for (int32 CornerIndex = 0; CornerIndex < 4; ++CornerIndex)
		{
			FVector2D ViewportPos;
			GetViewportPoint(WorldCorners[CornerIndex], ViewportPos);

			if (ViewportPos.X < TopLeft.X)
			{
				TopLeft.X = ViewportPos.X;
			}

			if (ViewportPos.X > BottomRight.X)
			{
				BottomRight.X = ViewportPos.X;
			}

			if (ViewportPos.Y < TopLeft.Y)
			{
				TopLeft.Y = ViewportPos.Y;
			}

			if (ViewportPos.Y > BottomRight.Y)
			{
				BottomRight.Y = ViewportPos.Y;
			}
		}

		FRectBox DesignerRect = GetCheckRect(TopLeft, BottomRight,false);

		if (CheckRect.Contains(DesignerRect))
		{
			if (SelectedDesignerRects.Contains(DesignerRectTransform))
			{
				continue;
			}
			
			SelectedDesignerRects.Add(DesignerRectTransform);
		}
		else
		{
			if (SelectedDesignerRects.Contains(DesignerRectTransform))
			{
				SelectedDesignerRects.Remove(DesignerRectTransform);
				BlueprintEditorPtr.Pin()->ClearSelectionStateFor("Components");
			}
		}
	}
	
	BlueprintEditorPtr.Pin()->SelectedNodes(SelectedDesignerRects);
}

void UUIPrimitiveSelectComponent::GetDisableList(FSCSComponentEditorTreeNodePtrType Node,TArray<FSCSComponentEditorTreeNodePtrType>* DisableList)
{
	if (!ViewportClient.IsValid())
	{
		return;
	}

	const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
	if (!BlueprintEditorPtr.IsValid())
	{
		return;
	}

	const auto WidgetActor = ViewportClient.Pin()->GetPreviewActor();
	if (!IsValid(WidgetActor))
	{
		return;
	}

	TArray<FSCSComponentEditorTreeNodePtrType> ChildNodes = Node->GetChildren();

	if(!DisableList->Contains(Node))
	    DisableList->Emplace(Node);

	for(const auto& ChildNode : ChildNodes)
	{
		GetDisableList(ChildNode, DisableList);
	}
}

void UUIPrimitiveSelectComponent::OnPointerUp(UPointerEventData* EventData)
{
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	Super::OnPointerUp(EventData);

	const FVector2D CurMouseScreenPos = FVector2D(EventData->Position);
	if (!LastMouseScreenPos.Equals(CurMouseScreenPos))
	{
		LastMouseScreenPos = CurMouseScreenPos;
		ClickedDesignerRects.Reset();
	}

	if (DragMode != EUISelectDragMode::None && DragMode != EUISelectDragMode::CenterBox)
	{
		PressCursor = EMouseCursor::None;
		return;
	}

	if (bHasDragged)
		return;

	if (!ViewportClient.IsValid())
		return;

	const bool bIsCtrlDown = FSlateApplication::Get().GetModifierKeys().IsControlDown();
	
	if (!IsValid(EventData) || EventData->Button != EPointerInputButton::InputButton_Left)
		return;

	UpdateDesignerRects();

	FVector WorldRayOrigin;
	FVector WorldRayDirection;
	if (!GetWorldRay(FVector2D(EventData->Position), WorldRayOrigin, WorldRayDirection))
	{
		return;
	}

	URectTransformComponent* FirstSelectedRectTransform = nullptr;

	const AActor* PreviewActor = ViewportClient.Pin()->GetPreviewActor();

	SelectedCompInstance = nullptr;
	for (int32 Index = DesignerRects.Num() - 1; Index >= 0; --Index)
	{
		auto DesignerRect = DesignerRects[Index];
		
		if (IsValid(DesignerRect) && FRectTransformUtility::RectangleIntersectRay(DesignerRect, WorldRayOrigin, WorldRayDirection, false))
		{
			const AActor* DesignerRectActor = DesignerRect->GetOwner();

			while(DesignerRectActor != PreviewActor && IsValid(DesignerRectActor)
				&& IsValid(DesignerRectActor->GetRootComponent()))
			{
				DesignerRect = Cast<URectTransformComponent>(DesignerRectActor->GetRootComponent()->GetAttachParent());

				if (IsValid(DesignerRect))
				{
					DesignerRectActor = DesignerRect->GetOwner();
				}
				else
				{
					break;
				}
			}

			if (!IsValid(DesignerRect))
			{
				continue;
			}
			
			if (DesignerRect->bIsLockForEditor && GetDefault<UUIEditorPerProjectUserSettings>()->bRespectLock)
			{
				continue;
			}
			
			if (FirstSelectedRectTransform == nullptr)
			{
				FirstSelectedRectTransform = DesignerRect;
			}

			if (bIsCtrlDown)
			{
				SelectedCompInstance = DesignerRect;
				break;
			}
			
			if (ClickedDesignerRects.Contains(DesignerRect))
			{
				continue;
			}
			
			SelectedCompInstance = DesignerRect;
			ClickedDesignerRects.Add(DesignerRect);
			break;
		}
	}

	if (!bIsCtrlDown && SelectedCompInstance == nullptr)
	{
		if (IsValid(FirstSelectedRectTransform))
		{
			ClickedDesignerRects.Reset();
			ClickedDesignerRects.Add(FirstSelectedRectTransform);
		}
		SelectedCompInstance = FirstSelectedRectTransform;
	} 
	
	const auto BlueprintEditorPtr = ViewportClient.Pin()->GetBlueprintEditor();
	if (BlueprintEditorPtr.IsValid())
	{
		if (IsValid(SelectedCompInstance))
		{
			BlueprintEditorPtr.Pin()->FindAndSelectSCSEditorTreeNode(SelectedCompInstance, bIsCtrlDown);
		}
		else if (!bIsCtrlDown)
		{
			BlueprintEditorPtr.Pin()->ClearSelectionStateFor(TEXT("Components"));
		}
	}
}

void UUIPrimitiveSelectComponent::UpdateDesignerRects()
{
	if (!ViewportClient.IsValid())
	{
		return;
	}
	
	if (!ViewportClient.Pin()->bUpdateWidgetActorComponents)
	{
		if (DesignerRects.Num() > 0 && IsValid(DesignerRects[0]))
		{
			return;
		}
	}

	ViewportClient.Pin()->bUpdateWidgetActorComponents = false;
	DesignerRects.Reset();

	const AActor* WidgetActor = ViewportClient.Pin()->GetPreviewActor();
	if (IsValid(WidgetActor))
	{
		GetRectTransforms(WidgetActor->GetRootComponent());
	}
}

void UUIPrimitiveSelectComponent::UpdateCheckBox(const bool bInDraw)
{
	if (bInDraw)
	{
		CheckBoxDrawComponent->SetHidePrimitive(false);
		CheckBoxDrawComponent->SetAllDirty();

		FVector2D PivotPosition;
		FVector2D TopLeftPosition;
		FVector2D BottomRightPosition;
		FVector2D Size;

		const FRectBox CheckBox = GetCheckRect(CheckRectTopLeft, CheckRectBottomRight, true);

		ScreenPointToLocalPoint(this, CheckBox.Center, PivotPosition);
		ScreenPointToLocalPoint(this, CheckBox.Center - CheckBox.Size, TopLeftPosition);
		ScreenPointToLocalPoint(this, CheckBox.Center + CheckBox.Size, BottomRightPosition);

		Size.X = BottomRightPosition.X - TopLeftPosition.X;
		Size.Y = TopLeftPosition.Y - BottomRightPosition.Y;

		CheckBoxDrawComponent->SetAnchoredPosition(PivotPosition);
		CheckBoxDrawComponent->SetSizeDelta(Size);
	}
	else
	{
		CheckBoxDrawComponent->SetHidePrimitive(true);
		CheckBoxDrawComponent->SetAllDirty();
	}
}

void UUIPrimitiveSelectComponent::GetRectTransforms(USceneComponent* SceneComp)
{
	if (!IsValid(SceneComp))
		return;
	
	URectTransformComponent* RectComp = Cast<URectTransformComponent>(SceneComp);
	if (IsValid(RectComp))
	{
		DesignerRects.Add(RectComp);
	}
	
	for (const auto& ChildComp : SceneComp->GetAttachChildren())
	{
		GetRectTransforms(ChildComp);
	}
}

FRectBox UUIPrimitiveSelectComponent::GetCheckRect(const FVector2D& Point1,const FVector2D& Point2,bool bIsScreenSpace) const
{
	FVector2D ScreenTopLeft(FMath::Min(Point1.X, Point2.X),FMath::Min(Point1.Y,Point2.Y));
	FVector2D ScreenBottomRight(FMath::Max(Point1.X, Point2.X), FMath::Max(Point1.Y, Point2.Y));

	if(!bIsScreenSpace)
	{
		ScreenTopLeft *= ViewportClient.Pin()->EditorViewportSize;
		ScreenBottomRight *= ViewportClient.Pin()->EditorViewportSize;
	}

	 FRectBox RectBox;
	 RectBox.SetMinMax(ScreenTopLeft, ScreenBottomRight);

	return RectBox;
}

FRectBox UUIPrimitiveSelectComponent::ScreenBoxToLocal(const FRectBox& RaycastRectTemp)
{
	FVector2D PivotPosition;
	FVector2D TopLeftPosition;
	FVector2D BottomRightPosition;
	FVector2D Size;

	ScreenPointToLocalPoint(this, RaycastRectTemp.Center, PivotPosition);
	ScreenPointToLocalPoint(this, RaycastRectTemp.Center - RaycastRectTemp.Size, TopLeftPosition);
	ScreenPointToLocalPoint(this, RaycastRectTemp.Center + RaycastRectTemp.Size, BottomRightPosition);

	Size.X = BottomRightPosition.X - TopLeftPosition.X;
	Size.Y = TopLeftPosition.Y - BottomRightPosition.Y;

	FRectBox RaycastRect;
	RaycastRect.Center = PivotPosition;
	RaycastRect.Size = Size;

	return RaycastRect;
}

bool UUIPrimitiveSelectComponent::ScreenPointToLocalPointInRectangle(
	const URectTransformComponent* RectTransform, UCanvasSubComponent* InCanvas, const FVector2D& ScreenPosition, FVector2D& LocalPosition, bool bGetFromViewport)
{
	if (!IsValid(RectTransform))
		return false;

	FVector WorldRayOrigin;
	FVector WorldRayDir;

	if (bGetFromViewport)
	{
		if (!GetWorldRay(ScreenPosition, WorldRayOrigin, WorldRayDir))
		{
			return false;
		}
	}
	else if (!GetWorldRayFromCanvas(InCanvas, ScreenPosition, WorldRayOrigin, WorldRayDir))
	{
		return false;
	}

	const auto WorldToLocal = RectTransform->GetComponentTransform().Inverse();
	const auto LocalSpaceRayOrigin = WorldToLocal.TransformPosition(WorldRayOrigin);
	auto LocalSpaceRayDir = WorldToLocal.TransformVector(WorldRayDir);
	LocalSpaceRayDir.Normalize();

	const FVector PlaneNormal = FVector(0, 0, 1);
	const FVector PlaneOrigin = FVector::ZeroVector;

	const float A = FVector::DotProduct(LocalSpaceRayDir, PlaneNormal);
	if (FMathUtility::Approximately(A, 0.0f))
	{
		return false;
	}

	const float Distance = FVector::DotProduct((PlaneOrigin - LocalSpaceRayOrigin), PlaneNormal) / A;
	LocalPosition = FVector2D(LocalSpaceRayOrigin + LocalSpaceRayDir * Distance);
	return true;
}

bool UUIPrimitiveSelectComponent::GetWorldRay(const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection)
{
	//Get screen position, convert to range 0-1
	const FVector2D ViewportSize = UUGUISubsystem::GetViewportSize(this);
	FVector2D ViewPoint01 = ScreenPosition / ViewportSize;

	if (!InternalGetWorldRay(ViewPoint01, WorldRayOrigin, WorldRayDirection))
	{
		return false;
	}

	return true;
}

bool UUIPrimitiveSelectComponent::InternalGetWorldRay(FVector2D& ViewPoint01, FVector& OutRayOrigin, FVector& OutRayDirection) const
{
	if (!ViewportClient.IsValid())
	{
		return false;
	}
	
    ViewPoint01.Y = 1.0f - ViewPoint01.Y;

    const FMatrix& InvViewProjMatrix = ViewportClient.Pin()->ViewProjectionMatrix.InverseFast();

    const float ScreenSpaceX = (ViewPoint01.X - 0.5f) * 2.0f;
    const float ScreenSpaceY = (ViewPoint01.Y - 0.5f) * 2.0f;

    // The start of the raytrace is defined to be at MouseX, MouseY, 1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
    // To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (MouseX, MouseY, 0.5)
    const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

    // Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
    // by the projection matrix should use homogeneous coordinates (i.e. FPlane).
    const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
    const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);

    FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
    FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);

    // divide vectors by W to undo any projection and get the 3-space coordinate 
    if (HGRayStartWorldSpace.W != 0.0f)
    {
        RayStartWorldSpace /= HGRayStartWorldSpace.W;
    }

    if (HGRayEndWorldSpace.W != 0.0f)
    {
        RayEndWorldSpace /= HGRayEndWorldSpace.W;
    }

    // Finally, store the results in the outputs
    OutRayOrigin = RayStartWorldSpace;
    OutRayDirection = (RayEndWorldSpace - RayStartWorldSpace).GetSafeNormal();
    return true;
}

bool UUIPrimitiveSelectComponent::GetWorldRayFromCanvas(UCanvasSubComponent* InCanvas, const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection)
{
	if (!IsValid(InCanvas))
		return false;

	//Get screen position, convert to range 0-1
	const FVector2D ViewportSize = UUGUISubsystem::GetViewportSize(InCanvas);
	FVector2D ViewPoint01 = ScreenPosition / ViewportSize;

	const auto RootCanvas = InCanvas->GetRootCanvas();
	check(RootCanvas);

	if (!InternalGetWorldRayFromCanvas(ViewPoint01, RootCanvas, WorldRayOrigin, WorldRayDirection))
	{
		return false;
	}

	return true;
}

bool UUIPrimitiveSelectComponent::InternalGetWorldRayFromCanvas(FVector2D& ViewPoint01, UCanvasSubComponent* CanvasComp,
	FVector& OutRayOrigin, FVector& OutRayDirection)
{
	if (!IsValid(CanvasComp))
        return false;

    FVector ViewLocation;
    FMatrix ViewRotationMatrix;
    FMatrix ProjectionMatrix;
    FMatrix ViewProjectionMatrix;
    FMatrix LocalToWorldMatrix;
    CanvasComp->CalculateCanvasMatrices(ViewLocation, ViewRotationMatrix, ProjectionMatrix, ViewProjectionMatrix, LocalToWorldMatrix);

    ViewPoint01.Y = 1.0f - ViewPoint01.Y;

    const FMatrix& InvViewProjMatrix = ViewProjectionMatrix.InverseFast();

    const float ScreenSpaceX = (ViewPoint01.X - 0.5f) * 2.0f;
    const float ScreenSpaceY = (ViewPoint01.Y - 0.5f) * 2.0f;

    // The start of the raytrace is defined to be at MouseX, MouseY, 1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
    // To get the direction of the raytrace we need to use any z between the near and the far plane, so let's use (MouseX, MouseY, 0.5)
    const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
    const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.5f, 1.0f);

    // Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
    // by the projection matrix should use homogeneous coordinates (i.e. FPlane).
    const FVector4 HGRayStartWorldSpace = InvViewProjMatrix.TransformFVector4(RayStartProjectionSpace);
    const FVector4 HGRayEndWorldSpace = InvViewProjMatrix.TransformFVector4(RayEndProjectionSpace);

    FVector RayStartWorldSpace(HGRayStartWorldSpace.X, HGRayStartWorldSpace.Y, HGRayStartWorldSpace.Z);
    FVector RayEndWorldSpace(HGRayEndWorldSpace.X, HGRayEndWorldSpace.Y, HGRayEndWorldSpace.Z);

    // divide vectors by W to undo any projection and get the 3-space coordinate 
    if (HGRayStartWorldSpace.W != 0.0f)
    {
        RayStartWorldSpace /= HGRayStartWorldSpace.W;
    }

    if (HGRayEndWorldSpace.W != 0.0f)
    {
        RayEndWorldSpace /= HGRayEndWorldSpace.W;
    }

    // Finally, store the results in the outputs
    OutRayOrigin = RayStartWorldSpace;
    OutRayDirection = (RayEndWorldSpace - RayStartWorldSpace).GetSafeNormal();

    return true;
}

bool UUIPrimitiveSelectComponent::ScreenPointToLocalPoint(URectTransformComponent* RectTransform,
                                                          const FVector2D& ScreenPosition, FVector2D& LocalPosition, UCanvasSubComponent* InCanvas)
{
	if (!IsValid(RectTransform))
		return false;

	if (!IsValid(InCanvas))
	{
		InCanvas = RectTransform->GetOwnerCanvas();
	}

	if (!IsValid(InCanvas))
		return false;

	return ScreenPointToLocalPointInRectangle(RectTransform, InCanvas, ScreenPosition, LocalPosition);
}

/////////////////////////////////////////////////////
