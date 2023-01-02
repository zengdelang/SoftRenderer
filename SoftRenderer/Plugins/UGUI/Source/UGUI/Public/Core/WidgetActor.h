#pragma once

#include "CoreMinimal.h"
#include "Layout/RectTransformComponent.h"
#include "WidgetActor.generated.h"

#ifndef OPTIMIZE_SPAWN_ACTOR_FOR_UI
#define OPTIMIZE_SPAWN_ACTOR_FOR_UI 0
#endif

USTRUCT()
struct FChildComponentOrder
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FName> ChildrenNames;

};

class AWidgetActor;

#if WITH_EDITORONLY_DATA
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBackgroundImageFilePathChanged, AWidgetActor*, const FString&);
#endif

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActorEnableStateChanged, bool);

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
DECLARE_MULTICAST_DELEGATE(FOnSliceSpawnActorEvent);
#endif

UCLASS(Abstract, Blueprintable, BlueprintType)
class UGUI_API AWidgetActor : public AActor
{
    GENERATED_UCLASS_BODY()

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient);
	UObject* ParentContainerComponent;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWidgetActorConstructionEvent);
	UPROPERTY(Transient);
	FOnWidgetActorConstructionEvent OnWidgetActorConstruction;

	UPROPERTY(Transient);
	UObject* AttachParentComponent;
#endif

	UPROPERTY(Transient)
	UObject* ParametersObject;

	UPROPERTY()
	TMap<FName, FChildComponentOrder> ComponentChildrenOrders;

	TWeakObjectPtr<USceneComponent> AttachSceneComponent;

	FOnActorEnableStateChanged OnActorEnableStateChanged;
	
	UPROPERTY()
	uint8 bRebuildChildrenOrder : 1;

	UPROPERTY(Transient)
	uint8 bDisableCanvasUpdateRectTransform : 1;
	
	UPROPERTY(Transient)
	uint8 bStripCanvasComponent : 1;
	
	UPROPERTY(Transient)
	uint8 bStripCanvasScalerComponent : 1;

	uint8 bActorEnabled : 1;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = EditorPreview)
	FString BackgroundImageFilePath;
	
    static FOnBackgroundImageFilePathChanged OnBackgroundImageFilePathChanged;
#endif
	
#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
	FOnSliceSpawnActorEvent OnSlicePreRegisterComponents;
	FOnSliceSpawnActorEvent OnSliceSpawnActorDone;
#endif
	
#if WITH_EDITOR
	uint8 bRecordForUndo : 1;
#endif
	
public:
	virtual void Serialize(FArchive& Ar) override;
    virtual void PostRegisterAllComponents() override;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	virtual void PreRegisterAllComponents() override {}
	virtual void PostUnregisterAllComponents() override {}
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	/**
	 * Note that the object will be modified.  If we are currently recording into the 
	 * transaction buffer (undo/redo), save a copy of this object into the buffer and 
	 * marks the package as needing to be saved.
	 *
	 * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
	 *								currently recording an active undo/redo transaction
	 * @return true if the object was saved to the transaction buffer
	 */
#if WITH_EDITOR
	virtual bool Modify(bool bAlwaysMarkDirty = true) override;
#endif
	
protected:
	void AwakeComponents(USceneComponent* SceneComp) const;

protected:
	void RebuildChildrenOrders(USceneComponent* Comp);
	
#if WITH_EDITORONLY_DATA
	void SerializeChildrenOrders(const USceneComponent* Comp);
#endif

public:
	virtual UClass* GetCustomParametersObjectClass();
	virtual void SetCustomParametersObject(UObject* InParametersObject);

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
	virtual void UpdateAllReplicatedComponents() override {}
#endif

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = WidgetActor, meta = (DisplayName = "GetCustomParametersObjectClass"))
	UClass* ReceiveGetCustomParametersObjectClass();

	UFUNCTION(BlueprintImplementableEvent, Category = WidgetActor, meta = (DisplayName = "ReceiveCustomParametersObject"))
	void ReceiveCustomParametersObject(UObject* InParametersObject);

public:
	virtual void OnActorEnable()
	{
		bActorEnabled = true;
		OnActorEnableStateChanged.Broadcast(true);
	}
	
	virtual void OnActorDisable()
	{
		bActorEnabled = false;
		OnActorEnableStateChanged.Broadcast(false);
	}

public:
#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
	TArray<TWeakObjectPtr<AActor>> ChildSliceActors;
	
	virtual void RerunConstructionScripts() override;
    	
	virtual void SlicePreRegisterComponents() override;
	virtual ESpawnActorSliceAction SliceCustomActions() override;
	virtual void SlicePostActorConstructionDone() override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	URectTransformComponent* GetRootRectTransformComponent() const
	{
		return Cast<URectTransformComponent>(GetRootComponent());
	}

	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	void SetZOrder(int32 NewZOrder)
	{
		if (URectTransformComponent* RootRectComp = GetRootRectTransformComponent())
		{
			RootRectComp->SetZOrder(NewZOrder);
		}
	}

	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	void SetEnabled(bool bNewEnabled)
	{
		if (URectTransformComponent* RootRectComp = GetRootRectTransformComponent())
		{
			RootRectComp->SetEnabled(bNewEnabled);
		}
	}

	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	bool IsEnabled() const
	{
		if (const URectTransformComponent* RootRectComp = GetRootRectTransformComponent())
		{
			return RootRectComp->IsEnabled();
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	bool IsEnabledInHierarchy() const
	{
		if (const URectTransformComponent* RootRectComp = GetRootRectTransformComponent())
		{
			return RootRectComp->IsEnabledInHierarchy();
		}
		return false;
	}

	UFUNCTION(BlueprintCallable, Category = WidgetActor)
	bool IsActiveAndEnabled() const
	{
		if (const URectTransformComponent* RootRectComp = GetRootRectTransformComponent())
		{
			return RootRectComp->IsActiveAndEnabled();
		}
		return false;
	}
};
