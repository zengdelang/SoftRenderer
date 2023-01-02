#pragma once

#include "CoreMinimal.h"
#include "RectTransformComponent.h"
#include "RectTransformPreviewComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEditorUIDesignerInfoChanged, const UWorld*);

struct UGUI_API FEditorUIDesignerRect
{
public:
	float XMin;
	float YMin;
	
	float XMax;
	float YMax;

	FVector2D AnchorMin;
	FVector2D AnchorMax;

	FVector2D SizeDelta;
	FRect ParentRect;
	
	FTransform LocalToWorld;
	FTransform ParentLocalToWorld;

	TWeakObjectPtr<URectTransformPreviewComponent> Component;

	uint8 bMultiSelected : 1;
	uint8 bSelfSelected : 1;
	uint8 bSiblingSelected : 1;
	uint8 bIsRootCanvas: 1;
	uint8 bIsParentSelected : 1;
	
public:
	FEditorUIDesignerRect()
	{
		Reset();
	}

	void Reset()
	{
		XMin = 0;
		YMin = 0;
		XMax = 0;
		YMax = 0;

		AnchorMin = FVector2D::ZeroVector;
		AnchorMax = FVector2D::ZeroVector;

		SizeDelta = FVector2D::ZeroVector;
		ParentRect = FRect();
		
		LocalToWorld = FTransform::Identity;
		ParentLocalToWorld = FTransform::Identity;
		Component = nullptr;

		bMultiSelected = false;
		bSelfSelected = false;
		bSiblingSelected = false;
		bIsRootCanvas = false;
		bIsParentSelected = false;
	}
};

struct UGUI_API FEditorUIDesignerInfo
{
public:
	FEditorUIDesignerRect RootRect;
	FEditorUIDesignerRect ParentRect;
	FEditorUIDesignerRect CurSelectedRect;
	
	TArray<FEditorUIDesignerRect> CurSelectedSiblingRects;
	TArray<FEditorUIDesignerRect> NegativeSizeRects;

	TWeakObjectPtr<UActorComponent> RootRectComponent;
	TWeakObjectPtr<UActorComponent> ParentSelectedRectComponent;
	TWeakObjectPtr<UActorComponent> CurSelectedRectComponent;

	TArray<TWeakObjectPtr<UActorComponent>> CurSelectedSiblingRectComponents;

	bool bMultiSelected = false;
	
public:
	void Clear()
	{
		RootRect.Reset();
		CurSelectedRect.Reset();
		CurSelectedSiblingRects.Empty();
		NegativeSizeRects.Empty();

		RootRectComponent = nullptr;
		ParentSelectedRectComponent = nullptr;
		CurSelectedRectComponent = nullptr;

		CurSelectedSiblingRectComponents.Empty();

		bMultiSelected = true;
	}
};

UCLASS(Blueprintable, BlueprintType)
class UGUI_API URectTransformPreviewComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = RectTransformPreview)
	float XMin;

	UPROPERTY(EditAnywhere, Category = RectTransformPreview)
	float YMin;

	UPROPERTY(EditAnywhere, Category = RectTransformPreview)
	float XMax;

	UPROPERTY(EditAnywhere, Category = RectTransformPreview)
	float YMax;

	FVector2D AnchorMin;
	FVector2D AnchorMax;

	FVector2D SizeDelta;

	uint8 bIsRootCanvas : 1;
	
public:
	static FOnEditorUIDesignerInfoChanged OnEditorUIDesignerInfoChanged;
	
public:
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

public:
	virtual FPrimitiveSceneProxy* CreateSceneProxy()override;

public:
	static FEditorUIDesignerInfo* GetEditorUIDesignerInfo(UWorld* InWorld);
	static void ClearEditorUIDesignerInfo(UWorld* InWorld);

};
