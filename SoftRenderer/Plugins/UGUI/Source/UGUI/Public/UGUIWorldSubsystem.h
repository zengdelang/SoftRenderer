#pragma once

#include "CoreMinimal.h"
#include "Core/Render/CanvasSubComponent.h"
#include "Core/SpecializedCollections/HashOctree.h"
#include "EventSystem/EventData/PointerEventData.h"
#include "Subsystems/WorldSubsystem.h"
#include "UGUIWorldSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FMaterialInstanceDynamicPool
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> Instances;
};

UCLASS(BlueprintType, Blueprintable)
class UGUI_API UUGUIWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UUGUIWorldSubsystem();

public:
	DECLARE_MULTICAST_DELEGATE(FOnGameViewportResized)
	FOnGameViewportResized ViewportResizedEvent;

	int32 UIMeshCreationTimes;
	int32 MaterialDynamicInstanceCreationTimes;
	
protected:
	UPROPERTY(Transient)
	TArray<USceneComponent*> UIMeshes;

	UPROPERTY(Transient)
	TArray<UAudioComponent*> UIAudioComponents;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<UMaterialInterface>, FMaterialInstanceDynamicPool> DynamicMaterialInstances;
	
	UPROPERTY(Transient)
	AActor* UIMeshActor;
	
	FVector2D CacheViewportSize;

	bool bIsDirty;

	TMap<TWeakObjectPtr<UCanvasSubComponent>, FHashOctree> CanvasOctrees;
	
	TMap<TWeakObjectPtr<UCanvasSubComponent>, TSet<TWeakObjectPtr<UObject>>> PendingAddGraphics;
	TMap<TWeakObjectPtr<UCanvasSubComponent>, TSet<TWeakObjectPtr<UObject>>> PendingRemoveGraphics;
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

protected:
	void UpdateViewportSize();

public:
	void AddRaycastTarget(UCanvasSubComponent* InCanvas, UObject* Graphics);
	void RemoveRaycastTarget(UCanvasSubComponent* InCanvas, UObject* Graphics);

	void RaycastAll(const UPointerEventData* EventData, FRaycastResult& RaycastResult);
	
protected:
	void UpdateRectTransformOctree();
	
public:
	void AddUnusedMesh(USceneComponent* UIMeshComp);
	USceneComponent* GetUnusedMesh();

	void AddUnusedMaterial(UMaterialInterface* ParentMaterial, UMaterialInstanceDynamic* MaterialInstanceDynamic);
	UMaterialInstanceDynamic* GetUnusedMaterial(UMaterialInterface* ParentMaterial);
	
	UAudioComponent* GetAudioComponent(UObject* PrincipalObject,bool bMasterTrack);
	void CacheAudioComponent(UAudioComponent* CacheAudioComp);
	
protected:
	void ShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const;
	
};
