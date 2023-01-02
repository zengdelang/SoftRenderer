#pragma once

#include "CoreMinimal.h"
#include "TweenDefines.h"
#include "TweenBaseSubComponent.h"
#include "TweenPlayerSubComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Tween Player", DisallowMultipleComponent, HideEnableCheckBox = true))
class UGUI_API UTweenPlayerSubComponent : public UBehaviourSubComponent
{
	GENERATED_UCLASS_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Group Name"))
	FString TweenGroupName;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Group"))
	int32 TweenGroup;

	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "Auto Play"))
	uint8 bAutoPlay : 1;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = TweenBase, meta = (DisplayName = "EditPreview"))
	uint8 bEditPreview : 1;
#endif

	uint8 bNeedSetTimer : 1;
	
	UPROPERTY(Transient)
	TArray<UTweenBaseSubComponent*> TweenComponents;

	FTimerHandle WaitForRunTimerHandle;
	
	int32 ActiveTweenCount;
	
public:
	UPROPERTY(BlueprintAssignable, Category = TweenBase)
	FOnTweenFinished OnFinished;

protected:
	//~ Begin BehaviourSubComponent Interface
	virtual void Awake() override;
	virtual void OnEnable() override;
	virtual void OnDisable() override;
	//~ End BehaviourSubComponent Interface

protected:
	void CheckHasBegunPlay();
	void GetTweenComponents();
	
	UFUNCTION()
	void OnTweenFinished();
	
public:
	UFUNCTION(BlueprintCallable, Category = TweenBase)
	bool Play();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	bool PlayReverse();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	bool IsPlaying();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void Toggle();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void Stop();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void GoToBegin();

	UFUNCTION(BlueprintCallable, Category = TweenBase)
	void GoToEnd();

public:
	bool SetGroupName(const FString& InGroupName)
	{
		if (TweenGroupName != InGroupName)
		{
			TweenGroupName = InGroupName;
			GetTweenComponents();
			return true;
		}
		
		return false;
	}
};
