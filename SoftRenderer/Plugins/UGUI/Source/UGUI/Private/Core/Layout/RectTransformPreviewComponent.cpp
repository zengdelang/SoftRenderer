#include "Core/Layout/RectTransformPreviewComponent.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Engine/CollisionProfile.h"
#include "UGUISubsystem.h"

/////////////////////////////////////////////////////
// URectTransformPreviewComponent

class FRectTransformPreviewSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	/**
	* Initialization constructor.
	* @param	InComponent - game component to draw in the scene
	*/
	FRectTransformPreviewSceneProxy(const URectTransformPreviewComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
	{
		bWillEverBeLit = false;
		bIsRootCanvas = InComponent->bIsRootCanvas;

		XMin = InComponent->XMin;
		YMin = InComponent->YMin;
		
		XMax = InComponent->XMax;
		YMax = InComponent->YMax;

		AnchorMin = InComponent->AnchorMin;
		AnchorMax = InComponent->AnchorMax;
		
		SizeDelta = InComponent->SizeDelta;
		
		bDraw = true;

		const URectTransformComponent* ParentTransform = Cast<URectTransformComponent>(InComponent->GetAttachParent());
		if (IsValid(ParentTransform))
		{
			const auto RectParentComp = ParentTransform->GetAttachParent();
			if (IsValid(RectParentComp))
			{
				ParentRect = ParentTransform->GetParentRect();
				ParentLocalToWorldTransform = RectParentComp->GetComponentTransform();
			}
		}

#if WITH_EDITOR
		const FWorldViewportInfo* EditorViewportInfoPtr = UUGUISubsystem::GetWorldViewportInfo(InComponent);
		if (EditorViewportInfoPtr)
		{
			bDraw = EditorViewportInfoPtr->GetRenderMode() == ECanvasRenderMode::CanvasRenderMode_WorldSpace;
		}
#endif

		bool bNotifyDelegate = false;

		if (!InComponent || !InComponent->GetAttachParent())
		{
			return;
		}
		
		if (InComponent->GetWorld() && InComponent->GetWorld()->WorldType != EWorldType::EditorPreview)
		{
			return;
		}
		
		bMultiSelected = false;
		bSelfSelected = false;
		bSiblingSelected = false;
		bIsRootCanvas = false;
		bIsParentSelected = false;
		
		UObject* TemplateComponent = InComponent->GetAttachParent()->GetArchetype();
		
		FEditorUIDesignerInfo* UIDesignerInfo = URectTransformPreviewComponent::GetEditorUIDesignerInfo(InComponent->GetWorld());
		if (UIDesignerInfo)
		{
			bMultiSelected = UIDesignerInfo->bMultiSelected;
			bSelfSelected = UIDesignerInfo->CurSelectedRectComponent == TemplateComponent;
			bSiblingSelected = UIDesignerInfo->CurSelectedSiblingRectComponents.Contains(TemplateComponent);
			bIsRootCanvas = UIDesignerInfo->RootRectComponent == TemplateComponent;
			bIsParentSelected = UIDesignerInfo->ParentSelectedRectComponent == TemplateComponent;
			
			FEditorUIDesignerRect DesignerRect;
			DesignerRect.Component = const_cast<URectTransformPreviewComponent*>(InComponent);
			DesignerRect.LocalToWorld = InComponent->GetComponentTransform();
				
			DesignerRect.XMin = XMin;
			DesignerRect.YMin = YMin;
			DesignerRect.XMax = XMax;
			DesignerRect.YMax = YMax;

			DesignerRect.AnchorMin = AnchorMin;
			DesignerRect.AnchorMax = AnchorMax;

			DesignerRect.SizeDelta = SizeDelta;
			DesignerRect.ParentRect = ParentRect;
			DesignerRect.ParentLocalToWorld = ParentLocalToWorldTransform;
				
			DesignerRect.bMultiSelected = bMultiSelected;
			DesignerRect.bSelfSelected = bSelfSelected;
			DesignerRect.bSiblingSelected = bSiblingSelected;
			DesignerRect.bIsRootCanvas = bIsRootCanvas;
			DesignerRect.bIsParentSelected = bIsParentSelected;
			
			if (bIsRootCanvas)
			{
				UIDesignerInfo->RootRect = DesignerRect;
				bNotifyDelegate = true;
			}

			if (bIsParentSelected)
			{
				UIDesignerInfo->ParentRect = DesignerRect;
				bNotifyDelegate = true;
			}
			else if (bSelfSelected)
			{
				UIDesignerInfo->CurSelectedRect = DesignerRect;
				bNotifyDelegate = true;
			}
			else if (bSiblingSelected)
			{
				for (int32 Index = UIDesignerInfo->CurSelectedSiblingRects.Num() - 1; Index >= 0; --Index)
				{
					auto& SiblingRect = UIDesignerInfo->CurSelectedSiblingRects[Index];
					if (!SiblingRect.Component.IsValid())
					{
						UIDesignerInfo->CurSelectedSiblingRects.RemoveAt(Index, 1, false);
					}
					else if (SiblingRect.Component.Get() == InComponent)
					{
						UIDesignerInfo->CurSelectedSiblingRects.RemoveAt(Index, 1, false);
					}
				}
				
				UIDesignerInfo->CurSelectedSiblingRects.Add(DesignerRect);
				bNotifyDelegate = true;
			}

			if (SizeDelta.X < 0 || SizeDelta.Y < 0)
			{
				for (int32 Index = UIDesignerInfo->NegativeSizeRects.Num() - 1; Index >= 0; --Index)
				{
					auto& NegativeSizeRect = UIDesignerInfo->NegativeSizeRects[Index];
					if (!NegativeSizeRect.Component.IsValid())
					{
						UIDesignerInfo->NegativeSizeRects.RemoveAt(Index, 1, false);
					}
					else if (NegativeSizeRect.Component.Get() == InComponent)
					{
						UIDesignerInfo->NegativeSizeRects.RemoveAt(Index, 1, false);
					}
				}
				
				UIDesignerInfo->NegativeSizeRects.Add(DesignerRect);
				bNotifyDelegate = true;
			}
		}

		if (bNotifyDelegate)
		{
			URectTransformPreviewComponent::OnEditorUIDesignerInfoChanged.Broadcast(InComponent->GetWorld());
		}
	}

	// FPrimitiveSceneProxy interface.

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_RectTransformPreviewSceneProxy_DrawDynamicElements);

		if (!bDraw)
			return;
		
		FVector Verts[4]; 
		// Left-Bottom
		Verts[0] = FVector(XMin, YMin, 0);
		// Left-Top
		Verts[1] = FVector(XMin, YMax, 0);
		// Right-Top
		Verts[2] = FVector(XMax, YMax, 0);
		// Right-Bottom
		Verts[3] = FVector(XMax, YMin, 0);

		for (int32 X = 0; X < 4; ++X)
		{
			Verts[X] = GetLocalToWorld().TransformPosition(Verts[X]);
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ++ViewIndex)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];

				if (bIsRootCanvas)
				{
					PDI->DrawLine(Verts[0], Verts[1], FLinearColor(0.3, 0.3, 0.3), SDPG_Foreground, 0, 0, true);
					PDI->DrawLine(Verts[1], Verts[2], FLinearColor(0.3, 0.3, 0.3), SDPG_Foreground, 0, 0, true);
					PDI->DrawLine(Verts[2], Verts[3], FLinearColor(0.3, 0.3, 0.3), SDPG_Foreground, 0, 0, true);
					PDI->DrawLine(Verts[3], Verts[0], FLinearColor(0.3, 0.3, 0.3), SDPG_Foreground, 0, 0, true);
				}
			
				if (SizeDelta.X < 0 || SizeDelta.Y < 0)
				{
					PDI->DrawLine(Verts[0], Verts[2], FLinearColor::Red, SDPG_Foreground, 0, 0, true);
					PDI->DrawLine(Verts[1], Verts[3], FLinearColor::Red, SDPG_Foreground, 0, 0, true);
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = true;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:
	float XMin;
	float YMin;
	
	float XMax;
	float YMax;

	FVector2D AnchorMin;
	FVector2D AnchorMax;

	FVector2D SizeDelta;
	
	uint8 bMultiSelected : 1;
	uint8 bSelfSelected : 1;
	uint8 bSiblingSelected : 1;
	uint8 bIsRootCanvas: 1;
	uint8 bIsParentSelected : 1;
	uint8 bDraw : 1;

	FRect ParentRect;

	FTransform ParentLocalToWorldTransform;
};

FOnEditorUIDesignerInfoChanged URectTransformPreviewComponent::OnEditorUIDesignerInfoChanged;

URectTransformPreviewComponent::URectTransformPreviewComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseEditorCompositing = true;
	bHiddenInGame = true;
	
	UPrimitiveComponent::SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetGenerateOverlapEvents(false);

	XMin = 0;
	YMin = 0;

	XMax = 0;
	YMax = 0;

	bIsRootCanvas = false;
}

FBoxSphereBounds URectTransformPreviewComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	FBox LocalBox(ForceInit);
	LocalBox += FVector(XMin, YMin, 0);
	LocalBox += FVector(XMin, YMax, 0);
	LocalBox += FVector(XMax, YMax, 0);
	LocalBox += FVector(XMax, YMin, 0);
	
	const FBoxSphereBounds LocalBounds = LocalBox.IsValid ? FBoxSphereBounds(LocalBox) :
			FBoxSphereBounds(FVector(0, 0, 0), FVector(0, 0, 0), 0); // fallback to reset box sphere bounds
	FBoxSphereBounds Ret(LocalBounds.TransformBy(LocalToWorld));
	Ret.BoxExtent *= BoundsScale;
	Ret.SphereRadius *= BoundsScale;
	return Ret;
}

FPrimitiveSceneProxy* URectTransformPreviewComponent::CreateSceneProxy()
{
	return new FRectTransformPreviewSceneProxy(this);
}

FEditorUIDesignerInfo* URectTransformPreviewComponent::GetEditorUIDesignerInfo(UWorld* InWorld)
{
	if (!IsValid(InWorld))
		return nullptr;
	
	static TMap<TWeakObjectPtr<UWorld>, FEditorUIDesignerInfo> EditorUIDesignerInfoMap;

	FEditorUIDesignerInfo* UIDesignInfoPtr = EditorUIDesignerInfoMap.Find(InWorld);
	if (!UIDesignInfoPtr)
	{
		UIDesignInfoPtr = &EditorUIDesignerInfoMap.Emplace(InWorld, FEditorUIDesignerInfo());
	}
	return UIDesignInfoPtr;
}

void URectTransformPreviewComponent::ClearEditorUIDesignerInfo(UWorld* InWorld)
{
	FEditorUIDesignerInfo* DesignerInfo = GetEditorUIDesignerInfo(InWorld);
	if (DesignerInfo)
	{
		DesignerInfo->Clear();
		URectTransformPreviewComponent::OnEditorUIDesignerInfoChanged.Broadcast(InWorld);
	}
}

/////////////////////////////////////////////////////
