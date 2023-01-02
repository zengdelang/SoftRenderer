#pragma once

#include "CoreMinimal.h"
#include "VertexHelper.h"
#include "Core/UICommonDefinitions.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/Renderer/UIRenderProxyInterface.h"
#include "CanvasRendererSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Canvas Renderer", DisallowMultipleComponent, HideEnableCheckBox=true))
class UGUI_API UCanvasRendererSubComponent : public UBehaviourSubComponent
{
	GENERATED_UCLASS_BODY()

	friend class UCanvasSubComponent;
	friend class FCanvasManager;
	friend class FUIMeshBatchDesc;
	friend class FUIMeshBatchDescStorage;
	friend struct FRendererCanvasTree;
	friend class UUIMeshProxyComponent;

public:
	TWeakPtr<FUIRenderProxyInfo> OwnerRenderProxyInfo;
	
protected:
	UPROPERTY(Transient)
	UCanvasSubComponent* ParentCanvas;

	UPROPERTY(Transient)
	UMaterialInterface* Material;

	UPROPERTY(Transient)
	UTexture* Texture;
	
	TWeakObjectPtr<USceneComponent> CustomRenderProxyComponent;
	
	TWeakPtr<FUIGraphicData> GraphicData;
	
	TSharedPtr<FVertexHelper> Mesh;
	
	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;
	FLinearColor Color;

	float InheritedAlpha;
	
	int32 AbsoluteDepth;

	EUIGraphicType GraphicType;

	uint8 bRefreshRenderProxy : 1;
	uint8 bRectClipping : 1;
	uint8 bShouldCull : 1;
	uint8 bAntiAliasing : 1;
	uint8 bTextElement : 1;
	
	/**
	 * Not visible and hit-testable (can interact with cursor).
	 */
	UPROPERTY(EditAnywhere, Category = Graphic)
	uint8 bHidePrimitive : 1;

	uint8 bUseCustomRenderProxy : 1;

	uint8 bUpdateUV1 : 1;
	uint8 bUpdateInheritedAlpha : 1;
	
public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void OnDynamicAdd() override;
	virtual void Awake() override;
	virtual void OnDestroy() override;
	virtual void OnZOrderChanged() override;
	virtual void OnCanvasHierarchyChanged() override;
	virtual void OnTransformParentChanged() override;
	virtual void OnTransformChanged() override;
	//~ End BehaviourSubComponent Interface.

protected:
	FORCEINLINE void SetParentCanvas(UCanvasSubComponent* NewParentCanvas);
	FORCEINLINE void DirtyParentCanvasData();

public:
	FORCEINLINE int32 GetAbsoluteDepth() const
	{
		if (IsValid(ParentCanvas))
		{
			return AbsoluteDepth + ParentCanvas->GetNestedRenderDepth();
		}
		return -1;
	}

protected:
	FORCEINLINE void SetAbsoluteDepth(int32 NewDepth) { AbsoluteDepth = NewDepth; };
	
	FORCEINLINE bool CanRender() const;
	
public:
	void FillMesh(FVertexHelper& VertexHelper);
	
	FORCEINLINE const TSharedPtr<FVertexHelper>& GetMesh()
	{
		return Mesh;
	}

	void Clear();
	
	FORCEINLINE bool IsShouldCull() const { return bShouldCull; }
	void SetShouldCull(bool bCull);

	FORCEINLINE bool IsAntiAliasing() const { return bAntiAliasing; }
	void SetAntiAliasing(bool bInAntiAliasing);

	FORCEINLINE bool IsRectClipping() const { return bRectClipping; }

	FORCEINLINE bool IsTextElement() const { return bTextElement; }
	void SetTextElement(bool bInIsTextElement);
	
	FORCEINLINE bool IsHidePrimitive() const { return bHidePrimitive; }
	void SetHidePrimitive(bool bInHidePrimitive);

	void EnableRectClipping(FRect InClipRect, FRect InClipSoftnessRect);
	void DisableRectClipping();

	FORCEINLINE const FLinearColor& GetColor() const { return Color; }
	void SetColor(FLinearColor InColor);
	
	void SetAlpha(float InAlpha);

	FORCEINLINE UMaterialInterface* SetMaterial() const { return Material; }
	void SetMaterial(UMaterialInterface* InMaterial);

	FORCEINLINE UTexture* GetTexture() const { return Texture; }
	void SetTexture(UTexture* InTexture);

	FORCEINLINE EUIGraphicType GetGraphicType() const { return GraphicType; };
	void SetGraphicType(EUIGraphicType InGraphicType);

	FORCEINLINE FUIGraphicData* GetGraphicData() const { return GraphicData.Pin().Get(); }
	void SetGraphicData(TWeakPtr<FUIGraphicData> InGraphicData);
	
	FORCEINLINE bool GetUseCustomRenderProxy() const { return bUseCustomRenderProxy; }
	void SetUseCustomRenderProxy(bool bInUseCustomRenderProxy);

	FORCEINLINE IUIRenderProxyInterface* GetCustomRenderProxyComponent() const { return Cast<IUIRenderProxyInterface>(CustomRenderProxyComponent.Get()); }
	void SetCustomRenderProxyComponent(IUIRenderProxyInterface* InRenderProxy);

	void UpdateMeshUV1(FVector2D InUV1);
	
	void SetInheritedAlpha(float InInheritedAlpha);
	FORCEINLINE float GetInheritedAlpha() const { return InheritedAlpha; }

	FORCEINLINE bool IsRefreshRenderProxy()
	{
		const bool bOldRefreshRenderProxy = bRefreshRenderProxy;
		bRefreshRenderProxy = false;
		return bOldRefreshRenderProxy;
	}
};
