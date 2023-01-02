#pragma once

#include "CoreMinimal.h"
#include "BehaviourSubComponent.generated.h"

class UCanvasSubComponent;

UCLASS(Abstract, Blueprintable, BlueprintType, EditInlineNew)
class UGUI_API UBehaviourSubComponent : public UObject
{
	GENERATED_UCLASS_BODY()

	friend class UBehaviourComponent;
	friend class URectTransformComponent;
	friend class UCanvasSubComponent;
	
protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Activation)
	uint8 bIsEnabled : 1;

private:
	uint8 bHasBeenAwaken : 1;

	/** Indicates that OnEnable has been called, but OnDisable has not yet */
	uint8 bHasBeenEnabled : 1;

	/** Indicates that Start has been called */
	uint8 bHasStarted : 1;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Graphic)
	uint8 bInteractable : 1;

private:
	uint8 bCurrentInteractable : 1;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Graphic)
	uint8 bBlockRaycasts : 1;

private:
	uint8 bCurrentBlockRaycasts : 1;
	
protected:	
    UPROPERTY(Transient)
    class URectTransformComponent* AttachTransform;
    	
public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void InternalAdd();
	void InternalAwake();
	void InternalRemove();
	void InternalDestroy();

	void UpdateEnableState();
	void InternalUpdateInteractableState();
	void InternalUpdateBlockRaycastsState();

protected:
	UCanvasSubComponent* GetOwnerCanvas() const;
	
public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool HasBeenAwaken() const { return bHasBeenAwaken; }

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool HasBeenEnabled() const { return bHasBeenEnabled; }
	
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	virtual void SetEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsEnabled() const { return bIsEnabled; }

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsEnabledInHierarchy() const;

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsActiveAndEnabled() const { return IsEnabledInHierarchy() && bHasBeenEnabled; }

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsInteractableInHierarchy() const;
    
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsInteractable() const { return bInteractable; }

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	void SetInteractable(bool bInInteractable);

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsBlockRaycastsInHierarchy() const;
    
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsBlockRaycasts() const { return bBlockRaycasts; }

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	void SetBlockRaycasts(bool bInBlockRaycasts);

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	bool IsIgnoreReversedGraphicsInHierarchy() const;
	
protected:
	virtual void OnDynamicAdd() {}
	
	virtual void Awake();

	/** Note: Do not change the attachment state during this call. */
	virtual void OnEnable() {}

	virtual void Start() {}

	/** Note: Do not change the attachment state during this call. */
	virtual void OnDisable() {}

	virtual void OnDynamicRemove() {}
	
	virtual void OnDestroy() {}

	virtual void OnInteractableStateChanged() {}

	virtual void OnBlocksRaycastsStateChanged() {}
	
protected:
	/**
	 * This callback is called if an associated RectTransform has its dimensions changed. The call is also made to all child rect transforms, even if the child transform itself doesn't change - as it could have, depending on its anchoring.
	 *
	 * Note: Do not change the attachment state during this call.
	 */
	virtual void OnRectTransformDimensionsChange() {}

	virtual void OnTransformParentChanged() {}
	
	/**
	 * Called when the state of the parent Canvas is changed.
	 */
	virtual void OnCanvasHierarchyChanged() {}

	virtual void OnTransformChanged() {}

	virtual	void InternalOnZOrderChanged();

	virtual void OnZOrderChanged() {}

	virtual void OnChildAttachmentChanged() {}

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	UObject* GetComponent(UClass* InClass, bool bIncludeInactive = false) const;

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	UObject* GetComponentInChildren(UClass* InClass, bool bIncludeInactive = false) const;

private:
	UObject* InternalGetComponentInChildren(USceneComponent* SceneComp, UClass* InClass, bool bIncludeInactive = false) const;

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	UObject* GetComponentInParent(UClass* InClass, bool bIncludeInactive = false) const;

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	void GetComponents(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false) const;

	UFUNCTION(BlueprintCallable, Category = Behaviour)
	void GetComponentsInChildren(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false) const;

private:
	void InternalGetComponentsInChildren(USceneComponent* SceneComp, TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false) const;

public:
	UFUNCTION(BlueprintCallable, Category = Behaviour)
	void GetComponentsInParent(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false) const;

};
