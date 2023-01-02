#pragma once

#include "CoreMinimal.h"
#include "BehaviourSubComponent.h"
#include "BehaviourComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, HideCategories=(Tags, Cooking, Physics, Collision, LOD, Rendering, Activation, ComponentReplication, AssetUserData, ComponentTick))
class UGUI_API UBehaviourComponent : public USceneComponent
{
    GENERATED_UCLASS_BODY()

    friend class UCanvasSubComponent;
    
protected:
    UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = SubComponents)
    TArray<UBehaviourSubComponent*> SubComponents;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    float RenderOpacity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    int32 ZOrder;

    UPROPERTY(Transient)
    class UCanvasSubComponent* OwnerCanvas;

public:
#if WITH_EDITORONLY_DATA
    FName EditorVariableName;
    FLinearColor EditorRowColor = FLinearColor::White;
#endif

private:
    float ParentRenderOpacityInHierarchy;
    float CurrentRenderOpacity;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bEnabled : 1;
	
private:
    uint8 bParentEnabledInHierarchy : 1; 
	
    /** Indicates that OnEnable has been called, but OnDisable has not yet */
    uint8 bHasBeenEnabled : 1;
    
    uint8 bHasBeenAwaken : 1;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bInteractable : 1;
	
private:
    uint8 bParentInteractableInHierarchy : 1;
    uint8 bCurrentInteractable : 1;
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bBlockRaycasts : 1;
	
private:
    uint8 bParentBlockRaycastsInHierarchy : 1;
    uint8 bCurrentBlockRaycasts : 1;
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bIgnoreReversedGraphics : 1;
	
private:
    uint8 bParentIgnoreReversedGraphicsInHierarchy : 1;
    uint8 bCurrentIgnoreReversedGraphics : 1;
    
public:
    uint8 bDisableTransformParentChanged : 1;

	uint8 bIsRootComponent : 1;
    
#if WITH_EDITORONLY_DATA
    UPROPERTY(EditAnywhere, Category = Behaviour)
    uint8 bRefreshDetailForEditor : 1;

    UPROPERTY()
    uint8 bIsVariable : 1;

    UPROPERTY()
    uint8 bIsLockForEditor : 1;

    UPROPERTY()
    uint8 bIsVisibleForEditor : 1;
#endif

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bGraying : 1;

private:
    uint8 bParentGrayingInHierarchy : 1;
    uint8 bCurrentGraying : 1;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bInvertColor : 1;

    uint8 bAttachChildChanged : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bIgnoreParentRenderOpacity : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bIgnoreParentInteractable : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bIgnoreParentBlockRaycasts : 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behaviour)
    uint8 bIgnoreParentReversedGraphics : 1;

private:
    uint8 bParentInvertColorInHierarchy : 1;
    uint8 bCurrentInvertColor : 1;

public:
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

    virtual void AwakeFromLoad();
	
#if WITH_EDITORONLY_DATA
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    virtual void AddSubComponentForEditor(UBehaviourSubComponent* SubComponent);
    virtual void MoveSubComponentForEditor(int32 OldIndex, int32 NewIndex);
    virtual void RemoveSubComponentForEditor(int32 Index);
#endif

private:
    void InternalAwake();
    void InternalDestroy();

private:
    void UpdateParentEnabledInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void UpdateEnableState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    void UpdateParentInteractableInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void InternalUpdateInteractableState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    void UpdateParentBlockRaycastsInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void InternalUpdateBlockRaycastsState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    void UpdateParentIgnoreReversedGraphicsInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void InternalUpdateIgnoreReversedGraphicsState(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    void UpdateParentGrayingInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void UpdateGrayingStateChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    void UpdateParentInvertColorInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void UpdateInvertColorStateChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);
    
private:
    void UpdateParentRenderOpacityInHierarchy(const UBehaviourComponent* ParentBehaviourComp);
    static void UpdateRenderOpacityChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp);

private:
    static void InternalOnZOrderChanged(const UBehaviourComponent* BehaviourComp);

private:
    void SetOwnerCanvas(class UCanvasSubComponent* InCanvas, bool bUpdateOwnerCanvas, bool bThroughChildren);
    static void InternalSetOwnerCanvas(UBehaviourComponent* BehaviourComp, class UCanvasSubComponent* InCanvas, bool bUpdateOwnerCanvas, bool bThroughChildren);
    
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    float GetRenderOpacity() const { return RenderOpacity; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    float GetRenderOpacityInHierarchy() const { return RenderOpacity * ParentRenderOpacityInHierarchy; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    virtual void SetRenderOpacity(float InRenderOpacity);
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetIgnoreParentRenderOpacity(bool bInIgnoreParentRenderOpacity);
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreParentRenderOpacity() const { return bIgnoreParentRenderOpacity; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetIgnoreParentInteractable(bool bInIgnoreParentInteractable);
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreParentInteractable() const { return bIgnoreParentInteractable; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetIgnoreParentBlockRaycasts(bool bInIgnoreParentBlockRaycasts);
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreParentBlockRaycasts() const { return bIgnoreParentBlockRaycasts; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetIgnoreParentReversedGraphics(bool bInIgnoreParentReversedGraphics);
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreParentReversedGraphics() const { return bIgnoreParentReversedGraphics; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    virtual void SetZOrder(int32 InZOrder);

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    int32 GetZOrder() const { return ZOrder; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UCanvasSubComponent* GetOwnerCanvas() const;
    
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool HasBeenAwaken() const { return bHasBeenAwaken; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool HasBeenEnabled() const { return bHasBeenEnabled; }
	
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    virtual void SetEnabled(bool bNewEnabled);

#if WITH_EDITORONLY_DATA
    void SetVisibilityForEditor(bool bNewVisibility)
    {
        bIsVisibleForEditor = bNewVisibility;
        UpdateEnableState(this, Cast<UBehaviourComponent>(GetAttachParent()));
    }
#endif

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsEnabled() const { return bEnabled; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsEnabledInHierarchy() const;

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsActiveAndEnabled() const { return IsEnabledInHierarchy() && bHasBeenEnabled; }

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsInteractableInHierarchy() const { return bParentInteractableInHierarchy && bInteractable; }
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsInteractable() const { return bInteractable; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetInteractable(bool bInInteractable);
    
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsBlockRaycastsInHierarchy() const { return bParentBlockRaycastsInHierarchy && bBlockRaycasts; }
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsBlockRaycasts() const { return bBlockRaycasts; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetBlockRaycasts(bool bInBlockRaycasts);
    
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreReversedGraphicsInHierarchy() const { return bParentIgnoreReversedGraphicsInHierarchy && bIgnoreReversedGraphics; }
    
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsIgnoreReversedGraphics() const { return bIgnoreReversedGraphics; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void SetIgnoreReversedGraphics(bool bInIgnoreReversedGraphics);
	
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    virtual void SetGraying(bool bInGraying);
	
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsGraying() const { return bGraying; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsGrayingInHierarchy() const { return bGraying ||  bParentGrayingInHierarchy; }

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    virtual void SetInvertColor(bool bInInvertColor);

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsInvertColor() const { return bInvertColor; }

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool IsInvertColorInHierarchy() const { return bInvertColor || bParentInvertColorInHierarchy; }

protected:
    /**
    * Called after a child scene component is attached to this component.
    * Note: Do not change the attachment state of the child during this call.
    */
    virtual void OnChildAttached(USceneComponent* ChildComponent) override;

    /**
    * Called after a child scene component is detached from this component.
    * Note: Do not change the attachment state of the child during this call.
    */
    virtual void OnChildDetached(USceneComponent* ChildComponent) override;

public:
    FORCEINLINE bool IsAttachChildChanged() const
    {
        return bAttachChildChanged;
    }

    FORCEINLINE void ResetAttachChildChanged()
    {
        bAttachChildChanged = false;
    }
    
protected:
    virtual void Awake() {}

    /**
     * Note: Do not change the attachment state during this call.
     */
    virtual void OnEnable() {}

    /**
     * Note: Do not change the attachment state during this call.
     */
    virtual void OnDisable() {}
	
    virtual void OnDestroy() {}

protected:
    virtual void OnInteractableStateChanged() {}
    virtual void OnBlockRaycastsStateChanged() {}
    virtual void OnGrayingStateChanged() {}
    virtual void OnInvertColorStateChanged() {}
    virtual void OnRenderOpacityChanged() {}
	virtual void OnChildAttachmentChanged() {}
    virtual void OnParentCanvasChanged() {}
    
protected:
    virtual void InternalOnRectTransformDimensionsChange();
	virtual void InternalOnChildAttachmentChanged();

protected:
    virtual void InternalTransformParentChanged();

private:
    void UpdateTransformParentChanged(UBehaviourComponent* BehaviourComp, const UBehaviourComponent* ParentBehaviourComp,
        bool bUpdateOwnerCanvas, bool bUpdateEnableState = true, bool bUpdateRenderOpacity = true, bool bUpdateInvertColor = true, bool bUpdateGraying = true,
        bool bUpdateInteractable = true, bool bUpdateBlockRaycasts = true, bool bUpdateIgnoreReversedGraphics = true);
    
public:
    /**
     * This callback is called if an associated RectTransform has its dimensions changed. The call is also made to all child rect transforms, even if the child transform itself doesn't change - as it could have, depending on its anchoring.
     * Note: Do not change the attachment state during this call.
     */
    virtual void OnRectTransformDimensionsChange() {}

protected:
    virtual void OnTransformParentChanged() {}

public:
    /**
     * Called when the state of the parent Canvas is changed.
     */
    virtual void OnCanvasHierarchyChanged() {}

	virtual void OnTransformChanged() {}

public:
#if WITH_EDITORONLY_DATA
    virtual void UpdateRectTransformPreview() {}
    virtual void SetIsRootCanvas(bool bInIsRootCanvas) {}
#endif

public:
    const TArray<UBehaviourSubComponent*>& GetAllSubComponents() const
    {
        return SubComponents;
    }
	
    template<class T, class AllocatorType>
    void GetSubComponents(TArray<T*, AllocatorType>& OutComponents) const
    {
        OutComponents.Reset();
        ForEachSubComponent_Internal<T>(T::StaticClass(), [&](T* InComp)
            {
                OutComponents.Add(InComp);
            });
    }

private:
    template<class ComponentType, typename Func>
    void ForEachSubComponent_Internal(TSubclassOf<UBehaviourSubComponent> ComponentClass, Func InFunc) const
    {
        static_assert(TPointerIsConvertibleFromTo<ComponentType, const UBehaviourSubComponent>::Value, "'ComponentType' template parameter to ForEachSubComponent must be derived from UBehaviourSubComponent");

        check(ComponentClass->IsChildOf(ComponentType::StaticClass()));

        for (UBehaviourSubComponent* OwnedComponent : SubComponents)
        {
            if (OwnedComponent)
            {
                if (OwnedComponent->IsA(ComponentClass))
                {
                    InFunc(static_cast<ComponentType*>(OwnedComponent));
                }
            }
        }
    }

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UBehaviourSubComponent* AddSubComponentByClass(TSubclassOf<UBehaviourSubComponent> SubCompClass);

private:
    static bool IsDisallowMultipleComponent(UClass* InClass, UClass*& RootDisallowClass);

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    bool RemoveSubComponentByClass(TSubclassOf<UBehaviourSubComponent> SubCompClass, bool bRemoveAssociatedComponents = false);

private:
    void GetRequireSubClasses(UClass* InClass, TArray<UClass*, TInlineAllocator<8>>& RequireSubClasses) const;
	
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UObject* GetComponent(UClass* InClass, bool bIncludeInactive = false);

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UObject* GetComponentByInterface(TSubclassOf<UInterface> Interface, bool bIncludeInactive = false);
	
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UObject* GetComponentInChildren(UClass* InClass, bool bIncludeInactive = false);

private:
    UObject* InternalGetComponentInChildren(USceneComponent* SceneComp, UClass* InClass, bool bIncludeInactive = false) const;

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    UObject* GetComponentInParent(UClass* InClass, bool bIncludeInactive = false);

public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void GetComponents(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false);

    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void GetComponentsInChildren(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false);

private:
    void InternalGetComponentsInChildren(USceneComponent* SceneComp, TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false) const;
	
public:
    UFUNCTION(BlueprintCallable, Category = Behaviour)
    void GetComponentsInParent(TArray<UObject*>& Components, UClass* InClass, bool bIncludeInactive = false);

public:
    template<class T, class AllocatorType>
    void GetComponents(TArray<T*, AllocatorType>& Components, bool bIncludeInactive = false)
    {
        Components.Reset();
    	
        const auto SceneComp = Cast<USceneComponent>(this);
        if (IsValid(SceneComp))
        {
            const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
            if (IsValid(BehaviourComp))
            {
                if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
                {
                    auto TObj = Cast<T>(BehaviourComp);
                    if (TObj)
                    {
                        Components.Add(TObj);
                    }

                    const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
                    for (const auto& SubComponent : AllSubComponents)
                    {
                        if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
                        {
                            auto TSubObj = Cast<T>(SubComponent);
                            if (TSubObj)
                            {
                                Components.Add(TSubObj);
                            }
                        }
                    }
                }
            }
            else
            {
                auto TObj = Cast<T>(SceneComp);
                if (TObj)
                {
                    Components.Add(TObj);
                }
            }
        }
    }

    template<class T, class AllocatorType>
    void GetComponentsInChildren(TArray<T*, AllocatorType>& Components, bool bIncludeInactive = false)
    {
        Components.Reset();

        InternalGetComponentsInChildren<T, AllocatorType>(this, Components, bIncludeInactive);
    }

private:
    template<class T, class AllocatorType>
    void InternalGetComponentsInChildren(USceneComponent* SceneComp, TArray<T*, AllocatorType>& Components, bool bIncludeInactive = false)
    {
        if (IsValid(SceneComp))
        {
            const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
            if (IsValid(BehaviourComp))
            {
                if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
                {
                    auto TObj = Cast<T>(BehaviourComp);
                    if (TObj)
                    {
                        Components.Add(TObj);
                    }

                    const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
                    for (const auto& SubComponent : AllSubComponents)
                    {
                        if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
                        {
                            auto TSubObj = Cast<T>(SubComponent);
                            if (TSubObj)
                            {
                                Components.Add(TSubObj);
                            }
                        }
                    }
                }
            }
            else
            {
                auto TObj = Cast<T>(SceneComp);
                if (TObj)
                {
                    Components.Add(TObj);
                }
            }

            for (const auto& ChildComp : SceneComp->GetAttachChildren())
            {
                InternalGetComponentsInChildren<T, AllocatorType>(ChildComp, Components, bIncludeInactive);
            }
        }
    }

public:
    template<class T, class AllocatorType>
    void GetComponentsInParent(TArray<T*, AllocatorType>& Components, bool bIncludeInactive = false)
    {
        Components.Reset();

        auto SceneComp = Cast<USceneComponent>(this);
        while (IsValid(SceneComp))
        {
            const auto BehaviourComp = Cast<UBehaviourComponent>(SceneComp);
            if (IsValid(BehaviourComp))
            {
                if (bIncludeInactive || BehaviourComp->IsActiveAndEnabled())
                {
                    auto TObj = Cast<T>(BehaviourComp);
                    if (TObj)
                    {
                        Components.Add(TObj);
                    }

                    const auto& AllSubComponents = BehaviourComp->GetAllSubComponents();
                    for (const auto& SubComponent : AllSubComponents)
                    {
                        if (IsValid(SubComponent) && (bIncludeInactive || SubComponent->IsActiveAndEnabled()))
                        {
                            auto TSubObj = Cast<T>(SubComponent);
                            if (TSubObj)
                            {
                                Components.Add(TSubObj);
                            }
                        }
                    }
                }
            }
            else
            {
                auto TObj = Cast<T>(SceneComp);
                if (TObj)
                {
                    Components.Add(TObj);
                }
            }

            SceneComp = SceneComp->GetAttachParent();
        }
    }
};
