#pragma once

#include "CoreMinimal.h"
#include "Core/WidgetActor.h"
#include "Animation/UIAnimationAttachComponent.h"
#include "ChildWidgetActorComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Attach), meta = (UIBlueprintSpawnableComponent, BlueprintSpawnableComponent))
class UGUI_API UChildWidgetActorComponent : public URectTransformComponent
{
	GENERATED_UCLASS_BODY()

public:
	friend class AWidgetActor;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ChildWidget)
	TSubclassOf<AWidgetActor> ChildWidgetActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ChildWidget)
	uint8 bStripCanvasComponent : 1;

	UPROPERTY(Transient)
	AWidgetActor* ChildWidgetActor;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = ChildWidget, meta = (DisplayName = Parameters))
	UObject* ParametersObject;

	UPROPERTY(Transient)
	TMap<FName, UUIAnimationAttachComponent*> AttachChildrenNodes;

public:
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
protected:
	virtual void Awake() override;
	virtual void OnDestroy() override;

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
protected:
	void CreateChildWidgetActor();
	void DestroyChildWidgetActor();

public:
#if WITH_EDITORONLY_DATA
	void SetChildActorForEditor(AWidgetActor* WidgetActor);
	void SetChildWidgetActorClassForEditor(TSubclassOf<AWidgetActor> InClass);
	
	UObject* GetParametersObject() const
	{
		return ParametersObject;
	}
#endif

#if OPTIMIZE_SPAWN_ACTOR_FOR_UI
	void OnSliceSpawnActorDone();
#endif

public:
	UFUNCTION(BlueprintCallable, Category = ChildWidgetActor)
	AWidgetActor* GetChildWidgetActor() const { return ChildWidgetActor; }

	UFUNCTION(BlueprintCallable, Category = ChildWidgetActor)
	TSubclassOf<AWidgetActor> GetChildWidgetActorClass() const { return ChildWidgetActorClass; }

	UFUNCTION(BlueprintCallable, Category = ChildWidgetActor)
	bool GetStripCanvasComponent() const
	{
		return bStripCanvasComponent;
	}

	UFUNCTION(BlueprintCallable, Category = ChildWidgetActor)
	void SetStripCanvasComponent(bool bInStripCanvasComponent)
	{
		if (bStripCanvasComponent != bInStripCanvasComponent)
		{
			bStripCanvasComponent = bInStripCanvasComponent;
			SetChildWidgetActorClass(ChildWidgetActorClass);
		}
	}

	UFUNCTION(BlueprintCallable, Category = ChildWidgetActor)
	void SetChildWidgetActorClass(TSubclassOf<AWidgetActor> InClass);
	
};
