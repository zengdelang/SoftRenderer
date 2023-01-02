#pragma once

#include "CoreMinimal.h"
#include "RendererCanvasTree.h"
#include "UIMeshStorage.h"
#include "Core/Layout/RectTransformComponent.h"
#include "CanvasSubComponent.generated.h"

enum ECanvasDirtyFlag
{
	CDF_NotDirty = 0,
	CDF_BatchDirty = 1 << 1,
	CDF_DepthDirty = 1 << 2,
	CDF_MeshPriorityDirty = 1 << 3,
};

UENUM(BlueprintType)
enum class ECanvasPixelPerfectInheritMode : uint8
{
	CanvasPixelPerfectInheritMode_Inherit UMETA(DisplayName = Inherit),
	CanvasPixelPerfectInheritMode_On UMETA(DisplayName = On),
	CanvasPixelPerfectInheritMode_Off UMETA(DisplayName = Off),
};

DECLARE_MULTICAST_DELEGATE(FOnWillRenderCanvases);
DECLARE_MULTICAST_DELEGATE(FOnRenderModeChanged);
DECLARE_MULTICAST_DELEGATE(FOnScaleFactorChanged);
DECLARE_MULTICAST_DELEGATE(FOnParentCanvasChanged);

class UCanvasSubComponent;

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Canvas", DisallowMultipleComponent))
class UGUI_API UCanvasSubComponent : public UBehaviourSubComponent
{
	GENERATED_UCLASS_BODY()

	friend class FCanvasManager;
	friend struct FUIMeshStorage;
	friend class FUIMeshBatchDescStorage;
	friend class UUIMeshProxyComponent;
	friend struct FRendererCanvasTree;
	
protected:
	UPROPERTY(EditAnywhere, Category = CanvasSub)
	ECanvasRenderMode RenderMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CanvasSub)
	TEnumAsByte<ECameraProjectionMode::Type> ProjectionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CanvasSub)
	ECanvasRenderTargetMode RenderTargetMode;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CanvasSub)
	class UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CanvasSub)
	int32 RenderTimes;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* DefaultAAMaterial;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* DefaultTextMaterial;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* EditorPreviewDefaultMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* EditorPreviewDefaultAAMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	UMaterialInterface* EditorPreviewDefaultTextMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	ECanvasPixelPerfectInheritMode CanvasPixelPerfectInheritMode;
#endif
	
protected:
	UPROPERTY(Transient)
	UCanvasSubComponent* ParentCanvas;
	
	UPROPERTY(Transient)
	FRendererCanvasTree CanvasData;

	int32 NestedRenderDepth;
	int32 AbsoluteNestedRenderDepth;
	int32 CanvasInstructionIndex;
	
	UPROPERTY(Transient)
	FUIMeshStorage UIMeshStorage;

	int32 MinMeshPriority;

	int32 MeshPriorityCount;
	
	float ScaleFactor;
	float ReferencePixelsPerUnit;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	int32 SortingOrder;

	/** Cache that avoids Quat<->Rotator conversions if possible. Only to be used with RelativeRotation. */
	FRotationConversionCache RelativeRotationCache;

#if WITH_EDITOR
	
public:
	int32 UIBatches;
	int32 BlurUIBatches;
	int32 GlitchUIBatches;
	
#endif
	
public:
	FORCEINLINE URectTransformComponent* GetRectTransform() const
	{
		return AttachTransform;
	}
	
protected:	
	TSharedPtr<class FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;

	FDelegateHandle ViewportResizeDelegateHandle;

#if WITH_EDITOR
	FDelegateHandle EditorViewportResizeDelegateHandle;
#endif

	uint8 bUpdateNestedRenderDepth : 1;
	uint8 bHasValidViewportSize : 1;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	uint8 bOverrideSorting : 1;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	uint8 bPixelPerfect : 1;

	UPROPERTY(EditAnywhere, Category = CanvasSub)
	uint8 bOverridePixelPerfect : 1;
	
	UPROPERTY(EditAnywhere, Category = CanvasSub)
	uint8 bPerformFrustumCull : 1;
	
	UPROPERTY(EditAnywhere, Category = CanvasSub)
	uint8 bCheckSceneViewVisibility : 1;
	
	uint8 bClearMeshComponents : 1;

	uint8 bNeedUpdateRectTransform : 1;

protected:
	uint8 bUseVirtualWorldTransform : 1;

	uint8 bUpdateRenderTimes : 1;

	FTransform VirtualWorldTransform;

#if WITH_EDITOR
public:
	uint8 bForceUseSelfRenderMode : 1;
#endif
	
public:
	static FOnWillRenderCanvases OnWillRenderCanvases;
	static void ForceUpdateCanvases()
	{
		OnWillRenderCanvases.Broadcast();
	}

	FOnRenderModeChanged OnRenderModeChanged;
	FOnScaleFactorChanged OnScaleFactorChanged;
	FOnParentCanvasChanged OnParentCanvasChanged;
	
public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnDynamicAdd() override;
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDynamicRemove() override;
	virtual void OnDestroy() override;
	virtual void OnZOrderChanged() override;
	virtual void OnTransformParentChanged() override;
	virtual void OnCanvasHierarchyChanged() override;
	//~ End BehaviourSubComponent Interface.

protected:
	virtual void OnRemoveFromCanvasManager();
	virtual void OnAddToCanvasManager();
	virtual void OnSortInCanvasManager(int32 Index);

	void MarkNestedCanvasBatchDirty();
	
protected:
	void OnViewportResized();

#if WITH_EDITOR
	void OnEditorViewportResized(const UWorld* InWorld);
#endif
	
	void UpdateCanvasRectTransform();

public:
	TSharedPtr<class FUISceneViewExtension, ESPMode::ThreadSafe>& GetViewExtension();

	virtual bool CalculateCanvasMatrices(FVector& OutViewLocation, FMatrix& OutViewRotationMatrix, FMatrix& OutProjectionMatrix, FMatrix& OutViewProjectionMatrix, FMatrix& OutLocalToWorldMatrix);
	virtual bool CalculateCanvasMatrices(FVector& OutViewLocation, FMatrix& OutViewRotationMatrix, FMatrix& OutProjectionMatrix, FMatrix& OutViewProjectionMatrix, ECameraProjectionMode::Type InProjectionMode, FMatrix& OutLocalToWorldMatrix);

protected:
	virtual void CalculateProjectionMatrix(FMatrix& OutProjectionMatrix, ECameraProjectionMode::Type InProjectionMode);

	void AddNestedCanvas(UCanvasSubComponent* ChildCanvas);
	void RemoveNestedCanvas(UCanvasSubComponent* ChildCanvas);

private:
	void DispatchOnRectTransformDimensionsChange(USceneComponent* SceneComp) const;

	void UpdateCanvasOverrideSorting();

	void SyncTransformParent();
	static UBehaviourSubComponent* FindSubComponent(USceneComponent* SceneComp, UClass* SubClass);
	
public:
	ECameraProjectionMode::Type GetProjectionType() const
	{
		return ProjectionType;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UMaterialInterface* GetDefaultMaterial() const;
	
	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetDefaultMaterial(UMaterialInterface* InDefaultMaterial);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UMaterialInterface* GetDefaultAAMaterial() const;

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetDefaultAAMaterial(UMaterialInterface* InDefaultAAMaterial);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UMaterialInterface* GetDefaultTextMaterial() const;

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetDefaultTextMaterial(UMaterialInterface* InDefaultTextMaterial);

	UMaterialInterface* GetDefaultMaterialForUIMesh(bool bUseAntiAliasing, bool bTextMaterial) const;

public:
	FRendererCanvasTree& GetCanvasData()
	{
		return CanvasData;
	}
	
	void SetCanvasInstructionIndex(int32 InCanvasInstructionIndex)
	{
		CanvasInstructionIndex = InCanvasInstructionIndex;
	}
	
	int32 GetCanvasInstructionIndex() const
	{
		return CanvasInstructionIndex;
	}

	void SetNestedRenderDepth(int32 InNestedRenderDepth)
	{
		NestedRenderDepth = InNestedRenderDepth;
        bUpdateNestedRenderDepth = true;
        
        UpdateUpdateNestedCanvasRenderDepth(this);
	}

protected:
	static void UpdateUpdateNestedCanvasRenderDepth(UCanvasSubComponent* InCanvas)
	{
		for (const auto& NestedCanvas : InCanvas->CanvasData.NestedCanvases)
		{
			if (IsValid(NestedCanvas))
			{
				NestedCanvas->SetUpdateNestedRenderDepth(true);
				UpdateUpdateNestedCanvasRenderDepth(NestedCanvas);
			}
		}
	}
	
public:
	int32 GetNestedRenderDepth()
	{
		if (bUpdateNestedRenderDepth)
		{
			bUpdateNestedRenderDepth = false;
			
			if (!IsRootCanvas() && !bOverrideSorting)
			{
				AbsoluteNestedRenderDepth = NestedRenderDepth + ParentCanvas->GetNestedRenderDepth();
			}
			else
			{
				AbsoluteNestedRenderDepth = 0;
			}
		}
		return AbsoluteNestedRenderDepth;
	}

	void SetUpdateNestedRenderDepth(bool bInUpdateNestedRenderDepth)
	{
		bUpdateNestedRenderDepth = bInUpdateNestedRenderDepth;
	}
	
	UFUNCTION(BlueprintCallable, Category = Canvas)
	ECanvasRenderMode GetRenderMode() const
	{
		if (IsValid(ParentCanvas) && !bOverrideSorting)
			return ParentCanvas->GetRenderMode();
		
		return RenderMode;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetRenderMode(ECanvasRenderMode InRenderMode);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	ECanvasRenderTargetMode GetRenderTargetMode() const
	{
		return RenderTargetMode;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetRenderTargetMode(ECanvasRenderTargetMode InRenderTargetMode)
	{
		RenderTargetMode = InRenderTargetMode;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UTextureRenderTarget2D* GetRenderTarget() const
	{
		return RenderTarget;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetRenderTarget(UTextureRenderTarget2D* NewRenderTarget)
	{
		RenderTarget = NewRenderTarget;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	int32 GetRenderTimes() const
	{
		return RenderTimes;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetRenderTimes(int32 InRenderTimes)
	{
		RenderTimes = InRenderTimes;
		bUpdateRenderTimes = true;
	}

	bool IsUpdateRenderTimes() const
	{
		return bUpdateRenderTimes;
	}

	void SetUpdateRenderTimes(bool bInUpdateRenderTimes)
	{
		bUpdateRenderTimes = bInUpdateRenderTimes;
	}

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	UCanvasSubComponent* GetParentCanvas() const
	{
		return ParentCanvas;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UCanvasSubComponent* GetRootCanvas()
	{
		return IsValid(ParentCanvas) ? ParentCanvas->GetRootCanvas() : this;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	bool IsRootCanvas() const
	{
		return !IsValid(ParentCanvas);
	}
	
public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	float GetScaleFactor() const
	{
		return IsValid(ParentCanvas) ? ParentCanvas->GetScaleFactor() : ScaleFactor;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetScaleFactor(float InScaleFactor);

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	float GetReferencePixelsPerUnit() const
	{
		return IsValid(ParentCanvas) ? ParentCanvas->GetReferencePixelsPerUnit() : ReferencePixelsPerUnit;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetReferencePixelsPerUnit(float InReferencePixelsPerUnit);

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	int32 GetSortingOrder() const
	{
		return (IsValid(ParentCanvas) && !bOverrideSorting) ? ParentCanvas->GetSortingOrder() : SortingOrder;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetSortingOrder(int32 InSortingOrder);

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	int32 GetRenderOrder() const;

	UFUNCTION(BlueprintCallable, Category = Canvas)
	UCanvasSubComponent* GetOrderOverrideCanvas()
	{
		if (IsOverrideSorting() || !IsValid(ParentCanvas))
			return this;

		return ParentCanvas->GetOrderOverrideCanvas();
	}

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	bool IsOverrideSorting() const
	{
		return bOverrideSorting;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetOverrideSorting(bool bNewOverrideSorting);

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetVirtualWorldTransform(FTransform InVirtualWorldTransform);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void ClearVirtualWorldTransform();

	UFUNCTION(BlueprintCallable, Category = Canvas)
	FTransform GetVirtualWorldTransform() const
	{
		return VirtualWorldTransform;
	}

public:
	UFUNCTION(BlueprintCallable, Category = Canvas)
	bool GetPixelPerfect() const
	{
		if (IsValid(ParentCanvas) && !bOverridePixelPerfect)
			return ParentCanvas->GetPixelPerfect();

		return bPixelPerfect;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetPixelPerfect(bool bInPixelPerfect);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	bool GetOverridePixelPerfect() const
	{
		return bOverridePixelPerfect;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetOverridePixelPerfect(bool bInOverridePixelPerfect);

	UFUNCTION(BlueprintCallable, Category = Canvas)
	bool IsPerformFrustumCull() const
	{
		return bPerformFrustumCull;
	}

	UFUNCTION(BlueprintCallable, Category = Canvas)
	void SetPerformFrustumCull(bool bInPerformFrustumCull)
	{
		bPerformFrustumCull = bInPerformFrustumCull;
	}

	bool IsCheckSceneViewVisibility() const
	{
		return bCheckSceneViewVisibility;		
	}

protected:
	int32 UpdateBatches(bool bUpdateRectTransform, int32 StartPriority);

	void UpdateUIMeshBatches();
	
	void UpdateMeshComponentsPriority(int32 StartPriority);
	void UpdateCanvasMeshComponentsPriority(int32& StartPriority, int32& PriorityCount);
	
	void CacheAllUIMesh(bool bIncludeChildren = false);
	void ClearAllMeshes();
	
};
