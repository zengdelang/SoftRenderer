#pragma once

#include "CoreMinimal.h"
#include "Core/Widgets/ImageComponent.h"
#include "UIRaycastRegionComponent.generated.h"

struct FRaycastRegionBox
{
	FVector2D BottomLeft;
	FVector2D TopLeft;
	FVector2D TopRight;
	FVector2D BottomRight;
};

UCLASS(Blueprintable, BlueprintType)
class UIBLUEPRINTEDITOR_API UUIRaycastRegionComponent : public UMaskableGraphicComponent
{
	GENERATED_UCLASS_BODY()

public:
	TWeakPtr<class FSCSUIEditorViewportClient> ViewportClient;

protected:
	UPROPERTY(Transient)
	TArray<URectTransformComponent*> RaycastRegionComponents;
	
	TArray<FRaycastRegionBox> RaycastRegionBoxList;

	FLinearColor RegionColor;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void OnPopulateMesh(FVertexHelper& VertexHelper) override;

private:
	void GetGraphicElements(USceneComponent* SceneComp);

};
