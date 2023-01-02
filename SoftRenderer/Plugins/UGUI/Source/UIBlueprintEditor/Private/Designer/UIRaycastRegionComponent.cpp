#include "Designer/UIRaycastRegionComponent.h"
#include "SCSUIEditorViewportClient.h"
#include "SSCSUIEditorViewport.h"
#include "UIEditorPerProjectUserSettings.h"

/////////////////////////////////////////////////////
// UUIRaycastRegionComponent

UUIRaycastRegionComponent::UUIRaycastRegionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
	PrimaryComponentTick.bHighPriority = true;
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;

	bRaycastTarget = false;
	
	RegionColor = FLinearColor(0, 1, 0, 0.5);
}

void UUIRaycastRegionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!ViewportClient.IsValid())
	{
		SetHidePrimitive(true);
		return;
	}
	else
	{
		const auto UIEditorViewport = ViewportClient.Pin()->UIEditorViewport;
		if (UIEditorViewport.IsValid())
		{
			if (UIEditorViewport.Pin()->IsSimulateGameOverlay())
			{
				SetHidePrimitive(true);
			}
			else
			{
				SetHidePrimitive(false);
			}
		}
	}
	
	const int32 OldNum = RaycastRegionBoxList.Num();
	RaycastRegionBoxList.Reset();

	bool bUpdate = false;
	
	if (GetDefault<UUIEditorPerProjectUserSettings>()->bShowRaycastRegion)
	{
		RaycastRegionComponents.Reset();
		
		const AActor* WidgetActor = ViewportClient.Pin()->GetPreviewActor();
		if (IsValid(WidgetActor))
		{
			GetGraphicElements(WidgetActor->GetRootComponent());
		}

		const FTransform WorldToLocal = GetComponentTransform().Inverse();
		for (const auto& Comp : RaycastRegionComponents)
		{
			FVector WorldCorners[4];
			Comp->GetWorldCorners(WorldCorners);
			
			bUpdate = true;
			
			FRaycastRegionBox RaycastRegionBox;
			RaycastRegionBox.BottomLeft = FVector2D(WorldToLocal.TransformPosition(WorldCorners[0]));
			RaycastRegionBox.TopLeft = FVector2D(WorldToLocal.TransformPosition(WorldCorners[1]));
			RaycastRegionBox.TopRight = FVector2D(WorldToLocal.TransformPosition(WorldCorners[2]));
			RaycastRegionBox.BottomRight = FVector2D(WorldToLocal.TransformPosition(WorldCorners[3]));

			RaycastRegionBoxList.Emplace(RaycastRegionBox);
		}
	}

	const int32 NewNum = RaycastRegionBoxList.Num();

	if (bUpdate || OldNum != NewNum)
	{
		SetVerticesDirty();
	}
}

void UUIRaycastRegionComponent::OnPopulateMesh(FVertexHelper& VertexHelper)
{
	VertexHelper.Reset();

	const int32 BoxCount = RaycastRegionBoxList.Num();
	VertexHelper.Reserve(4 * BoxCount, 6 * BoxCount);
	
	for (int32 Index = 0; Index < BoxCount; ++Index)
	{
		const auto& RaycastRegionBox = RaycastRegionBoxList[Index];
		VertexHelper.AddVert(FVector(RaycastRegionBox.BottomLeft.X, RaycastRegionBox.BottomLeft.Y, 0), RegionColor, FVector2D(0, 1));
		VertexHelper.AddVert(FVector(RaycastRegionBox.TopLeft.X, RaycastRegionBox.TopLeft.Y, 0), RegionColor, FVector2D(0, 0));
		VertexHelper.AddVert(FVector(RaycastRegionBox.TopRight.X, RaycastRegionBox.TopRight.Y, 0), RegionColor, FVector2D(1, 0));
		VertexHelper.AddVert(FVector(RaycastRegionBox.BottomRight.X, RaycastRegionBox.BottomRight.Y, 0), RegionColor, FVector2D(1, 1));

		const int32 IndexDelta = 4 * Index;
		VertexHelper.AddTriangle(0 + IndexDelta, 1 + IndexDelta, 2 + IndexDelta);
		VertexHelper.AddTriangle(2 + IndexDelta, 3 + IndexDelta, 0 + IndexDelta);
	}
}

void UUIRaycastRegionComponent::GetGraphicElements(USceneComponent* SceneComp)
{
	if (!IsValid(SceneComp))
		return;
	
	URectTransformComponent* RectTransformComponent = Cast<URectTransformComponent>(SceneComp);
	if (IsValid(RectTransformComponent))
	{
		const TScriptInterface<IGraphicElementInterface> Graphic = RectTransformComponent->GetComponentByInterface(UGraphicElementInterface::StaticClass(), true);
		if (Graphic && Graphic->IsRaycastTarget())
		{
			RaycastRegionComponents.Add(RectTransformComponent);

#if WITH_EDITORONLY_DATA
			bool bCheckColor = false;
			UBehaviourComponent* TemplateComp = nullptr;
			const AActor* WidgetActor = ViewportClient.Pin()->GetPreviewActor();
			if (IsValid(WidgetActor) && WidgetActor != SceneComp->GetOwner())
			{
				auto ParentComp = RectTransformComponent->GetAttachParent();
				while (ParentComp != nullptr)
				{
					if (WidgetActor == ParentComp->GetOwner())
					{
						bCheckColor = true;
						TemplateComp = Cast<UBehaviourComponent>(ParentComp->GetArchetype());
						break;
					}
					ParentComp = ParentComp->GetAttachParent();
				}
			}
			else
			{
				TemplateComp = Cast<UBehaviourComponent>(RectTransformComponent->GetArchetype());
			}
			
			if (TemplateComp)
			{
				if (bCheckColor && TemplateComp->EditorRowColor != FLinearColor::Green)
				{
					TemplateComp->EditorRowColor = FLinearColor(0, 0.5, 0, 1);
				}
				else
				{
					TemplateComp->EditorRowColor = FLinearColor::Green;
				}
			}
#endif
		}
		else
		{
#if WITH_EDITORONLY_DATA
			UBehaviourComponent* TemplateComp = TemplateComp = Cast<UBehaviourComponent>(RectTransformComponent->GetArchetype());
			if (TemplateComp)
			{
				TemplateComp->EditorRowColor = FLinearColor::White;
			}
#endif
		}
	}
	
	for (const auto& ChildComp : SceneComp->GetAttachChildren())
	{
		GetGraphicElements(ChildComp);
	}
}

/////////////////////////////////////////////////////
