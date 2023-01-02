#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/ScrollbarComponent.h"
#include "Core/Widgets/SelectableComponent.h"
#include "Core/CanvasElementInterface.h"
#include "Core/Layout/LayoutElementInterface.h"
#include "Core/Layout/LayoutGroupInterface.h"
#include "Core/Render/LateUpdateInterface.h"
#include "EventSystem/Interfaces/EndDragHandlerInterface.h"
#include "EventSystem/Interfaces/ScrollHandlerInterface.h"
#include "ScrollRectComponent.generated.h"

USTRUCT(BlueprintType)
struct UGUI_API FBounds
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bounds)
	FVector Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bounds)
	FVector Extents;

public:
	FBounds()
	{
		Center = FVector::ZeroVector;
		Extents = FVector::ZeroVector;
	}

	FBounds(FVector InCenter, FVector InSize)
	{
		Center = InCenter;
		Extents = InSize * 0.5f;
	}

public:
	FORCEINLINE bool operator == (const FBounds& Other) const
	{
		return Center == Other.Center && Extents == Other.Extents;
	}

	FORCEINLINE bool operator != (const FBounds& Other) const
	{
		return !(*this == Other);
	}
	
	FORCEINLINE FVector GetSize() const
	{
		return Extents * 2;
	}

	FORCEINLINE void SetSize(FVector InSize)
	{
		Extents = InSize * 0.5f;
	}

	FORCEINLINE FVector GetMin() const
	{
		return Center - Extents;
	}

	FORCEINLINE FVector GetMax() const
	{
		return Center + Extents;
	}

	FORCEINLINE void SetMinMax(FVector Min, FVector Max)
	{
		Extents = (Max - Min) * 0.5f;
		Center = Min + Extents;
	}

	FORCEINLINE void Encapsulate(FVector Point)
	{
		const FVector& Min = GetMin();
		const FVector& Max = GetMax();	
		SetMinMax(FVector(FMath::Min(Min.X, Point.X), FMath::Min(Min.Y, Point.Y), FMath::Min(Min.Z, Point.Z)),
			FVector(FMath::Max(Max.X, Point.X), FMath::Max(Max.Y, Point.Y), FMath::Max(Max.Z, Point.Z)));
	}
};

enum EContentPosDirtyMode
{
	DirtyMode_None = 0,
	DirtyMode_Normal = 1 << 1,
	DirtyMode_UserDirty = 1 << 2,
	DirtyMode_LayoutDirty = 1 << 3,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollRectValueChangedEvent, const FVector2D&, InValue);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnScrollRectContentPosChangedEvent, int32, const FVector2D&, const FVector2D&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollRectBeginDragEvent, const FVector2D&, InValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollRectDragEvent, const FVector2D&, InValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrollRectEndDragEvent, const FVector2D&, InValue);

/**
 * A setting for which behavior to use when content moves beyond the confines of its container.
 */
UENUM(BlueprintType)
enum class EScrollRectMovementType : uint8
{
	/**
	 * Unrestricted movement. The content can move forever.
	 */
	MovementType_Unrestricted UMETA(DisplayName = "Unrestricted"),

	/**
	 * Elastic movement. The content is allowed to temporarily move beyond the container, but is pulled back elastically.
	 */
	MovementType_Elastic UMETA(DisplayName = "Elastic"),

	/**
	 * Clamped movement. The content can not be moved beyond its container.
	 */
	MovementType_Clamped UMETA(DisplayName = "Clamped"),
};

/**
 * Enum for which behavior to use for scrollbar visibility.
 */
UENUM(BlueprintType)
enum class EScrollRectScrollbarVisibility : uint8
{
	/**
	 * Always show the scrollbar.
	 */
	ScrollbarVisibility_Permanent UMETA(DisplayName = "Permanent"),

	/**
	 * Automatically hide the scrollbar when no scrolling is needed on this axis. The viewport rect will not be changed.
	 */
	ScrollbarVisibility_AutoHide UMETA(DisplayName = "AutoHide"),

	/**
	 * Automatically hide the scrollbar when no scrolling is needed on this axis, and expand the viewport rect accordingly.
	 *
	 * When this setting is used, the scrollbar and the viewport rect become driven, meaning that values in the RectTransform are calculated automatically and can't be manually edited.
	 */
	ScrollbarVisibility_AutoHideAndExpandViewport UMETA(DisplayName = "AutoHideAndExpandViewport"),
};

/**
 * A component for making a child RectTransform scroll.
 *
 * ScrollRect will not do any clipping on its own. Combined with a Mask component, it can be turned into a scroll view.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Interaction), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UScrollRectComponent : public USelectableComponent, public IScrollHandlerInterface, public IInitializePotentialDragHandlerInterface,
	public IBeginDragHandlerInterface, public IEndDragHandlerInterface,public IDragHandlerInterface, public ILayoutGroupInterface, public ILateUpdateInterface, public ICanvasElementInterface, public ILayoutElementInterface
{
	GENERATED_UCLASS_BODY()

public:
	/**
	 * The behavior to use when the content moves beyond the scroll rect.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	EScrollRectMovementType MovementType;

	/**
	 * The amount of elasticity to use when the content moves beyond the scroll rect.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	float Elasticity;

	/**
	 * The rate at which movement slows down.
	 *
	 * The deceleration rate is the speed reduction per second. A value of 0.5 halves the speed each second. The default is 0.135. The deceleration rate is only used when inertia is enabled.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	float DecelerationRate;

	/**
	 * The sensitivity to scroll wheel and track pad scroll events.
	 *
	 * Higher values indicate higher sensitivity.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	float ScrollSensitivity;

protected:
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	TArray<int32> ContentPath;

	UPROPERTY(EditAnywhere, Category = ScrollRect)
	TArray<int32> ViewportPath;

	UPROPERTY(EditAnywhere, Category = ScrollRect)
	TArray<int32> HorizontalScrollbarPath;

	UPROPERTY(EditAnywhere, Category = ScrollRect)
	TArray<int32> VerticalScrollbarPath;
	
public:
	/**
	 * The content that can be scrolled. It should be a child of the GameObject with ScrollRect on it.
	 */
	UPROPERTY(Transient)
	URectTransformComponent* Content;

	/**
	 * Reference to the viewport RectTransform that is the parent of the content RectTransform.
	 */
	UPROPERTY(Transient)
	URectTransformComponent* Viewport;

protected:
	/**
	 * Optional Scrollbar object linked to the horizontal scrolling of the ScrollRect.
	 */
	UPROPERTY(Transient)
	UScrollbarComponent* HorizontalScrollbar;

	/**
	 * Optional Scrollbar object linked to the vertical scrolling of the ScrollRect.
	 */
	UPROPERTY(Transient)
	UScrollbarComponent* VerticalScrollbar;

protected:
	/**
	 * The mode of visibility for the horizontal scrollbar.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	EScrollRectScrollbarVisibility HorizontalScrollbarVisibility;

	/**
	 * The mode of visibility for the vertical scrollbar.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	EScrollRectScrollbarVisibility VerticalScrollbarVisibility;

	/**
	 * The space between the scrollbar and the viewport.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	float HorizontalScrollbarSpacing;

	/**
	 * The space between the scrollbar and the viewport.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	float VerticalScrollbarSpacing;

public:
	/**
	 * Callback executed when the position of the child changes.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnScrollRectValueChangedEvent OnValueChanged;

	FOnScrollRectContentPosChangedEvent OnContentPosChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnScrollRectBeginDragEvent OnBeginDragEvent;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnScrollRectDragEvent OnDragEvent;

	UPROPERTY(BlueprintAssignable, Category = "Button|Event")
	FOnScrollRectEndDragEvent OnEndDragEvent;

protected:
	// The offset from handle position to mouse down position
	FVector2D PointerStartLocalCursor;
	FVector2D ContentStartPosition;

protected:
	UPROPERTY(Transient)
	URectTransformComponent* ViewRect;

protected:
	FBounds ContentBounds;
	FBounds ViewBounds;

public:
	FVector2D Velocity;

private:
	FVector2D PrevPosition;
	
	FBounds PrevContentBounds;
	FBounds PrevViewBounds;

private:
	UPROPERTY(Transient)
	URectTransformComponent* HorizontalScrollbarRect;

	UPROPERTY(Transient)
	URectTransformComponent* VerticalScrollbarRect;

	float HSliderHeight;
	float VSliderWidth;

	FVector2D OldContentPosition;
	
	TFrameValue<bool> bLateUpdated;
	int32 ContentPosDirtyMode;
	
public:
	/**
	 * Should horizontal scrolling be enabled?
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	uint8 bHorizontal : 1;

	/**
	 * Should vertical scrolling be enabled?
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	uint8 bVertical : 1;

	/**
	 * Should movement inertia be enabled?
	 *
	 * Inertia means that the scroll rect content will keep scrolling for a while after being dragged. It gradually slows down according to the decelerationRate.
	 */
	UPROPERTY(EditAnywhere, Category = ScrollRect)
	uint8 bInertia : 1;

private:
	uint8 bDragging : 1;
	uint8 bScrolling : 1;

	uint8 bHasRebuiltLayout : 1;
	
	uint8 bHSliderExpand : 1;
	uint8 bVSliderExpand : 1;

public:
	FVector2D GetContentStartPosition() const { return ContentStartPosition; };
	void SetContentStartPosition(FVector2D InNewPos) { ContentStartPosition = InNewPos; };

	FVector2D GetPrevPosition() const { return PrevPosition; };
	void SetPrevPosition(FVector2D InNewPos) { PrevPosition = InNewPos; };

	bool IsDragging() const { return bDragging; }

	void MarkContentDirtyByUser() { SetContentPosDirtyMode(EContentPosDirtyMode::DirtyMode_UserDirty); }

protected:
	void SetContentPosDirtyMode(EContentPosDirtyMode DirtyMode);
	
public:
	//~ Begin ICanvasElementInterface Interface
	virtual const USceneComponent* GetTransform() const override { return this; };
	virtual void Rebuild(ECanvasUpdate Executing) override;
	virtual void LayoutComplete() override {};
	virtual void GraphicUpdateComplete() override {};
	virtual bool IsDestroyed() override { return !IsValid(this); };
	virtual FString ToString() override { return GetFName().ToString(); };
	//~ End ICanvasElementInterface Interface

private:
	void UpdateCachedData();
	
public:
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnRectTransformDimensionsChange() override;
	//~ End BehaviourComponent Interface.

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	bool IsScrollRectActive() const;

private:
	void EnsureLayoutHasRebuilt() const;

public:
	virtual void InverseVelocity()
	{
		Velocity = -Velocity;
	}
	
	virtual void OnScroll(UPointerEventData* EventData) override;
	virtual void OnInitializePotentialDrag(UPointerEventData* EventData) override;

	/**
	 * Handling for when the content is beging being dragged.
	 */
	virtual void OnBeginDrag(UPointerEventData* EventData) override;

	/**
	 * Handling for when the content has finished being dragged.
	 */
	virtual void OnEndDrag(UPointerEventData* EventData) override;

	/**
	 * Handling for when the content is dragged.
	 */
	virtual void OnDrag(UPointerEventData* EventData) override;

protected:
	/**
	 * Sets the anchored position of the content.
	 */
	virtual void SetContentAnchoredPosition(FVector2D InPosition);

public:
	//~ Begin ILateUpdateInterface Interface
	virtual void LateUpdate() override;
	//~ End ILateUpdateInterface Interface.
	
protected:
	/**
	 * Helper function to update the previous data fields on a ScrollRect. Call this before you change data in the ScrollRect.
	 */
	void UpdatePrevData();

private:
	void UpdateScrollbars(const FVector2D& InOffset);

public:
	/**
	 * The scroll position as a Vector2 between (0,0) and (1,1) with (0,0) being the lower left corner.
	 */
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	FVector2D GetNormalizedPosition();

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetNormalizedPosition(FVector2D InNormalizedPosition);

	/**
	 * The horizontal scroll position as a value between 0 and 1, with 0 being at the left.
	 */
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	float GetHorizontalNormalizedPosition();

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetHorizontalNormalizedPosition(float InValue);

	/**
	 * The vertical scroll position as a value between 0 and 1, with 0 being at the bottom.
	 */
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	float GetVerticalNormalizedPosition();

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetVerticalNormalizedPosition(float InValue);

private:
	UFUNCTION()
	void InternalSetHorizontalNormalizedPosition(float InValue);

	UFUNCTION()
	void InternalSetVerticalNormalizedPosition(float InValue);

protected:
	/**
	 * Set the horizontal or vertical scroll position as a value between 0 and 1, with 0 being at the left or at the bottom.
	 *
	 * @param  InValue  The position to set, between 0 and 1.
	 * @param  InAxis  The axis to set: 0 for horizontal, 1 for vertical.
	 */
	virtual void InternalSetNormalizedPosition(float InValue, int32 InAxis);

private:
	static float RubberDelta(float OverStretching, float ViewSize);

private:
	bool IsHScrollingNeeded() const;
	bool IsVScrollingNeeded() const;

public:
	//~ Begin ILayoutElementInterface Interface 
	virtual void CalculateLayoutInputHorizontal() override {}
	virtual void CalculateLayoutInputVertical() override {}
	virtual float GetMinWidth() override { return -1; }
	virtual float GetPreferredWidth() override { return -1; }
	virtual float GetFlexibleWidth() override { return -1; }
	virtual float GetMinHeight() override { return -1; }
	virtual float GetPreferredHeight() override { return -1; }
	virtual float GetFlexibleHeight() override { return -1; }
	virtual int32 GetLayoutPriority() override { return -1; }
	//~ End ILayoutElementInterface Interface

public:
	//~ Begin ILayoutGroupInterface Interface
	virtual void SetLayoutHorizontal() override;
	virtual void SetLayoutVertical() override;
	//~ End ILayoutGroupInterface Interface

private:
	void UpdateScrollbarVisibility() const;

	static void UpdateOneScrollbarVisibility(bool bXScrollingNeeded, bool bXAxisEnabled, 
		EScrollRectScrollbarVisibility ScrollbarVisibility, UScrollbarComponent* Scrollbar);

	void UpdateScrollbarLayout() const;

protected:
	/**
	 * Calculate the bounds the ScrollRect should be using.
	 */
	void UpdateScrollRectBounds();

	static void AdjustBounds(const FBounds& InViewBounds, const FVector2D& InContentPivot, FVector& OutContentSize, FVector& OutContentPos);

private:
	FBounds GetBounds();

protected:
	static FBounds InternalGetBounds(FVector(&Corners)[4], const FTransform& ViewWorldToLocalMatrix);

private:
	FVector2D CalculateOffset(const FVector2D& InDelta) const;

protected:
	static FVector2D InternalCalculateOffset(const FBounds& InViewBounds, const FBounds& InContentBounds, bool bInHorizontal, bool bInVertical, EScrollRectMovementType InMovementType, const FVector2D& InDelta);
	
public:
	/**
	 * Sets the velocity to zero on both axes so the content stops moving.
	 */
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	virtual void StopMovement()
	{
		Velocity = FVector2D::ZeroVector;
	}
	
public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	URectTransformComponent* GetViewport() const
	{
		return Viewport;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetViewport(URectTransformComponent* InViewport)
	{
		Viewport = InViewport;
		SetDirtyCaching();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	UScrollbarComponent* GetHorizontalScrollbar() const
	{
		return HorizontalScrollbar;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetHorizontalScrollbar(UScrollbarComponent* InHorizontalScrollbar)
	{
		if (IsValid(HorizontalScrollbar))
		{
			HorizontalScrollbar->OnValueChanged.RemoveAll(this);
		}
		
		HorizontalScrollbar = InHorizontalScrollbar;
		
		if (IsValid(HorizontalScrollbar))
		{
			HorizontalScrollbar->OnValueChanged.AddUniqueDynamic(this, &UScrollRectComponent::InternalSetHorizontalNormalizedPosition);
		}

		SetDirtyCaching();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	UScrollbarComponent* GetVerticalScrollbar() const
	{
		return VerticalScrollbar;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetVerticalScrollbar(UScrollbarComponent* InVerticalScrollbar)
	{
		if (IsValid(VerticalScrollbar))
		{
			VerticalScrollbar->OnValueChanged.RemoveAll(this);
		}
			
		VerticalScrollbar = InVerticalScrollbar;
		
		if (IsValid(VerticalScrollbar))
		{
			VerticalScrollbar->OnValueChanged.AddUniqueDynamic(this, &UScrollRectComponent::InternalSetVerticalNormalizedPosition);
		}
		
		SetDirtyCaching();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	EScrollRectScrollbarVisibility GetHorizontalScrollbarVisibility() const
	{
		return HorizontalScrollbarVisibility;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetHorizontalScrollbarVisibility(EScrollRectScrollbarVisibility InHorizontalScrollbarVisibility)
	{
		HorizontalScrollbarVisibility = InHorizontalScrollbarVisibility;
		SetDirtyCaching();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	EScrollRectScrollbarVisibility GetVerticalScrollbarVisibility() const
	{
		return VerticalScrollbarVisibility;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetVerticalScrollbarVisibility(EScrollRectScrollbarVisibility InVerticalScrollbarVisibility)
	{
		VerticalScrollbarVisibility = InVerticalScrollbarVisibility;
		SetDirtyCaching();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	float GetHorizontalScrollbarSpacing() const
	{
		return HorizontalScrollbarSpacing;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetHorizontalScrollbarSpacing(float InHorizontalScrollbarSpacing)
	{
		HorizontalScrollbarSpacing = InHorizontalScrollbarSpacing;
		SetDirty();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	float GetVerticalScrollbarSpacing() const
	{
		return VerticalScrollbarSpacing;
	}

	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void SetVerticalScrollbarSpacing(float InVerticalScrollbarSpacing)
	{
		VerticalScrollbarSpacing = InVerticalScrollbarSpacing;
		SetDirty();
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	URectTransformComponent* GetViewRect()
	{
		if (!IsValid(ViewRect))
			ViewRect = Viewport;
		
		if (!IsValid(ViewRect))
			ViewRect = this;
		
		return ViewRect;
	}

public:
	UFUNCTION(BlueprintCallable, Category = ScrollRect)
	void StartAnimation(FVector2D TargetPos, bool bResetStartPos = true, bool bUseAnimation = false);

protected:
	void SetDirty();

	void SetDirtyCaching();
	
};
