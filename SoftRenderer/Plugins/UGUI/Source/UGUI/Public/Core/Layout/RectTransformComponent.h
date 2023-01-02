#pragma once

#include "CoreMinimal.h"
#include "Core/BehaviourComponent.h"
#include "RectTransformComponent.generated.h"

UENUM(BlueprintType)
enum class ERectTransformAxis : uint8
{
	RectTransformAxis_Horizontal UMETA(DisplayName = "Horizontal"),
	
	RectTransformAxis_Vertical UMETA(DisplayName = "Vertical"),
};

UENUM(BlueprintType)
enum class ERectTransformEdge : uint8
{
	RectTransformEdge_Left UMETA(DisplayName = "Left"),

	RectTransformEdge_Right UMETA(DisplayName = "Right"),

	RectTransformEdge_Top UMETA(DisplayName = "Top"),

	RectTransformEdge_Bottom UMETA(DisplayName = "Bottom"),
};

USTRUCT(BlueprintType)
struct UGUI_API FRect
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rect)
	float XMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rect)
	float YMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rect)
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Rect)
	float Height;

public:
	FRect()
	{
		XMin = 0;
		YMin = 0;
		Width = 0;
		Height = 0;
	}
	
	FRect(float InXMin, float InYMin, float InWidth, float InHeight)
	{
		XMin = InXMin;
		YMin = InYMin;
		Width = InWidth;
		Height = InHeight;
	}

	FRect(const FVector2D& Position, const FVector2D& Size)
	{
		XMin = Position.X;
		YMin = Position.Y;
		Width = Size.X;
		Height = Size.Y;
	}

	FRect(const FRect& Other)
	{
		XMin = Other.XMin;
		YMin = Other.YMin;
		Width = Other.Width;
		Height = Other.Height;
	}

	FORCEINLINE FRect& operator = (const FRect& Other)
	{
		XMin = Other.XMin;
		YMin = Other.YMin;
		Width = Other.Width;
		Height = Other.Height;
		return *this;
	}

	FORCEINLINE bool operator == (const FRect& Other) const
	{
		return (XMin == Other.XMin && YMin == Other.YMin &&
			Width == Other.Width && Height == Other.Height);
	}
	
	FORCEINLINE bool operator != (const FRect& Other) const
	{
		return !(*this == Other);
	}

	FORCEINLINE FVector2D GetSize() const
	{
		return FVector2D(Width, Height);
	}

	FORCEINLINE float GetXMax() const
	{
		return XMin + Width;
	}

	FORCEINLINE void SetXMax(float InXMax)
	{
		Width = InXMax - XMin;
	}

	FORCEINLINE float GetYMax() const
	{
		return YMin + Height;
	}

	FORCEINLINE void SetYMax(float InYMax)
	{
		Height = InYMax - YMin;
	}

	FORCEINLINE FVector2D GetMin() const
	{
		return FVector2D(XMin, YMin);
	}

	FORCEINLINE FVector2D GetMax() const
	{
		return FVector2D(XMin + Width, YMin + Height);
	}
	
	FORCEINLINE bool Overlaps(const FRect& Other) const
	{
		return Other.GetXMax() > XMin && Other.XMin < GetXMax() && Other.GetYMax() > YMin && Other.YMin < GetYMax();
	}

	FORCEINLINE bool Overlaps(FRect Other, bool bAllowInverse) 
	{
		FRect& Rect = *this;
		if (bAllowInverse)
		{
			Rect = FRect::OrderMinMax(Rect);
			Other = FRect::OrderMinMax(Other);
		}
		return Rect.Overlaps(Other);
	}

	FORCEINLINE FVector2D GetPosition() const
	{
		return FVector2D(XMin, YMin);
	}

	FORCEINLINE FVector2D GetCenter() const
	{
		return FVector2D(XMin + Width * 0.5f, YMin + Height * 0.5f);
	}

private:
	static FRect& OrderMinMax(FRect& Rect)
	{
		const float NewXMax = Rect.GetXMax();
		if (Rect.XMin > NewXMax)
		{
			const float XMin = Rect.XMin;
			Rect.XMin = NewXMax;
			Rect.SetXMax(XMin);
		}

		const float NewYMax = Rect.GetYMax();
		if (Rect.YMin > NewYMax)
		{
			const float YMin = Rect.YMin;
			Rect.YMin = NewYMax;
			Rect.SetYMax(YMin);
		}
	
		return Rect;
	}
	
};

/**
 * NOTE:
 *	  1 Don't use SetRelativeLocation(), SetRelativeRotation(), SetRelativeScale3D(), please use SetLocalLocation(), SetLocalRotation(), SetLocalScale() instead.
 *	  2 Don't use Transform for UI sequence, please use LocalRotation, LocalScale, LocalPosition, LocalTransform, LocalPositionZ, AnchoredPosition instead.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Layout), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API URectTransformComponent : public UBehaviourComponent
{
	GENERATED_UCLASS_BODY()

public:
	friend class FRectTransformPreviewSceneProxy;
	
protected:
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D AnchorMin;

	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D AnchorMax;
	
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D AnchoredPosition;

	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D SizeDelta;

	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D Pivot;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D OffsetMin;

	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector2D OffsetMax;

	// for UI sequence
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	float LocalPositionZ;

	// for UI sequence
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector LocalLocation;

	// for UI sequence
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FRotator LocalRotation;

	// for UI sequence
	UPROPERTY(EditAnywhere, Category = RectTransformCommon)
	FVector LocalScale;
#endif
	
	FRect Rect;
	
#if WITH_EDITORONLY_DATA
	class URectTransformPreviewComponent* RectTransformPreview;
#endif
	
	uint8 bIsDetaching : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)
	uint8 bIsRootCanvas: 1;
#endif 
	
public:
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void AwakeFromLoad() override;
	
	virtual void OnAttachmentChanged() override;
	virtual void DetachFromComponent(const FDetachmentTransformRules& DetachmentRules) override;
	
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	virtual void UpdateRectPreview() const;
#endif

public:
#if WITH_EDITORONLY_DATA
	virtual void UpdateRectTransformPreview() override;
	virtual void SetIsRootCanvas(bool bInIsRootCanvas) override;
#endif
	
protected: 
	virtual void InternalTransformParentChanged() override;

public:
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetAnchorMin() const
	{
		return AnchorMin;
	}
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetAnchorMin(FVector2D InAnchorMin);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetAnchorMax() const
	{
		return AnchorMax;
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetAnchorMax(FVector2D InAnchorMax);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetAnchoredPosition() const
	{
		return AnchoredPosition;
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector GetAnchoredPosition3D() const
	{
		return FVector(AnchoredPosition.X, AnchoredPosition.Y, GetRelativeLocation().Z);
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	bool SetAnchoredPosition3D(FVector InValue);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	bool SetAnchoredPosition(FVector2D InAnchoredPosition);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetSizeDelta() const
	{
		return SizeDelta;
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetSizeDelta(FVector2D InSizeDelta);
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetPivot() const
	{
		return Pivot;
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetPivot(FVector2D InPivot);
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetOffsetMin() const
	{
		return AnchoredPosition - SizeDelta * Pivot;
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetOffsetMin(FVector2D InOffsetMin);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FVector2D GetOffsetMax() const
	{
		return AnchoredPosition + SizeDelta * (FVector2D::UnitVector - Pivot);
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetOffsetMax(FVector2D InOffsetMax);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FORCEINLINE FRect GetRect() const
	{
		return Rect;
	}

	FORCEINLINE void SetAnchor(FVector2D InAnchorMin, FVector2D InAnchorMax)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndSize(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InSizeDelta)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		SizeDelta = InSizeDelta;

#if WITH_EDITORONLY_DATA
		OffsetMin = GetOffsetMin();
		OffsetMax = GetOffsetMax();
#endif
		
		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndOffset(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InOffsetMin, FVector2D InOffsetMax)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;
		
		{
			const FVector2D Offset = InOffsetMin - (AnchoredPosition - SizeDelta * Pivot);
			const FVector2D AnchoredPosOffset = Offset * (FVector2D::UnitVector - Pivot);

			SizeDelta -= Offset;
			AnchoredPosition += AnchoredPosOffset;

#if WITH_EDITORONLY_DATA
			OffsetMin = InOffsetMin;
#endif
		}

		{
			const FVector2D Offset = InOffsetMax - (AnchoredPosition + SizeDelta * (FVector2D::UnitVector - Pivot));
			const FVector2D AnchoredPosOffset = Offset * Pivot;

			SizeDelta += Offset;
			AnchoredPosition += AnchoredPosOffset;
			
#if WITH_EDITORONLY_DATA
			OffsetMax = InOffsetMax;
#endif
		}

		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndOffsetAndPivot(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InOffsetMin, FVector2D InOffsetMax, FVector2D InPivot)
	{
		Pivot = InPivot;
		
		SetAnchorAndOffset(InAnchorMin, InAnchorMax, InOffsetMin, InOffsetMax);
	}

	FORCEINLINE void SetAnchorAndPivot(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InPivot)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		Pivot = InPivot;

#if WITH_EDITORONLY_DATA
		OffsetMin = GetOffsetMin();
		OffsetMax = GetOffsetMax();
#endif
		
		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndSizeAndPivot(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InSizeDelta, FVector2D InPivot)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		SizeDelta = InSizeDelta;

		Pivot = InPivot;

#if WITH_EDITORONLY_DATA
		OffsetMin = GetOffsetMin();
		OffsetMax = GetOffsetMax();
#endif
		
		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndPosition(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InAnchoredPosition)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		AnchoredPosition = InAnchoredPosition;

#if WITH_EDITORONLY_DATA
		OffsetMin = GetOffsetMin();
		OffsetMax = GetOffsetMax();
#endif
		
		UpdateRect();
	}

	FORCEINLINE void SetAnchorAndSizeAndPosition(FVector2D InAnchorMin, FVector2D InAnchorMax, FVector2D InSizeDelta, FVector2D InAnchoredPosition)
	{
		AnchorMin = InAnchorMin;
		AnchorMax = InAnchorMax;

		SizeDelta = InSizeDelta;
		
		AnchoredPosition = InAnchoredPosition;

#if WITH_EDITORONLY_DATA
		OffsetMin = GetOffsetMin();
		OffsetMax = GetOffsetMax();
#endif
		
		UpdateRect();
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetInsetAndSizeFromParentEdge(ERectTransformEdge Edge, float Inset, float Size);
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetSizeWithCurrentAnchors(ERectTransformAxis Axis, float Size);

	void GetLocalCorners(FVector(&Corners)[4]) const;
	void GetWorldCorners(FVector(&Corners)[4]) const;
	
public:
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FVector GetLocalLocation() const
	{
		return GetRelativeLocation();
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	bool SetLocalLocation(FVector NewLocation)
	{
		const FVector& CurRelativeLocation = GetRelativeLocation();
		return SetAnchoredPosition3D(FVector(AnchoredPosition.X + NewLocation.X - CurRelativeLocation.X,
			AnchoredPosition.Y + NewLocation.Y - CurRelativeLocation.Y, NewLocation.Z));
	}

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	float GetLocalPositionZ() const
	{
		return GetRelativeLocation().Z;
	}
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetLocalPositionZ(float NewZ);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FRotator GetLocalRotation() const
	{
		return GetRelativeRotation();
	}
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetLocalRotation(FRotator NewRotation);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FVector GetLocalScale() const
	{
		return GetRelativeScale3D();
	}
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetLocalScale(FVector NewScale);

	UFUNCTION(BlueprintCallable, Category = RectTransform)
	FTransform GetLocalTransform() const
	{
		return GetRelativeTransform();
	}
	
	UFUNCTION(BlueprintCallable, Category = RectTransform)
	void SetLocalTransform(FTransform NewTransform);

public:
	virtual void SetEnabled(bool bNewEnabled) override;
	
protected:
	FORCEINLINE FVector2D GetParentSize() const
	{
		const auto ParentRectTransform = Cast<URectTransformComponent>(GetAttachParent());
		if (!IsValid(ParentRectTransform))
			return FVector2D::ZeroVector;
		return ParentRectTransform->Rect.GetSize();
	}

	FORCEINLINE FRect GetParentRect() const
	{
		const auto ParentRectTransform = Cast<URectTransformComponent>(GetAttachParent());
		if (!IsValid(ParentRectTransform))
			return FRect();
		return ParentRectTransform->Rect;
	}

	bool UpdateRect(bool bUpdateRect = true, bool bTransformChanged = false);

private:
	void DispatchOnTransformChanged(UBehaviourComponent* BehaviourComp) const;

};
