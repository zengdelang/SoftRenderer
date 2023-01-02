#pragma once

#include "CoreMinimal.h"
#include "RaycastResult.h"
#include "Core/Layout/RectTransformComponent.h"
#include "Framework/Application/IInputProcessor.h"
#include "EventSystemComponent.generated.h"

class UPointerEventData;
class UBaseEventData;
class UBaseInputModuleSubComponent;
class UEventSystemComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSelectedGameObjectChangedEvent, const UWorld*, const USceneComponent*);

class FEventSystemInputProcessor : public IInputProcessor
{
public:
	FEventSystemInputProcessor(UEventSystemComponent* InEventSystemComponent);
	virtual ~FEventSystemInputProcessor() override = default;
	
public:
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

protected:
	TWeakObjectPtr<UEventSystemComponent> EventSystemComponent;
	
};

/**
 * Handles input, raycasting, and sending events.
 *
 * The EventSystem is responsible for processing and handling events in a Unity scene. A scene should only contain one EventSystem. The EventSystem works in conjunction with a number of modules and mostly just holds state and delegates functionality to specific, overrideable components.
 * When the EventSystem is started it searches for any BaseInputModules attached to the same GameObject and adds them to an internal list. On update each attached module receives an UpdateModules call, where the module can modify internal state. After each module has been Updated the active module has the Process call executed.This is where custom module processing can take place.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Event), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UEventSystemComponent : public URectTransformComponent
{
	GENERATED_UCLASS_BODY()

private:
	/**
	 * The currently active EventSystems.BaseInputModule.
	 */
	UPROPERTY(Transient)
	UBaseInputModuleSubComponent* CurrentInputModule;

	UPROPERTY(Transient)
	TArray<UBaseInputModuleSubComponent*> SystemInputModules;

	/**
	 * Only one object can be selected at a time. Think: controller-selected button.
	 */
	UPROPERTY(Transient)
	USceneComponent* FirstSelected;

	/**
	 * The GameObject currently considered active by the EventSystem.
	 */
	UPROPERTY(Transient)
	USceneComponent* CurrentSelected;
	
	/**
	 * The soft area for dragging in pixels.
	 */
	UPROPERTY(EditAnywhere, Category = EventSystem)
	int32 DragThreshold;

	UPROPERTY(Transient)
	UBaseEventData* DummyData;

public:
	/**
	 * Should the EventSystem allow navigation events (move / submit / cancel).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EventSystem)
	uint8 bSendNavigationEvents : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = EventSystem)
	uint8 bCheckMultipleComponents : 1;
#endif
	
#if WITH_EDITOR
	uint8 bRegisterInEditor : 1;
#endif

private:
	UPROPERTY(Transient)
	uint8 bSelectionGuard : 1;

protected:
	TSharedPtr<FEventSystemInputProcessor> EventSystemInputProcessor;

public:
	static FOnSelectedGameObjectChangedEvent OnSelectedGameObjectChangedEvent;
	
protected:
	static TMap<TWeakObjectPtr<UWorld>, TArray<TWeakObjectPtr<UEventSystemComponent>>> EventSystems;

public:
	/**
	 * Return the current EventSystem.
	 */
	static UEventSystemComponent* GetCurrentEventSystem(UWorld* InWorld);
	static void SetCurrentEventSystem(UEventSystemComponent* InEventSystem);

public:
	//~ Begin BehaviourComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	virtual void OnDestroy() override;
	//~ End BehaviourComponent Interface.

public:
	virtual void TickEventSystem(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable, Category = EventSystem)
	UBaseInputModuleSubComponent* GetCurrentInputModule() const
	{
		return CurrentInputModule;
	}

	UFUNCTION(BlueprintCallable, Category = EventSystem)
	USceneComponent* GetFirstSelectedGameObject() const
	{
		return FirstSelected;
	}

	UFUNCTION(BlueprintCallable, Category = EventSystem)
	void SetFirstSelectedGameObject(USceneComponent* InFirstSelected)
	{
		FirstSelected = InFirstSelected;
	}

	UFUNCTION(BlueprintCallable, Category = EventSystem)
	USceneComponent* GetCurrentSelectedGameObject() const
	{
		return CurrentSelected;
	}
	
	UFUNCTION(BlueprintCallable, Category = EventSystem)
	int32 GetPixelDragThreshold() const
	{
		return DragThreshold;
	}

	UFUNCTION(BlueprintCallable, Category = EventSystem)
	void SetPixelDragThreshold(int32 InDragThreshold)
	{
		DragThreshold = InDragThreshold;
	}

	UFUNCTION(BlueprintCallable, Category = EventSystem)
	bool AlreadySelecting() const
	{
		return bSelectionGuard;
	}

private:
	UBaseEventData* GetBaseEventDataCache();

public:
	/**
	 * Set the object as selected. Will send an OnDeselect the the old selected object and OnSelect to the new selected object.
	 */
	void SetSelectedGameObject(USceneComponent* InSelectedObject, UBaseEventData* Pointer);

	/**
	 * Set the object as selected. Will send an OnDeselect the the old selected object and OnSelect to the new selected object.
	 */
	UFUNCTION(BlueprintCallable, Category = EventSystem)
	void SetSelectedGameObject(USceneComponent* InSelectedObject)
	{
		SetSelectedGameObject(InSelectedObject, GetBaseEventDataCache());
	}

	/**
	 * Is the pointer with the given ID over an EventSystem object?
	 */
	bool IsPointerOverGameObject() const;

	/**
	 * Is the pointer with the given ID over an EventSystem object?
	 *
	 * If you use IsPointerOverGameObject() without a parameter, it points to the "left mouse button" (pointerId = -1); therefore when you use IsPointerOverGameObject for touch, you should consider passing a pointerId to it
	 * Note that for touch, IsPointerOverGameObject should be used with ''OnMouseDown()'' or ''Input.GetMouseButtonDown(0)'' or ''Input.GetTouch(0).phase == TouchPhase.Began''.
	 */
	UFUNCTION(BlueprintCallable, Category = EventSystem)
	bool IsPointerOverGameObject(int32 PointerId) const;

	/**
	 * Recalculate the internal list of BaseInputModules.
	 */
	void UpdateModules();

private:
	void TickModules();

	void ChangeEventModule(UBaseInputModuleSubComponent* Module);

public:
	static void RaycastAll(const UPointerEventData* EventData, FRaycastResult& RaycastResult, const UWorld* InWorld);
	
	virtual FString ToString() const;

protected:
	void ShowDebugInfo(class AHUD* HUD, class UCanvas* Canvas, const class FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos) const;

};
