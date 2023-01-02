#pragma once

#include "CoreMinimal.h"
#include "UIRenderProxyInterface.h"
#include "UISceneViewExtension.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "Core/UICommonDefinitions.h"
#include "Core/Renderer/UIVertex.h"
#include "Core/SpecializedCollections/ObjectIndexedSet.h"
#include "UIMeshProxyComponent.generated.h"

struct FMergedUIMesh
{
	FBox LocalBox;
	
	TArray<FUIVertex> Vertices;
	TArray<FDynamicMeshVertex> WorldSpaceVertices;
	TArray<uint32> Indices;

public:
	FMergedUIMesh()
	{
		LocalBox.IsValid = false;
		LocalBox.Min = FVector(MAX_FLT, MAX_FLT, MAX_FLT);
		LocalBox.Max = FVector(-MAX_FLT, -MAX_FLT, -MAX_FLT);
	}
};

struct FRenderDataUpdateInfo
{
	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;
	TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> GraphicData;
	TSharedPtr<FMergedUIMesh, ESPMode::ThreadSafe> MergedUIMesh;
	
	UTexture* MainTexture;
	UMaterialInterface* Material;

	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;

	FMatrix VirtualWorldTransform;
	uint32 VirtualTransformID;
	int32 TranslucentSortPriority;
	
	int32 BatchVerticesCount;
	int32 BatchIndexCount;
	
	ECanvasRenderMode RenderMode;
	EUIGraphicType GraphicType;
	
	uint8 bUseTextureArray : 1;
	uint8 bRectClipping : 1;
	uint8 bUseVirtualWorldTransform : 1;
	
public:
	FRenderDataUpdateInfo(): MainTexture(nullptr), Material(nullptr), VirtualTransformID(0),
	                         TranslucentSortPriority(0), BatchVerticesCount(0), BatchIndexCount(0),
	                         RenderMode(), GraphicType(),
	                         bUseTextureArray(false),
	                         bRectClipping(false),
	                         bUseVirtualWorldTransform(false)
	{
	}
};

UCLASS(Blueprintable, BlueprintType)
class UGUI_API UUIMeshProxyComponent : public UMeshComponent, public IUIRenderProxyInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	UTexture* MainTexture;

	UPROPERTY(Transient)
	UMaterialInterface* MaterialInterface;
	
	TSharedPtr<FUIMeshBatchElement> UIMeshBatchElement;

	TSharedPtr<FMergedUIMesh, ESPMode::ThreadSafe> MergedUIMesh;
	TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension;

	TWeakObjectPtr<class UUGUIWorldSubsystem> UIWorldSubsystem;
	TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> GraphicData;

	TSet<TWeakObjectPtr<UCanvasRendererSubComponent>> DirtyRenderers;

	FTransform WorldToCanvasTransform;
	
	int32 BatchVerticesCount;
	int32 BatchIndexCount;
	
	EUIGraphicType GraphicType;

	ECanvasRenderMode RenderMode;

	FLinearColor ClipRect;
	FLinearColor ClipSoftnessRect;
	uint8 bRectClipping : 1;
	
	uint8 bUseTextureArray : 1;
	uint8 bRefreshRenderData : 1;
	uint8 bUpdateTranslucentSortPriority : 1;
	uint8 bUpdateVirtualWorldTransform : 1;
	uint8 bUpdateSceneProxyTransform : 1;

public:
	UUIMeshProxyComponent(const FObjectInitializer& ObjectInitializer);

public:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.
	
protected:
	//~ Begin UActorComponent Interface.
	virtual void CreateRenderState_Concurrent(FRegisterComponentContext* Context) override;
	virtual void SendRenderTransform_Concurrent() override;
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual void UpdateBounds() override {}
	//~ Begin UActorComponent Interface.
	
public:
	virtual void SetRenderMode(ECanvasRenderMode InRenderMode, TWeakPtr<FUISceneViewExtension, ESPMode::ThreadSafe> ViewExtension) override;
	
	virtual void SetGraphicType(EUIGraphicType InGraphicType) override;
	virtual void SetGraphicData(TSharedPtr<FUIGraphicData, ESPMode::ThreadSafe> InGraphicData) override;
	
	virtual void ClearUIMesh() override;

	virtual void SetUIMaterial(const USceneComponent* CanvasSceneComp, UMaterialInterface* InMaterial, UTexture* InTexture,
		const FLinearColor& InClipRect, const FLinearColor& InClipSoftnessRect, bool bInRectClipping, bool bInUseTextureArray, bool bRefreshRenderProxy) override;
	
	virtual void UpdateTranslucentSortPriority(int32 NewTranslucentSortPriority) override;

	virtual bool IsExternalRenderProxy() override { return false; }

	virtual void UpdateVirtualWorldTransform(uint32 InVirtualTransformID, FMatrix InVirtualWorldTransform, bool bRemove,  USceneComponent* CanvasAttachComponent) override;

	virtual void SetupCanvas(const UCanvasSubComponent* Canvas, TSharedPtr<FUIRenderProxyInfo> InProxyInfo, TSharedPtr<FUIMeshBatchElement> InUIMeshBatchElement,
		const int32 InBatchVerticesCount, const int32 InBatchIndexCount, bool& bRefreshRenderProxy) override;

public:
	void UpdateDirtyRenderers();
	
};
