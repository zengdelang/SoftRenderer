#pragma once

#include "CoreMinimal.h"
#include "UISelectedRectDrawerComponent.h"
#include "Core/Widgets/SelectableComponent.h"
#include "EventSystem/Interfaces/BeginDragHandlerInterface.h"
#include "EventSystem/Interfaces/DragHandlerInterface.h"
#include "Core/Widgets/ImageComponent.h"
#include "SSCSComponentEditor.h"
#include "EventSystem/Interfaces/EndDragHandlerInterface.h"
#include "UIPrimitiveSelectComponent.generated.h"

struct FRectBox
{
	FVector2D Center;
	FVector2D Size;
	
public:
	FRectBox()
	{
		Center = FVector2D::ZeroVector;
		Size = FVector2D(-10, -10);
	}
	
	FRectBox(const FVector2D& InCenter, const FVector2D& InSize)
	{
		Center = InCenter;
		Size = InSize;
	}

public:
	void SetMinMax(const FVector2D& Min, const FVector2D& Max)
	{
		Center = (Min + Max) * 0.5;
		Size = (Max - Min) * 0.5;
	}

	void SetCenterSize(const FVector2D& InCenter, const FVector2D& InSize)
	{
		Center = InCenter;
		Size = InSize;
	}

	float GetXMax() const
	{
		return Center.X + Size.X;
	}

	float GetXMin() const
	{
		return Center.X - Size.X;
	}

	float GetYMax() const
	{
		return Center.Y + Size.Y;
	}

	float GetYMin() const
	{
		return Center.Y - Size.Y;
	}
	
	bool Contains(const FVector2D& Point) const
	{
		const FVector2D Delta = Center - Point;
		return FMath::Abs(Delta.X) <= Size.X && FMath::Abs(Delta.Y) <= Size.Y;
	}

	bool Contains(const FRectBox& Other) const
	{
		return this->GetXMin() < Other.GetXMin()&& this->GetYMin() < Other.GetYMin()&& this->GetXMax() > Other.GetXMax() && this->GetYMax() > Other.GetYMax();
	}
	
};

enum class EUISelectDragMode : uint8
{
	None,
	CenterBox,
	LeftEdge,
	RightEdge,
	TopEdge,
	BottomEdge,
	TopLeftPoint,
	TopRightPoint,
	BottomLeftPoint,
	BottomRightPoint,
};

UCLASS(Blueprintable, BlueprintType)
class UIBLUEPRINTEDITOR_API UUIPrimitiveSelectComponent : public USelectableComponent, public IBeginDragHandlerInterface,
	public IDragHandlerInterface, public IEndDragHandlerInterface
{
	GENERATED_UCLASS_BODY()

public:
	TWeakPtr<class FSCSUIEditorViewportClient> ViewportClient;

	UPROPERTY(Transient)
	UUISelectedRectDrawerComponent* SelectedRectDrawerComponent;

	UPROPERTY(Transient)
    UImageComponent* CheckBoxDrawComponent;

protected:
	UPROPERTY(Transient)
	TArray<URectTransformComponent*> DesignerRects;

	UPROPERTY(Transient)
	TArray<URectTransformComponent*> ClickedDesignerRects;

	UPROPERTY(Transient)
    TArray<URectTransformComponent*> SelectedDesignerRects;

	UPROPERTY(Transient)
	URectTransformComponent* SelectedCompInstance;
	
	FVector2D LastMouseScreenPos;
	bool bHasDragged;

	FVector2D SelectedRectTopLeft;
	FVector2D SelectedRectBottomRight;
	bool bHasValidSelectedRect;

	FVector2D CheckRectTopLeft;
	FVector2D CheckRectBottomRight;
	bool bHasValidCheckRect;
	
protected:
	FRectBox CenterDragBox;

	FRectBox LeftEdgeBox;
	FRectBox RightEdgeBox;
	FRectBox TopEdgeBox;
	FRectBox BottomEdgeBox;

	FRectBox TopLeftPointBox;
	FRectBox TopRightPointBox;
	FRectBox BottomLeftPointBox;
	FRectBox BottomRightPointBox;
	
	FRectBox CheckRect;

	EUISelectDragMode DragMode;
	FVector2D LastDragScreenPos;
	
protected:
	UPROPERTY(Transient)
	USceneComponent* SelectedParentCompInstance;

	FVector LastPos;
	FVector LastLocalPos;

	EMouseCursor::Type PressCursor = EMouseCursor::None;
	
public:
	virtual void Awake() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	bool GetViewportPoint(const FVector& WorldPos, FVector2D& ViewportPos) const;
	
public:
	virtual void OnPointerDown(UPointerEventData* EventData) override;
	virtual void OnBeginDrag(UPointerEventData* EventData) override;
	virtual void OnDrag(UPointerEventData* EventData) override;
	virtual void OnEndDrag(UPointerEventData* EventData) override;
	virtual void OnPointerUp(UPointerEventData* EventData) override;
 
protected:
	void DragSelectedComponents(const FVector2D& CurScreenPos);
	void ResizeSelectedComponents(const FVector2D& CurScreenPos);
	void CheckSelectedComponents();
	void GetDisableList(FSCSComponentEditorTreeNodePtrType Node, TArray<FSCSComponentEditorTreeNodePtrType>* DisableList);
	
protected:
	void UpdateDesignerRects();
	void UpdateCheckBox(const bool bInDraw);

	void GetRectTransforms(USceneComponent* SceneComp);
	FRectBox GetCheckRect(const FVector2D& Point1,const FVector2D& Point2, bool bIsScreenSpace) const;
	FRectBox ScreenBoxToLocal(const FRectBox& RaycastRectTemp);

	bool ScreenPointToLocalPointInRectangle(const URectTransformComponent* RectTransform, UCanvasSubComponent* InCanvas, const FVector2D& ScreenPosition, FVector2D& LocalPosition, bool bGetFromViewport = false);
	bool GetWorldRay(const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection);
	bool InternalGetWorldRay(FVector2D& ViewPoint01, FVector& OutRayOrigin, FVector& OutRayDirection) const;

	static bool GetWorldRayFromCanvas(UCanvasSubComponent* InCanvas, const FVector2D& ScreenPosition, FVector& WorldRayOrigin, FVector& WorldRayDirection);
	static bool InternalGetWorldRayFromCanvas(FVector2D& ViewPoint01, UCanvasSubComponent* CanvasComp, FVector& OutRayOrigin, FVector& OutRayDirection);
	
public:
	bool ScreenPointToLocalPoint(URectTransformComponent* RectTransform, const FVector2D& ScreenPosition, FVector2D& LocalPosition, UCanvasSubComponent* InCanvas = nullptr);
	
};
